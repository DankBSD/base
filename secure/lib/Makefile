# $FreeBSD$

.include <src.opts.mk>

SUBDIR= 
.if ${MK_OPENSSL} != "no"
SUBDIR+=libcrypto libssl libtls
.if ${MK_OPENSSH} != "no"
SUBDIR+=libssh
.endif
.endif

SUBDIR.${MK_TESTS}+= tests

SUBDIR_WITH_INCS=libcrypto libtls

.include <bsd.subdir.mk>
