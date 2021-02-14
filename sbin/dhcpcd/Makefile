.include <src.opts.mk>

PROG=	dhcpcd

DHCPCDSRC=    ${SRCTOP}/contrib/dhcpcd
.PATH:  ${SRCTOP}/sbin/dhcpcd ${DHCPCDSRC}

SRCS= \
	src/arp.c \
	src/auth.c \
	src/bpf.c \
	src/common.c \
	src/control.c \
	src/dhcp-common.c \
	src/dhcp.c \
	src/dhcp6.c \
	src/dhcpcd.c \
	src/dhcpcd-embedded.c \
	src/dhcpcd-embedded.h \
	src/duid.c \
	src/eloop.c \
	src/if-bsd.c \
	src/if-options.c \
	src/if.c \
	src/ipv4.c \
	src/ipv4ll.c \
	src/ipv6.c \
	src/ipv6nd.c \
	src/logerr.c \
	src/privsep.c \
	src/privsep-bpf.c \
	src/privsep-bsd.c \
	src/privsep-control.c \
	src/privsep-inet.c \
	src/privsep-root.c \
	src/route.c \
	src/sa.c \
	src/script.c \
	compat/crypt/hmac.c \
	compat/pidfile.c \
	compat/rb.c \
	compat/strtoi.c \
	compat/strtou.c \

MK_WERROR=	no
LIBADD=	md
CSTD?=	c99
CFLAGS+=	-I${DHCPCDSRC} -I${DHCPCDSRC}/compat/ -I${DHCPCDSRC}/src/ -I${SRCTOP}/sbin/dhcpcd -I${.OBJDIR}/src
CFLAGS+=	-DHAVE_CONFIG_H -DNDEBUG -DINET -DARP -DARPING -DIPV4LL -DINET6 -DDHCP6 -DAUTH -DPRIVSEP

MAN= \
	src/dhcpcd.conf.5 \
	src/dhcpcd.8

CONFS=		dhcpcd.conf
CONFDIR=	/etc
CONFMODE=	644

src/dhcpcd-embedded.h: src/genembedh src/dhcpcd-definitions.conf src/dhcpcd-definitions-small.conf src/dhcpcd-embedded.h.in
	${HOST_SH} ${.ALLSRC} $^ > $@

src/dhcpcd-embedded.c: src/genembedc src/dhcpcd-definitions.conf src/dhcpcd-definitions-small.conf src/dhcpcd-embedded.c.in
	${HOST_SH} ${.ALLSRC} $^ > $@

src/dhcpcd.conf.5: src/dhcpcd.conf.5.in
	sed -e 's:@RUNDIR@:/var/run/dhcpcd:g' -e 's:@DBDIR@:/var/db/dhcpcd:g' -e 's:@SCRIPT@:${SCRIPT}:g' -e 's:@SYSCONFDIR@:/etc:g' -e 's:@HOOKDIR@:/usr/libexec/dhcpcd-hooks:g' -e 's:@DATADIR@:/usr/share:g' -e 's:@LIBDIR@:/lib:g' ${.ALLSRC} > $@
	
src/dhcpcd.8: src/dhcpcd.8.in
	sed -e 's:@RUNDIR@:/var/run/dhcpcd:g' -e 's:@DBDIR@:/var/db/dhcpcd:g' -e 's:@SCRIPT@:${SCRIPT}:g' -e 's:@SYSCONFDIR@:/etc:g' -e 's:@HOOKDIR@:/usr/libexec/dhcpcd-hooks:g' -e 's:@DATADIR@:/usr/share:g' -e 's:@LIBDIR@:/lib:g' ${.ALLSRC} > $@

.include <bsd.prog.mk>
