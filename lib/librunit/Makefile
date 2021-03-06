RUNITDIR=${SRCTOP}/contrib/runit

PACKAGE=lib${LIB}
.PATH: ${RUNITDIR}/src

LIB=	runit
INTERNALLIB=

CFLAGS+=	-I${RUNITDIR}/include
CFLAGS+=	-I${SRCTOP}/usr.sbin/runit

SRCS= \
	lib/byte/byte_chr.c lib/byte/byte_copy.c lib/byte/byte_cr.c lib/byte/byte_diff.c lib/byte/byte_rchr.c lib/byte/fmt_ptime.c lib/byte/fmt_uint.c lib/byte/fmt_uint0.c lib/byte/fmt_ulong.c lib/byte/scan_ulong.c lib/byte/str_chr.c lib/byte/str_diff.c lib/byte/str_len.c lib/byte/str_start.c \
	lib/time/iopause.c lib/time/tai_now.c lib/time/tai_pack.c lib/time/tai_sub.c lib/time/tai_unpack.c lib/time/taia_add.c lib/time/taia_approx.c lib/time/taia_frac.c lib/time/taia_less.c lib/time/taia_now.c lib/time/taia_pack.c lib/time/taia_sub.c lib/time/taia_uint.c \
	lib/unix/alloc_re.c lib/unix/alloc.c lib/unix/buffer_0.c lib/unix/buffer_1.c lib/unix/buffer_2.c lib/unix/buffer_get.c lib/unix/buffer_put.c lib/unix/buffer_read.c lib/unix/buffer_write.c lib/unix/buffer.c lib/unix/coe.c lib/unix/env.c lib/unix/error_str.c lib/unix/error.c lib/unix/fd_copy.c lib/unix/fd_move.c lib/unix/fifo.c lib/unix/lock_ex.c lib/unix/lock_exnb.c lib/unix/ndelay_off.c lib/unix/ndelay_on.c lib/unix/open_append.c lib/unix/open_read.c lib/unix/open_trunc.c lib/unix/open_write.c lib/unix/openreadclose.c lib/unix/pathexec_env.c lib/unix/pathexec_run.c lib/unix/pmatch.c lib/unix/prot.c lib/unix/readclose.c lib/unix/seek_set.c lib/unix/sgetopt.c lib/unix/sig_block.c lib/unix/sig_catch.c lib/unix/sig_pause.c lib/unix/sig.c lib/unix/stralloc_cat.c lib/unix/stralloc_catb.c lib/unix/stralloc_cats.c lib/unix/stralloc_eady.c lib/unix/stralloc_opyb.c lib/unix/stralloc_opys.c lib/unix/stralloc_pend.c lib/unix/strerr_die.c lib/unix/strerr_sys.c lib/unix/subgetopt.c lib/unix/uidgid.c lib/unix/wait_nohang.c lib/unix/wait_pid.c

MK_WERROR=	no
WARNS?=	1

.include <bsd.lib.mk>
