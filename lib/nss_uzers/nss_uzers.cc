#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>

#include <errno.h>
#include <grp.h>
#include <libzfs.h>
#include <nsswitch.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <functional>
#include <optional>

static libzfs_handle_t *zfsh = nullptr;

static std::optional<char *>
populate_buf(char *buf, size_t bsize, size_t *cursize, const char *what)
{
	size_t len = strlen(what);
	if (*cursize + len + 1 > bsize)
		return {};
	strcpy(buf + *cursize, what);
	size_t oldsize = *cursize;
	*cursize += len + 1;
	return buf + oldsize;
}

static int
z_iter_cxx(zfs_handle_t *h, void *lambda)
{
	return (*static_cast<std::function<int(zfs_handle_t *)> *>(lambda))(h);
}

static std::optional<const char *>
z_get_uprop(nvlist_t *props, const char *key)
{
	nvlist_t *prop = nullptr;
	if (nvlist_lookup_nvlist(props, key, &prop) != 0 || !prop)
		return {};
	char *val = nullptr;
	if (nvlist_lookup_string(prop, "value", &val) != 0 || !val)
		return {};
	return val;
}

static bool
fill_uzer(zfs_handle_t *h, nvlist_t *uprops, struct passwd *pbuf, char *buf,
    size_t bsize, int *errnop)
{
	size_t cursize = 0;
	auto z_uid = z_get_uprop(uprops, "dankbsd.uzers:uid");
	auto z_gid = z_get_uprop(uprops, "dankbsd.uzers:gid");
	auto z_name = z_get_uprop(uprops, "dankbsd.uzers:name");
	if (!z_uid || !z_gid || !z_name)
		return false;
	auto z_gecos = z_get_uprop(uprops, "dankbsd.uzers:gecos");
	auto z_shell = z_get_uprop(uprops, "dankbsd.uzers:shell");
	auto z_class = z_get_uprop(uprops, "dankbsd.uzers:class");

	if (sscanf(z_uid.value(), "%d", &pbuf->pw_uid) == 0)
		return false;
	if (sscanf(z_gid.value(), "%d", &pbuf->pw_gid) == 0)
		return false;

	char mp[ZFS_MAXPROPLEN];
	(void)zfs_prop_get(
	    h, ZFS_PROP_MOUNTPOINT, mp, sizeof(mp), NULL, NULL, 0, B_FALSE);

	auto r_name = populate_buf(buf, bsize, &cursize, z_name.value());
	if (!r_name) {
		*errnop = ERANGE;
		return false;
	}
	auto r_passwd = populate_buf(buf, bsize, &cursize, "*");
	if (!r_passwd) {
		*errnop = ERANGE;
		return false;
	}
	auto r_dir = populate_buf(buf, bsize, &cursize, mp);
	if (!r_dir) {
		*errnop = ERANGE;
		return false;
	}
	auto r_shell = populate_buf(
	    buf, bsize, &cursize, z_shell.value_or("/bin/sh"));
	if (!r_shell) {
		*errnop = ERANGE;
		return false;
	}
	auto r_gecos = populate_buf(buf, bsize, &cursize, z_gecos.value_or(""));
	if (!r_gecos) {
		*errnop = ERANGE;
		return false;
	}
	auto r_class = populate_buf(
	    buf, bsize, &cursize, z_class.value_or("default"));
	if (!r_class) {
		*errnop = ERANGE;
		return false;
	}

	pbuf->pw_name = r_name.value();
	pbuf->pw_passwd = r_passwd.value();
	pbuf->pw_class = r_class.value();
	pbuf->pw_gecos = r_gecos.value();
	pbuf->pw_dir = r_dir.value();
	pbuf->pw_shell = r_shell.value();
	pbuf->pw_fields = _PWF_NAME | _PWF_PASSWD | _PWF_UID | _PWF_GID |
	    _PWF_CLASS | _PWF_GECOS | _PWF_DIR | _PWF_SHELL;

	return true;
}

template <typename F>
static int
iterate_uzers(F cb)
{
	bool found = false; // to break out in the whole call tree

	std::function<int(zfs_handle_t *)> iter_cb = [&](zfs_handle_t *h) {
		if (found)
			return 1;
		auto *uprops = zfs_get_user_props(h);
		if (!uprops) {
			fprintf(
			    stderr, "WTF!? nss_uzers could not get uprops\n");
			zfs_close(h);
			return 0;
		}

		if (cb(h, uprops)) {
			found = true;
			zfs_close(h);
			return 1;
		}

		zfs_iter_filesystems(h, z_iter_cxx, (void *)&iter_cb);
		zfs_close(h);
		return 0;
	};
	zfs_iter_root(zfsh, z_iter_cxx, (void *)&iter_cb);

	return 0;
}

static int
nss_uzers_getpwuid_r(void *rv, void *, va_list ap)
{
	uid_t uid = va_arg(ap, uid_t);
	struct passwd *pbuf = va_arg(ap, struct passwd *);
	char *buf = va_arg(ap, char *);
	size_t bsize = va_arg(ap, size_t);
	int *errnop = va_arg(ap, int *);
	bool found = false;

	if (!zfsh)
		return NS_UNAVAIL;

	iterate_uzers([&](auto *h, auto *uprops) {
		auto z_uid = z_get_uprop(uprops, "dankbsd.uzers:uid");
		uid_t this_uid;
		if (!z_uid || sscanf(z_uid.value(), "%d", &this_uid) == 0 ||
		    this_uid != uid)
			return false;
		if (fill_uzer(h, uprops, pbuf, buf, bsize, errnop)) {
			void **ptr = static_cast<void **>(rv);
			*ptr = pbuf;
			found = true;
			return true;
		}
		return false;
	});

	if (*errnop != 0)
		return NS_RETURN;
	return found ? NS_SUCCESS : NS_NOTFOUND;
}

static int
nss_uzers_getpwnam_r(void *rv, void *, va_list ap)
{
	char *name = va_arg(ap, char *);
	struct passwd *pbuf = va_arg(ap, struct passwd *);
	char *buf = va_arg(ap, char *);
	size_t bsize = va_arg(ap, size_t);
	int *errnop = va_arg(ap, int *);
	bool found = false;

	if (!zfsh)
		return NS_UNAVAIL;

	iterate_uzers([&](auto *h, auto *uprops) {
		auto z_name = z_get_uprop(uprops, "dankbsd.uzers:name");
		if (!z_name || strcmp(z_name.value(), name) != 0)
			return false;
		if (fill_uzer(h, uprops, pbuf, buf, bsize, errnop)) {
			void **ptr = static_cast<void **>(rv);
			*ptr = pbuf;
			found = true;
			return true;
		}
		return false;
	});

	if (*errnop != 0)
		return NS_RETURN;
	return found ? NS_SUCCESS : NS_NOTFOUND;
}

static size_t getpwent_cursor = 0, getpwent_max = 0;

static int
nss_uzers_getpwent_r(void *rv, void *, va_list ap)
{
	struct passwd *pbuf = va_arg(ap, struct passwd *);
	char *buf = va_arg(ap, char *);
	size_t bsize = va_arg(ap, size_t);
	int *errnop = va_arg(ap, int *);
	bool found = false;

	if (!zfsh)
		return NS_UNAVAIL;

	if (getpwent_cursor >= getpwent_max)
		return NS_NOTFOUND;

	size_t i = 0;
	iterate_uzers([&](auto *h, auto *uprops) {
		if (i++ < getpwent_cursor)
			return false;
		if (fill_uzer(h, uprops, pbuf, buf, bsize, errnop)) {
			void **ptr = static_cast<void **>(rv);
			*ptr = pbuf;
			found = true;
			return true;
		}
		return false;
	});
	getpwent_cursor = i;

	if (*errnop != 0)
		return NS_RETURN;
	return found ? NS_SUCCESS : NS_NOTFOUND;
}

int
nss_uzers_setpwent(void *, void *, va_list)
{
	getpwent_cursor = 0;
	getpwent_max = 0;
	iterate_uzers([&](auto *, auto *) {
		getpwent_max++;
		return false;
	});
	return NS_SUCCESS;
}

int
nss_uzers_endpwent(void *, void *, va_list)
{
	return NS_SUCCESS;
}

extern "C" void
nss_uzers_cleanup(ns_mtab *, unsigned int)
{
	if (zfsh) {
		libzfs_fini(zfsh);
		zfsh = nullptr;
	}
}

extern "C" ns_mtab *
nss_module_register(const char * /*modname*/, unsigned int *plen,
    nss_module_unregister_fn *fptr)
{
	zfsh = libzfs_init();
	if (!zfsh) {
		fprintf(stderr, "WTF!? nss_uzers could not open ZFS\n");
		*plen = 0;
		*fptr = nullptr;
		return nullptr;
	}

	static ns_mtab mtab[] = {
		{ "passwd", "getpwuid_r", &nss_uzers_getpwuid_r, 0 },
		{ "passwd", "getpwnam_r", &nss_uzers_getpwnam_r, 0 },
		{ "passwd", "getpwent_r", &nss_uzers_getpwent_r, 0 },
		{ "passwd", "setpwent", &nss_uzers_setpwent, 0 },
		{ "passwd", "endpwent", &nss_uzers_endpwent, 0 },

		// { "group", "getgrgid_r", &nss_uzers_getgrgid_r, 0 },
		// { "group", "getgrnam_r", &nss_uzers_getgrnam_r, 0 },
		// { "group", "getgrent_r", &nss_uzers_getgrent_r, 0 },
		// { "group", "setgrent", &nss_uzers_setgrent, 0 },
		// { "group", "endgrent", &nss_uzers_endgrent, 0 },
		// { "group", "getgroupmembership",
		// &nss_uzers_getgroupmembership,
		//     0 }, /* aka getgrouplist */
	};

	*plen = sizeof(mtab) / sizeof(mtab[0]);
	*fptr = nss_uzers_cleanup;

	return mtab;
}
