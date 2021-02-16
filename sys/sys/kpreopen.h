#ifndef _SYS_KPREOPEN_H
#define _SYS_KPREOPEN_H

#ifndef _KERNEL
#include <stdbool.h>
#endif
#include <sys/types.h>
#include <sys/queue.h>
#include <sys/syslimits.h>

#ifdef _KERNEL
#include <sys/malloc.h>
MALLOC_DECLARE(M_KPREOPEN);
#endif

struct kpreopen {
	STAILQ_ENTRY(kpreopen) kp_entry;
	char kp_path[PATH_MAX];
	size_t kp_pathlen;
	int kp_fd;
};

static inline bool
isprefix(const char *dir, size_t dirlen, const char *path)
{
	size_t i;
	for (i = 0; i < dirlen; i++)
		if (path[i] != dir[i])
			return false;
	return path[i] == '/' || path[i] == '\0';
}

#endif
