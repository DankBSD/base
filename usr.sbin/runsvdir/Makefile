RUNITDIR=${SRCTOP}/contrib/runit
.PATH: ${RUNITDIR}/src

CFLAGS+=	-I${RUNITDIR}/include
CFLAGS+=	-I${SRCTOP}/usr.sbin/runit

PROG=	runsvdir
SRCS=	runsvdir.c
MAN=	../doc/man/runsvdir.8

LIBADD=	runit

MK_WERROR=	no
WARNS?=	1

.include <bsd.prog.mk>
