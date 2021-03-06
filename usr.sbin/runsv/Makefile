RUNITDIR=${SRCTOP}/contrib/runit
.PATH: ${RUNITDIR}/src

CFLAGS+=	-I${RUNITDIR}/include
CFLAGS+=	-I${SRCTOP}/usr.sbin/runit

PROG=	runsv
SRCS=	runsv.c
MAN=	../doc/man/runsv.8

LIBADD=	runit

MK_WERROR=	no
WARNS?=	1

.include <bsd.prog.mk>
