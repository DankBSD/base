.include <src.opts.mk>

PACKAGE=lib${LIB}

LIB=	tls
SHLIB_MAJOR=	20

LIBTLSSRC=	${SRCTOP}/contrib/libretls

.PATH:	${LIBTLSSRC}

LIBADD=	crypto ssl
CFLAGS+=	-I${LIBTLSSRC}/include
CFLAGS+=	-D__BEGIN_HIDDEN_DECLS= -D__END_HIDDEN_DECLS=
LDFLAGS+=	--no-undefined

INCS= include/tls.h

SRCS = \
	tls.c \
	tls_bio_cb.c \
	tls_client.c \
	tls_config.c \
	tls_conninfo.c \
	tls_keypair.c \
	tls_ocsp.c \
	tls_peer.c \
	tls_server.c \
	tls_util.c \
	tls_verify.c \
	compat/getentropy_freebsd.c \
	compat/freezero.c

MAN= \
	man/tls_accept_socket.3 \
	man/tls_client.3 \
	man/tls_config_ocsp_require_stapling.3 \
	man/tls_config_set_protocols.3 \
	man/tls_config_set_session_id.3 \
	man/tls_config_verify.3 \
	man/tls_conn_version.3 \
	man/tls_connect.3 \
	man/tls_init.3 \
	man/tls_load_file.3 \
	man/tls_ocsp_process_response.3 \
	man/tls_read.3

.include <bsd.lib.mk>
