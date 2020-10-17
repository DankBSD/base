# $FreeBSD$

PACKAGE= tests

TESTSDIR= ${TESTSBASE}

${PACKAGE}FILES+=		README

KYUAFILE= yes

SUBDIR+= etc
SUBDIR+= sys

SUBDIR_PARALLEL=
# Skip make includes for this part of the tree
SUBDIR_WITH_INCS=

afterinstall: install-tests-local
install-tests-local: .PHONY
	${INSTALL_SYMLINK} -T 'package=tests' \
	    ../local/tests ${DESTDIR}${TESTSDIR}/local

.include "Makefile.inc0"
.include <bsd.test.mk>
