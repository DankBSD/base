#	$NetBSD: Makefile,v 1.1 2002/03/01 11:21:58 ad Exp $
# $FreeBSD$

.include <src.opts.mk>

PACKAGE=runtime
PROG=	pkill

LIBADD=	kvm jail

LINKS=	${BINDIR}/pkill ${BINDIR}/pgrep
MLINKS=	pkill.1 pgrep.1

HAS_TESTS=
SUBDIR.${MK_TESTS}+= tests

.include <bsd.prog.mk>
