RUNITDIR=${SRCTOP}/contrib/runit
.PATH: ${RUNITDIR}/src

CFLAGS+=	-I${RUNITDIR}/include
CFLAGS+=	-I${SRCTOP}/usr.sbin/runit

PROG=	runit
SRCS=	runit.c
MAN=	../doc/man/runit.8

LIBADD=	runit

MK_WERROR=	no
WARNS?=	1

.include <bsd.prog.mk>
