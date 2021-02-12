.include <src.opts.mk>

OPENNTPDSRC=	${SRCTOP}/contrib/openntpd

.PATH:	${OPENNTPDSRC}

CONFS=	ntpd.conf
PROG=	ntpd
LINKS=	${BINDIR}/ntpd ${BINDIR}/ntpctl

SRCS=	\
	src/client.c \
	src/config.c \
	src/control.c \
	src/fake-sensors.c \
	src/log.c \
	src/ntp_dns.c \
	src/ntp_msg.c \
	src/ntp.c \
	src/ntpd.c \
	src/parse.c \
	src/server.c \
	src/util.c \
	compat/adjfreq_freebsd.c \
	compat/freezero.c \
	compat/imsg.c \
	compat/imsg-buffer.c \
	compat/md5.c \
	compat/progname.c \
	compat/recallocarray.c \
	compat/setproctitle.c

MAN= \
	src/ntpd.conf.5 \
	src/ntpctl.8 \
	src/ntpd.8

MK_WERROR=	no
CFLAGS+=	-DHAVE_CLOSEFROM -DHAVE_GETENTROPY -DHAVE_SETGROUPS -DHAVE_SETRESGID -DHAVE_SETRESUID -DHAVE___PROGNAME
CFLAGS+=	-DNTPD_USER=\"_ntp\" -DCONSTRAINT_CA=\"/etc/ssl/cert.pem\"
CFLAGS+=	-I${OPENNTPDSRC}/include \
	-I${OPENNTPDSRC}/src

.if ${MK_OPENSSL} != "no"
LIBADD+=	crypto ssl tls
CFLAGS+=	-DHAVE_LIBTLS
SRCS+=	src/constraint.c
.else
SRCS+=	src/constraint-disabled.c
.endif

.include <bsd.prog.mk>
