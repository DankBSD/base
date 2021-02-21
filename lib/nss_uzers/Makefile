# $FreeBSD$

.include <src.opts.mk>

SHLIB_MAJOR=	1
SHLIB_NAME?=	${LIB}.so.${SHLIB_MAJOR}
LIB=			nss_uzers
SRCS=			nss_uzers.cc

LIBADD=		nvpair zfs cxxrt

MK_WERROR=no
CXXSTD=		c++17
CFLAGS+=	-DIN_BASE
CFLAGS+=	-I${.CURDIR}
CFLAGS+=	-I${SRCTOP}/sys/contrib/openzfs/include
CFLAGS+=	-I${SRCTOP}/sys/contrib/openzfs/lib/libspl/include
CFLAGS+=	-I${SRCTOP}/sys/contrib/openzfs/lib/libspl/include/os/freebsd
CFLAGS+=	-I${SRCTOP}/sys
CFLAGS+=	-include ${SRCTOP}/sys/contrib/openzfs/include/os/freebsd/spl/sys/ccompile.h
CFLAGS+=	-I${SRCTOP}/cddl/usr.sbin
CFLAGS+=	-D_MACHINE_FLOAT_H_ -DHAVE_ISSETUGID
CXXFLAGS+=-fno-rtti -fno-exceptions

.include <bsd.lib.mk>
