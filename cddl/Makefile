# $FreeBSD$

.include <src.opts.mk>

SUBDIR=	lib .WAIT \
	sbin share usr.bin usr.sbin

SUBDIR.${MK_TESTS}+= tests

SUBDIR_WITH_INCS=lib usr.bin usr.sbin
SUBDIR_PARALLEL=

.include <bsd.subdir.mk>
