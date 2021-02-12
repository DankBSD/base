DOASSRC=	${SRCTOP}/contrib/doas

.PATH:	${SRCTOP}/usr.bin/doas ${DOASSRC}

PROG=	doas

LIBADD=	util pam

BINOWN=	root
BINMODE=4555
PRECIOUSPROG=

SRCS= \
	doas.c \
	env.c \
	parse.y \
	compat/execvpe.c \
	compat/reallocarray.c

MAN= \
	doas.1 \
	doas.conf.5

MK_WERROR=	no
CFLAGS+=	-DUSE_PAM -DDOAS_CONF=\"/etc/doas.conf\" -DHAVE_LOGIN_CAP_H
CFLAGS+=	-I${DOASSRC}

CONFS=	doas.conf
CONFDIR=	/etc
CONFMODE=	644

.include <bsd.prog.mk>
