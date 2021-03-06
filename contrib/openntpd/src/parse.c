/* original parser id follows */
/* yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93" */
/* (use YYMAJOR/YYMINOR for ifdefs dependent on parser version) */

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYPATCH 20200330

#define YYEMPTY        (-1)
#define yyclearin      (yychar = YYEMPTY)
#define yyerrok        (yyerrflag = 0)
#define YYRECOVERING() (yyerrflag != 0)
#define YYENOMEM       (-2)
#define YYEOF          0
#define YYPREFIX "yy"

#define YYPURE 0

#line 23 "/usr/src/contrib/openntpd/src/parse.y"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "ntpd.h"

TAILQ_HEAD(files, file)		 files = TAILQ_HEAD_INITIALIZER(files);
static struct file {
	TAILQ_ENTRY(file)	 entry;
	FILE			*stream;
	char			*name;
	int			 lineno;
	int			 errors;
} *file, *topfile;
struct file	*pushfile(const char *);
int		 popfile(void);
int		 yyparse(void);
int		 yylex(void);
int		 yyerror(const char *, ...)
    __attribute__((__format__ (printf, 1, 2)))
    __attribute__((__nonnull__ (1)));
int		 kw_cmp(const void *, const void *);
int		 lookup(char *);
int		 lgetc(int);
int		 lungetc(int);
int		 findeol(void);

struct sockaddr_in		 query_addr4;
struct sockaddr_in6		 query_addr6;
int				 poolseqnum;

struct opts {
	int		weight;
	int		correction;
	int		stratum;
	int		rtable;
	int		trusted;
	char		*refstr;
} opts;
void		opts_default(void);

typedef struct {
	union {
		int64_t			 number;
		char			*string;
		struct ntp_addr_wrap	*addr;
		struct opts		 opts;
	} v;
	int lineno;
} YYSTYPE;

#line 83 "y.tab.c"

/* compatibility with bison */
#ifdef YYPARSE_PARAM
/* compatibility with FreeBSD */
# ifdef YYPARSE_PARAM_TYPE
#  define YYPARSE_DECL() yyparse(YYPARSE_PARAM_TYPE YYPARSE_PARAM)
# else
#  define YYPARSE_DECL() yyparse(void *YYPARSE_PARAM)
# endif
#else
# define YYPARSE_DECL() yyparse(void)
#endif

/* Parameters sent to lex. */
#ifdef YYLEX_PARAM
# define YYLEX_DECL() yylex(void *YYLEX_PARAM)
# define YYLEX yylex(YYLEX_PARAM)
#else
# define YYLEX_DECL() yylex(void)
# define YYLEX yylex()
#endif

#if !(defined(yylex) || defined(YYSTATE))
int YYLEX_DECL();
#endif

/* Parameters sent to yyerror. */
#ifndef YYERROR_DECL
#define YYERROR_DECL() yyerror(const char *s)
#endif
#ifndef YYERROR_CALL
#define YYERROR_CALL(msg) yyerror(msg)
#endif

extern int YYPARSE_DECL();

#define LISTEN 257
#define ON 258
#define CONSTRAINT 259
#define CONSTRAINTS 260
#define FROM 261
#define QUERY 262
#define TRUSTED 263
#define SERVER 264
#define SERVERS 265
#define SENSOR 266
#define CORRECTION 267
#define RTABLE 268
#define REFID 269
#define STRATUM 270
#define WEIGHT 271
#define ERROR 272
#define STRING 273
#define NUMBER 274
#define YYERRCODE 256
typedef int YYINT;
static const YYINT yylhs[] = {                           -1,
    0,    0,    0,    0,   19,   19,   19,   19,   19,   19,
   19,    1,    3,    3,    2,   20,    4,    4,    5,    5,
    6,   21,    7,    7,    8,    8,    9,    9,   22,   10,
   10,   11,   11,   12,   12,   12,   12,   12,   13,   15,
   16,   17,   14,   18,
};
static const YYINT yylen[] = {                            2,
    0,    2,    3,    3,    4,    3,    3,    3,    3,    3,
    3,    1,    2,    1,    1,    0,    2,    0,    2,    1,
    1,    0,    2,    0,    2,    1,    1,    1,    0,    2,
    0,    2,    1,    1,    1,    1,    1,    1,    2,    2,
    2,    2,    2,    1,
};
static const YYINT yydefred[] = {                         1,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    2,
    0,    4,    0,    0,    0,    0,   12,    0,    0,    0,
    3,    0,   15,   14,    0,    9,    6,    8,    0,    7,
   11,    0,    5,    0,   13,   44,    0,    0,   26,   27,
   28,    0,    0,    0,    0,   33,   34,   35,   36,   37,
   38,    0,    0,   20,   21,   42,   25,   39,   40,   41,
   32,   43,   19,
};
static const YYINT yydgoto[] = {                          1,
   18,   24,   25,   33,   53,   54,   28,   38,   39,   31,
   45,   46,   47,   55,   48,   49,   40,   41,   11,   34,
   29,   32,
};
static const YYINT yysindex[] = {                         0,
    8,   -2, -249, -250, -241, -240, -251, -251, -247,    0,
   14,    0, -251, -246, -246, -245,    0,    0,    0,    0,
    0,    0,    0,    0, -251,    0,    0,    0, -248,    0,
    0, -257,    0, -243,    0,    0, -244, -248,    0,    0,
    0, -242, -239, -238, -257,    0,    0,    0,    0,    0,
    0, -237, -243,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,
};
static const YYINT yyrindex[] = {                         0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   -9,   -9,  -10,
    0,   -5,    0,    0,   19,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   21,    0,    0,
    0,    0,    0,    0,   23,    0,    0,    0,    0,    0,
    0,    0,   25,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,
};
static const YYINT yygindex[] = {                         0,
   -6,   24,    0,    0,    0,  -15,   22,    0,    2,    0,
    0,   -3,    0,    0,    0,    0,  -29,  -28,    0,    0,
    0,    0,
};
#define YYTABLESIZE 274
static const YYINT yytable[] = {                         31,
   24,   19,   50,   51,   18,   36,   22,   12,   13,   42,
   14,   43,   44,   37,   36,   50,   51,   10,   35,   15,
   16,   17,   37,   21,   52,   20,   23,   27,   10,   56,
   23,   58,   30,   59,   17,   60,   62,   63,   26,   57,
   30,   61,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   29,   22,    0,    0,   29,    0,   29,   29,
   29,   22,   16,    2,    3,    0,    4,    5,    0,    6,
    0,    7,    8,    9,
};
static const YYINT yycheck[] = {                         10,
   10,    8,   32,   32,   10,  263,   13,   10,  258,  267,
  261,  269,  270,  271,  263,   45,   45,   10,   25,  261,
  261,  273,  271,   10,  268,  273,  273,  273,   10,  274,
   10,  274,   10,  273,   10,  274,  274,   53,   15,   38,
   19,   45,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,  263,  263,   -1,   -1,  267,   -1,  269,  270,
  271,  271,  268,  256,  257,   -1,  259,  260,   -1,  262,
   -1,  264,  265,  266,
};
#define YYFINAL 1
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 274
#define YYUNDFTOKEN 299
#define YYTRANSLATE(a) ((a) > YYMAXTOKEN ? YYUNDFTOKEN : (a))
#if YYDEBUG
static const char *const yyname[] = {

"end-of-file",0,0,0,0,0,0,0,0,0,"'\\n'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"LISTEN","ON","CONSTRAINT",
"CONSTRAINTS","FROM","QUERY","TRUSTED","SERVER","SERVERS","SENSOR","CORRECTION",
"RTABLE","REFID","STRATUM","WEIGHT","ERROR","STRING","NUMBER",0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"illegal-symbol",
};
static const char *const yyrule[] = {
"$accept : grammar",
"grammar :",
"grammar : grammar '\\n'",
"grammar : grammar main '\\n'",
"grammar : grammar error '\\n'",
"main : LISTEN ON address listen_opts",
"main : QUERY FROM STRING",
"main : SERVERS address server_opts",
"main : SERVER address server_opts",
"main : CONSTRAINTS FROM url",
"main : CONSTRAINT FROM urllist",
"main : SENSOR STRING sensor_opts",
"address : STRING",
"urllist : urllist address",
"urllist : url",
"url : STRING",
"$$1 :",
"listen_opts : $$1 listen_opts_l",
"listen_opts :",
"listen_opts_l : listen_opts_l listen_opt",
"listen_opts_l : listen_opt",
"listen_opt : rtable",
"$$2 :",
"server_opts : $$2 server_opts_l",
"server_opts :",
"server_opts_l : server_opts_l server_opt",
"server_opts_l : server_opt",
"server_opt : weight",
"server_opt : trusted",
"$$3 :",
"sensor_opts : $$3 sensor_opts_l",
"sensor_opts :",
"sensor_opts_l : sensor_opts_l sensor_opt",
"sensor_opts_l : sensor_opt",
"sensor_opt : correction",
"sensor_opt : refid",
"sensor_opt : stratum",
"sensor_opt : weight",
"sensor_opt : trusted",
"correction : CORRECTION NUMBER",
"refid : REFID STRING",
"stratum : STRATUM NUMBER",
"weight : WEIGHT NUMBER",
"rtable : RTABLE NUMBER",
"trusted : TRUSTED",

};
#endif

#if YYDEBUG
int      yydebug;
#endif

int      yyerrflag;
int      yychar;
YYSTYPE  yyval;
YYSTYPE  yylval;
int      yynerrs;

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 10000
#define YYMAXDEPTH  10000
#endif
#endif

#define YYINITSTACKSIZE 200

typedef struct {
    unsigned stacksize;
    YYINT    *s_base;
    YYINT    *s_mark;
    YYINT    *s_last;
    YYSTYPE  *l_base;
    YYSTYPE  *l_mark;
} YYSTACKDATA;
/* variables for the parser stack */
static YYSTACKDATA yystack;
#line 498 "/usr/src/contrib/openntpd/src/parse.y"

void
opts_default(void)
{
	memset(&opts, 0, sizeof opts);
	opts.weight = 1;
	opts.stratum = 1;
}

struct keywords {
	const char	*k_name;
	int		 k_val;
};

int
yyerror(const char *fmt, ...)
{
	va_list		 ap;
	char		*msg;

	file->errors++;
	va_start(ap, fmt);
	if (vasprintf(&msg, fmt, ap) == -1)
		fatalx("yyerror vasprintf");
	va_end(ap);
	log_warnx("%s:%d: %s", file->name, yylval.lineno, msg);
	free(msg);
	return (0);
}

int
kw_cmp(const void *k, const void *e)
{
	return (strcmp(k, ((const struct keywords *)e)->k_name));
}

int
lookup(char *s)
{
	/* this has to be sorted always */
	static const struct keywords keywords[] = {
		{ "constraint",		CONSTRAINT},
		{ "constraints",	CONSTRAINTS},
		{ "correction",		CORRECTION},
		{ "from",		FROM},
		{ "listen",		LISTEN},
		{ "on",			ON},
		{ "query",		QUERY},
		{ "refid",		REFID},
		{ "rtable",		RTABLE},
		{ "sensor",		SENSOR},
		{ "server",		SERVER},
		{ "servers",		SERVERS},
		{ "stratum",		STRATUM},
		{ "trusted",		TRUSTED},
		{ "weight",		WEIGHT}
	};
	const struct keywords	*p;

	p = bsearch(s, keywords, sizeof(keywords)/sizeof(keywords[0]),
	    sizeof(keywords[0]), kw_cmp);

	if (p)
		return (p->k_val);
	else
		return (STRING);
}

#define MAXPUSHBACK	128

u_char	*parsebuf;
int	 parseindex;
u_char	 pushback_buffer[MAXPUSHBACK];
int	 pushback_index = 0;

int
lgetc(int quotec)
{
	int		c, next;

	if (parsebuf) {
		/* Read character from the parsebuffer instead of input. */
		if (parseindex >= 0) {
			c = parsebuf[parseindex++];
			if (c != '\0')
				return (c);
			parsebuf = NULL;
		} else
			parseindex++;
	}

	if (pushback_index)
		return (pushback_buffer[--pushback_index]);

	if (quotec) {
		if ((c = getc(file->stream)) == EOF) {
			yyerror("reached end of file while parsing "
			    "quoted string");
			if (file == topfile || popfile() == EOF)
				return (EOF);
			return (quotec);
		}
		return (c);
	}

	while ((c = getc(file->stream)) == '\\') {
		next = getc(file->stream);
		if (next != '\n') {
			c = next;
			break;
		}
		yylval.lineno = file->lineno;
		file->lineno++;
	}

	while (c == EOF) {
		if (file == topfile || popfile() == EOF)
			return (EOF);
		c = getc(file->stream);
	}
	return (c);
}

int
lungetc(int c)
{
	if (c == EOF)
		return (EOF);
	if (parsebuf) {
		parseindex--;
		if (parseindex >= 0)
			return (c);
	}
	if (pushback_index < MAXPUSHBACK-1)
		return (pushback_buffer[pushback_index++] = c);
	else
		return (EOF);
}

int
findeol(void)
{
	int	c;

	parsebuf = NULL;

	/* skip to either EOF or the first real EOL */
	while (1) {
		if (pushback_index)
			c = pushback_buffer[--pushback_index];
		else
			c = lgetc(0);
		if (c == '\n') {
			file->lineno++;
			break;
		}
		if (c == EOF)
			break;
	}
	return (ERROR);
}

int
yylex(void)
{
	u_char	 buf[8096];
	u_char	*p;
	int	 quotec, next, c;
	int	 token;

	p = buf;
	while ((c = lgetc(0)) == ' ' || c == '\t')
		; /* nothing */

	yylval.lineno = file->lineno;
	if (c == '#')
		while ((c = lgetc(0)) != '\n' && c != EOF)
			; /* nothing */

	switch (c) {
	case '\'':
	case '"':
		quotec = c;
		while (1) {
			if ((c = lgetc(quotec)) == EOF)
				return (0);
			if (c == '\n') {
				file->lineno++;
				continue;
			} else if (c == '\\') {
				if ((next = lgetc(quotec)) == EOF)
					return (0);
				if (next == quotec || next == ' ' ||
				    next == '\t')
					c = next;
				else if (next == '\n') {
					file->lineno++;
					continue;
				} else
					lungetc(next);
			} else if (c == quotec) {
				*p = '\0';
				break;
			} else if (c == '\0') {
				yyerror("syntax error");
				return (findeol());
			}
			if (p + 1 >= buf + sizeof(buf) - 1) {
				yyerror("string too long");
				return (findeol());
			}
			*p++ = c;
		}
		yylval.v.string = strdup(buf);
		if (yylval.v.string == NULL)
			fatal("yylex: strdup");
		return (STRING);
	}

#define allowed_to_end_number(x) \
	(isspace(x) || x == ')' || x ==',' || x == '/' || x == '}' || x == '=')

	if (c == '-' || isdigit(c)) {
		do {
			*p++ = c;
			if ((size_t)(p-buf) >= sizeof(buf)) {
				yyerror("string too long");
				return (findeol());
			}
		} while ((c = lgetc(0)) != EOF && isdigit(c));
		lungetc(c);
		if (p == buf + 1 && buf[0] == '-')
			goto nodigits;
		if (c == EOF || allowed_to_end_number(c)) {
			const char *errstr = NULL;

			*p = '\0';
			yylval.v.number = strtonum(buf, LLONG_MIN,
			    LLONG_MAX, &errstr);
			if (errstr) {
				yyerror("\"%s\" invalid number: %s",
				    buf, errstr);
				return (findeol());
			}
			return (NUMBER);
		} else {
nodigits:
			while (p > buf + 1)
				lungetc(*--p);
			c = *--p;
			if (c == '-')
				return (c);
		}
	}

#define allowed_in_string(x) \
	(isalnum(x) || (ispunct(x) && x != '(' && x != ')' && \
	x != '{' && x != '}' && x != '<' && x != '>' && \
	x != '!' && x != '=' && x != '/' && x != '#' && \
	x != ','))

	if (isalnum(c) || c == ':' || c == '_' || c == '*') {
		do {
			*p++ = c;
			if ((size_t)(p-buf) >= sizeof(buf)) {
				yyerror("string too long");
				return (findeol());
			}
		} while ((c = lgetc(0)) != EOF && (allowed_in_string(c)));
		lungetc(c);
		*p = '\0';
		if ((token = lookup(buf)) == STRING)
			if ((yylval.v.string = strdup(buf)) == NULL)
				fatal("yylex: strdup");
		return (token);
	}
	if (c == '\n') {
		yylval.lineno = file->lineno;
		file->lineno++;
	}
	if (c == EOF)
		return (0);
	return (c);
}

struct file *
pushfile(const char *name)
{
	struct file	*nfile;

	if ((nfile = calloc(1, sizeof(struct file))) == NULL) {
		log_warn("%s", __func__);
		return (NULL);
	}
	if ((nfile->name = strdup(name)) == NULL) {
		log_warn("%s", __func__);
		free(nfile);
		return (NULL);
	}
	if ((nfile->stream = fopen(nfile->name, "r")) == NULL) {
		log_warn("%s: %s", __func__, nfile->name);
		free(nfile->name);
		free(nfile);
		return (NULL);
	}
	nfile->lineno = 1;
	TAILQ_INSERT_TAIL(&files, nfile, entry);
	return (nfile);
}

int
popfile(void)
{
	struct file	*prev;

	if ((prev = TAILQ_PREV(file, files, entry)) != NULL)
		prev->errors += file->errors;

	TAILQ_REMOVE(&files, file, entry);
	fclose(file->stream);
	free(file->name);
	free(file);
	file = prev;
	return (file ? 0 : EOF);
}

int
parse_config(const char *filename, struct ntpd_conf *xconf)
{
	int		 errors = 0;

	conf = xconf;
	TAILQ_INIT(&conf->listen_addrs);
	TAILQ_INIT(&conf->ntp_peers);
	TAILQ_INIT(&conf->ntp_conf_sensors);
	TAILQ_INIT(&conf->constraints);

	if ((file = pushfile(filename)) == NULL) {
		return (-1);
	}
	topfile = file;

	yyparse();
	errors = file->errors;
	popfile();

	return (errors ? -1 : 0);
}
#line 707 "y.tab.c"

#if YYDEBUG
#include <stdio.h>	/* needed for printf */
#endif

#include <stdlib.h>	/* needed for malloc, etc */
#include <string.h>	/* needed for memset */

/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int yygrowstack(YYSTACKDATA *data)
{
    int i;
    unsigned newsize;
    YYINT *newss;
    YYSTYPE *newvs;

    if ((newsize = data->stacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return YYENOMEM;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = (int) (data->s_mark - data->s_base);
    newss = (YYINT *)realloc(data->s_base, newsize * sizeof(*newss));
    if (newss == NULL)
        return YYENOMEM;

    data->s_base = newss;
    data->s_mark = newss + i;

    newvs = (YYSTYPE *)realloc(data->l_base, newsize * sizeof(*newvs));
    if (newvs == NULL)
        return YYENOMEM;

    data->l_base = newvs;
    data->l_mark = newvs + i;

    data->stacksize = newsize;
    data->s_last = data->s_base + newsize - 1;
    return 0;
}

#if YYPURE || defined(YY_NO_LEAKS)
static void yyfreestack(YYSTACKDATA *data)
{
    free(data->s_base);
    free(data->l_base);
    memset(data, 0, sizeof(*data));
}
#else
#define yyfreestack(data) /* nothing */
#endif

#define YYABORT  goto yyabort
#define YYREJECT goto yyabort
#define YYACCEPT goto yyaccept
#define YYERROR  goto yyerrlab

int
YYPARSE_DECL()
{
    int yym, yyn, yystate;
#if YYDEBUG
    const char *yys;

    if ((yys = getenv("YYDEBUG")) != NULL)
    {
        yyn = *yys;
        if (yyn >= '0' && yyn <= '9')
            yydebug = yyn - '0';
    }
#endif

    yym = 0;
    yyn = 0;
    yynerrs = 0;
    yyerrflag = 0;
    yychar = YYEMPTY;
    yystate = 0;

#if YYPURE
    memset(&yystack, 0, sizeof(yystack));
#endif

    if (yystack.s_base == NULL && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
    yystack.s_mark = yystack.s_base;
    yystack.l_mark = yystack.l_base;
    yystate = 0;
    *yystack.s_mark = 0;

yyloop:
    if ((yyn = yydefred[yystate]) != 0) goto yyreduce;
    if (yychar < 0)
    {
        yychar = YYLEX;
        if (yychar < 0) yychar = YYEOF;
#if YYDEBUG
        if (yydebug)
        {
            if ((yys = yyname[YYTRANSLATE(yychar)]) == NULL) yys = yyname[YYUNDFTOKEN];
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
    }
    if (((yyn = yysindex[yystate]) != 0) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) yychar)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, yystate, yytable[yyn]);
#endif
        if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
        yystate = yytable[yyn];
        *++yystack.s_mark = yytable[yyn];
        *++yystack.l_mark = yylval;
        yychar = YYEMPTY;
        if (yyerrflag > 0)  --yyerrflag;
        goto yyloop;
    }
    if (((yyn = yyrindex[yystate]) != 0) && (yyn += yychar) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) yychar)
    {
        yyn = yytable[yyn];
        goto yyreduce;
    }
    if (yyerrflag != 0) goto yyinrecovery;

    YYERROR_CALL("syntax error");

    goto yyerrlab; /* redundant goto avoids 'unused label' warning */
yyerrlab:
    ++yynerrs;

yyinrecovery:
    if (yyerrflag < 3)
    {
        yyerrflag = 3;
        for (;;)
        {
            if (((yyn = yysindex[*yystack.s_mark]) != 0) && (yyn += YYERRCODE) >= 0 &&
                    yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) YYERRCODE)
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *yystack.s_mark, yytable[yyn]);
#endif
                if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
                yystate = yytable[yyn];
                *++yystack.s_mark = yytable[yyn];
                *++yystack.l_mark = yylval;
                goto yyloop;
            }
            else
            {
#if YYDEBUG
                if (yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *yystack.s_mark);
#endif
                if (yystack.s_mark <= yystack.s_base) goto yyabort;
                --yystack.s_mark;
                --yystack.l_mark;
            }
        }
    }
    else
    {
        if (yychar == YYEOF) goto yyabort;
#if YYDEBUG
        if (yydebug)
        {
            if ((yys = yyname[YYTRANSLATE(yychar)]) == NULL) yys = yyname[YYUNDFTOKEN];
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, yystate, yychar, yys);
        }
#endif
        yychar = YYEMPTY;
        goto yyloop;
    }

yyreduce:
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, yystate, yyn, yyrule[yyn]);
#endif
    yym = yylen[yyn];
    if (yym > 0)
        yyval = yystack.l_mark[1-yym];
    else
        memset(&yyval, 0, sizeof yyval);

    switch (yyn)
    {
case 4:
#line 106 "/usr/src/contrib/openntpd/src/parse.y"
	{ file->errors++; }
break;
case 5:
#line 109 "/usr/src/contrib/openntpd/src/parse.y"
	{
			struct listen_addr	*la;
			struct ntp_addr		*h, *next;

			if ((h = yystack.l_mark[-1].v.addr->a) == NULL &&
			    (host_dns(yystack.l_mark[-1].v.addr->name, 0, &h) == -1 || !h)) {
				yyerror("could not resolve \"%s\"", yystack.l_mark[-1].v.addr->name);
				free(yystack.l_mark[-1].v.addr->name);
				free(yystack.l_mark[-1].v.addr);
				YYERROR;
			}

			for (; h != NULL; h = next) {
				next = h->next;
				la = calloc(1, sizeof(struct listen_addr));
				if (la == NULL)
					fatal("listen on calloc");
				la->fd = -1;
				la->rtable = yystack.l_mark[0].v.opts.rtable;
				memcpy(&la->sa, &h->ss,
				    sizeof(struct sockaddr_storage));
				TAILQ_INSERT_TAIL(&conf->listen_addrs, la,
				    entry);
				free(h);
			}
			free(yystack.l_mark[-1].v.addr->name);
			free(yystack.l_mark[-1].v.addr);
		}
break;
case 6:
#line 137 "/usr/src/contrib/openntpd/src/parse.y"
	{
			struct sockaddr_in sin4;
			struct sockaddr_in6 sin6;

			memset(&sin4, 0, sizeof(sin4));
			sin4.sin_family = AF_INET;
			memset(&sin6, 0, sizeof(sin6));
			sin6.sin6_family = AF_INET6;
#ifdef HAVE_STRUCT_SOCKADDR_IN_SIN_LEN
			sin4.sin_len = sizeof(struct sockaddr_in);
			sin6.sin6_len = sizeof(struct sockaddr_in6);
#endif

			if (inet_pton(AF_INET, yystack.l_mark[0].v.string, &sin4.sin_addr) == 1)
				memcpy(&query_addr4, &sin4, sizeof(struct sockaddr_in));
			else if (inet_pton(AF_INET6, yystack.l_mark[0].v.string, &sin6.sin6_addr) == 1)
				memcpy(&query_addr6, &sin6, sizeof(struct sockaddr_in6));
			else {
				yyerror("invalid IPv4 or IPv6 address: %s\n",
				    yystack.l_mark[0].v.string);
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}

			free(yystack.l_mark[0].v.string);
		}
break;
case 7:
#line 163 "/usr/src/contrib/openntpd/src/parse.y"
	{
			struct ntp_peer		*p;
			struct ntp_addr		*h, *next;

			h = yystack.l_mark[-1].v.addr->a;
			do {
				if (h != NULL) {
					next = h->next;
					if (h->ss.ss_family != AF_INET &&
					    h->ss.ss_family != AF_INET6) {
						yyerror("IPv4 or IPv6 address "
						    "or hostname expected");
						free(h);
						free(yystack.l_mark[-1].v.addr->name);
						free(yystack.l_mark[-1].v.addr);
						YYERROR;
					}
					h->next = NULL;
				} else
					next = NULL;

				p = new_peer();
				p->weight = yystack.l_mark[0].v.opts.weight;
				p->trusted = yystack.l_mark[0].v.opts.trusted;
				conf->trusted_peers = conf->trusted_peers ||
				    yystack.l_mark[0].v.opts.trusted;
				p->query_addr4 = query_addr4;
				p->query_addr6 = query_addr6;
				p->addr = h;
				p->addr_head.a = h;
				p->addr_head.pool = ++poolseqnum;
				p->addr_head.name = strdup(yystack.l_mark[-1].v.addr->name);
				if (p->addr_head.name == NULL)
					fatal(NULL);
				if (p->addr != NULL)
					p->state = STATE_DNS_DONE;
				TAILQ_INSERT_TAIL(&conf->ntp_peers, p, entry);
				h = next;
			} while (h != NULL);

			free(yystack.l_mark[-1].v.addr->name);
			free(yystack.l_mark[-1].v.addr);
		}
break;
case 8:
#line 206 "/usr/src/contrib/openntpd/src/parse.y"
	{
			struct ntp_peer		*p;
			struct ntp_addr		*h, *next;

			p = new_peer();
			for (h = yystack.l_mark[-1].v.addr->a; h != NULL; h = next) {
				next = h->next;
				if (h->ss.ss_family != AF_INET &&
				    h->ss.ss_family != AF_INET6) {
					yyerror("IPv4 or IPv6 address "
					    "or hostname expected");
					free(h);
					free(p);
					free(yystack.l_mark[-1].v.addr->name);
					free(yystack.l_mark[-1].v.addr);
					YYERROR;
				}
				h->next = p->addr;
				p->addr = h;
			}

			p->weight = yystack.l_mark[0].v.opts.weight;
			p->trusted = yystack.l_mark[0].v.opts.trusted;
			conf->trusted_peers = conf->trusted_peers ||
			    yystack.l_mark[0].v.opts.trusted;
			p->query_addr4 = query_addr4;
			p->query_addr6 = query_addr6;
			p->addr_head.a = p->addr;
			p->addr_head.pool = 0;
			p->addr_head.name = strdup(yystack.l_mark[-1].v.addr->name);
			if (p->addr_head.name == NULL)
				fatal(NULL);
			if (p->addr != NULL)
				p->state = STATE_DNS_DONE;
			TAILQ_INSERT_TAIL(&conf->ntp_peers, p, entry);
			free(yystack.l_mark[-1].v.addr->name);
			free(yystack.l_mark[-1].v.addr);
		}
break;
case 9:
#line 244 "/usr/src/contrib/openntpd/src/parse.y"
	{
			struct constraint	*p;
			struct ntp_addr		*h, *next;

			h = yystack.l_mark[0].v.addr->a;
			do {
				if (h != NULL) {
					next = h->next;
					if (h->ss.ss_family != AF_INET &&
					    h->ss.ss_family != AF_INET6) {
						yyerror("IPv4 or IPv6 address "
						    "or hostname expected");
						free(h);
						free(yystack.l_mark[0].v.addr->name);
						free(yystack.l_mark[0].v.addr->path);
						free(yystack.l_mark[0].v.addr);
						YYERROR;
					}
					h->next = NULL;
				} else
					next = NULL;

				p = new_constraint();
				p->addr = h;
				p->addr_head.a = h;
				p->addr_head.pool = ++poolseqnum;
				p->addr_head.name = strdup(yystack.l_mark[0].v.addr->name);
				p->addr_head.path = strdup(yystack.l_mark[0].v.addr->path);
				if (p->addr_head.name == NULL ||
				    p->addr_head.path == NULL)
					fatal(NULL);
				if (p->addr != NULL)
					p->state = STATE_DNS_DONE;
				constraint_add(p);
				h = next;
			} while (h != NULL);

			free(yystack.l_mark[0].v.addr->name);
			free(yystack.l_mark[0].v.addr);
		}
break;
case 10:
#line 284 "/usr/src/contrib/openntpd/src/parse.y"
	{
			struct constraint	*p;
			struct ntp_addr		*h, *next;

			p = new_constraint();
			for (h = yystack.l_mark[0].v.addr->a; h != NULL; h = next) {
				next = h->next;
				if (h->ss.ss_family != AF_INET &&
				    h->ss.ss_family != AF_INET6) {
					yyerror("IPv4 or IPv6 address "
					    "or hostname expected");
					free(h);
					free(p);
					free(yystack.l_mark[0].v.addr->name);
					free(yystack.l_mark[0].v.addr->path);
					free(yystack.l_mark[0].v.addr);
					YYERROR;
				}
				h->next = p->addr;
				p->addr = h;
			}

			p->addr_head.a = p->addr;
			p->addr_head.pool = 0;
			p->addr_head.name = strdup(yystack.l_mark[0].v.addr->name);
			p->addr_head.path = strdup(yystack.l_mark[0].v.addr->path);
			if (p->addr_head.name == NULL ||
			    p->addr_head.path == NULL)
				fatal(NULL);
			if (p->addr != NULL)
				p->state = STATE_DNS_DONE;
			constraint_add(p);
			free(yystack.l_mark[0].v.addr->name);
			free(yystack.l_mark[0].v.addr);
		}
break;
case 11:
#line 319 "/usr/src/contrib/openntpd/src/parse.y"
	{
			struct ntp_conf_sensor	*s;

			s = new_sensor(yystack.l_mark[-1].v.string);
			s->weight = yystack.l_mark[0].v.opts.weight;
			s->correction = yystack.l_mark[0].v.opts.correction;
			s->refstr = yystack.l_mark[0].v.opts.refstr;
			s->stratum = yystack.l_mark[0].v.opts.stratum;
			s->trusted = yystack.l_mark[0].v.opts.trusted;
			conf->trusted_sensors = conf->trusted_sensors ||
			    yystack.l_mark[0].v.opts.trusted;
			free(yystack.l_mark[-1].v.string);
			TAILQ_INSERT_TAIL(&conf->ntp_conf_sensors, s, entry);
		}
break;
case 12:
#line 335 "/usr/src/contrib/openntpd/src/parse.y"
	{
			if ((yyval.v.addr = calloc(1, sizeof(struct ntp_addr_wrap))) ==
			    NULL)
				fatal(NULL);
			host(yystack.l_mark[0].v.string, &yyval.v.addr->a);
			yyval.v.addr->name = yystack.l_mark[0].v.string;
		}
break;
case 13:
#line 344 "/usr/src/contrib/openntpd/src/parse.y"
	{
			struct ntp_addr *p, *q = NULL;
			struct in_addr ina;
			struct in6_addr in6a;

			if (inet_pton(AF_INET, yystack.l_mark[0].v.addr->name, &ina) != 1 &&
			    inet_pton(AF_INET6, yystack.l_mark[0].v.addr->name, &in6a) != 1) {
				yyerror("url can only be followed by IP "
				    "addresses");
				free(yystack.l_mark[0].v.addr->name);
				free(yystack.l_mark[0].v.addr);
				YYERROR;
			}
			p = yystack.l_mark[0].v.addr->a;
			while (p != NULL) {
				q = p;
				p = p->next;
			}
			if (q != NULL) {
				q->next = yystack.l_mark[-1].v.addr->a;
				yystack.l_mark[-1].v.addr->a = yystack.l_mark[0].v.addr->a;
				free(yystack.l_mark[0].v.addr);
			}
			yyval.v.addr = yystack.l_mark[-1].v.addr;
		}
break;
case 14:
#line 369 "/usr/src/contrib/openntpd/src/parse.y"
	{
			yyval.v.addr = yystack.l_mark[0].v.addr;
		}
break;
case 15:
#line 374 "/usr/src/contrib/openntpd/src/parse.y"
	{
			char	*hname, *path;

			if ((yyval.v.addr = calloc(1, sizeof(struct ntp_addr_wrap))) ==
			    NULL)
				fatal("calloc");

			if (strncmp("https://", yystack.l_mark[0].v.string,
			    strlen("https://")) != 0) {
				host(yystack.l_mark[0].v.string, &yyval.v.addr->a);
				yyval.v.addr->name = yystack.l_mark[0].v.string;
			} else {
				hname = yystack.l_mark[0].v.string + strlen("https://");

				path = hname + strcspn(hname, "/\\");
				if (*path != '\0') {
					if ((yyval.v.addr->path = strdup(path)) == NULL)
						fatal("strdup");
					*path = '\0';
				}
				host(hname, &yyval.v.addr->a);
				if ((yyval.v.addr->name = strdup(hname)) == NULL)
					fatal("strdup");
			}
			if (yyval.v.addr->path == NULL &&
			    (yyval.v.addr->path = strdup("/")) == NULL)
				fatal("strdup");
		}
break;
case 16:
#line 404 "/usr/src/contrib/openntpd/src/parse.y"
	{ opts_default(); }
break;
case 17:
#line 406 "/usr/src/contrib/openntpd/src/parse.y"
	{ yyval.v.opts = opts; }
break;
case 18:
#line 407 "/usr/src/contrib/openntpd/src/parse.y"
	{ opts_default(); yyval.v.opts = opts; }
break;
case 22:
#line 415 "/usr/src/contrib/openntpd/src/parse.y"
	{ opts_default(); }
break;
case 23:
#line 417 "/usr/src/contrib/openntpd/src/parse.y"
	{ yyval.v.opts = opts; }
break;
case 24:
#line 418 "/usr/src/contrib/openntpd/src/parse.y"
	{ opts_default(); yyval.v.opts = opts; }
break;
case 29:
#line 427 "/usr/src/contrib/openntpd/src/parse.y"
	{ opts_default(); }
break;
case 30:
#line 429 "/usr/src/contrib/openntpd/src/parse.y"
	{ yyval.v.opts = opts; }
break;
case 31:
#line 430 "/usr/src/contrib/openntpd/src/parse.y"
	{ opts_default(); yyval.v.opts = opts; }
break;
case 39:
#line 442 "/usr/src/contrib/openntpd/src/parse.y"
	{
			if (yystack.l_mark[0].v.number < -127000000 || yystack.l_mark[0].v.number > 127000000) {
				yyerror("correction must be between "
				    "-127000000 and 127000000 microseconds");
				YYERROR;
			}
			opts.correction = yystack.l_mark[0].v.number;
		}
break;
case 40:
#line 452 "/usr/src/contrib/openntpd/src/parse.y"
	{
			size_t l = strlen(yystack.l_mark[0].v.string);

			if (l < 1 || l > 4) {
				yyerror("refid must be 1 to 4 characters");
				free(yystack.l_mark[0].v.string);
				YYERROR;
			}
			opts.refstr = yystack.l_mark[0].v.string;
		}
break;
case 41:
#line 464 "/usr/src/contrib/openntpd/src/parse.y"
	{
			if (yystack.l_mark[0].v.number < 1 || yystack.l_mark[0].v.number > 15) {
				yyerror("stratum must be between "
				    "1 and 15");
				YYERROR;
			}
			opts.stratum = yystack.l_mark[0].v.number;
		}
break;
case 42:
#line 474 "/usr/src/contrib/openntpd/src/parse.y"
	{
			if (yystack.l_mark[0].v.number < 1 || yystack.l_mark[0].v.number > 10) {
				yyerror("weight must be between 1 and 10");
				YYERROR;
			}
			opts.weight = yystack.l_mark[0].v.number;
		}
break;
case 43:
#line 481 "/usr/src/contrib/openntpd/src/parse.y"
	{
#ifdef RT_TABLEID_MAX
			if (yystack.l_mark[0].v.number < 0 || yystack.l_mark[0].v.number > RT_TABLEID_MAX) {
				yyerror("rtable must be between 1"
				    " and RT_TABLEID_MAX");
				YYERROR;
			}
#endif
			opts.rtable = yystack.l_mark[0].v.number;
		}
break;
case 44:
#line 493 "/usr/src/contrib/openntpd/src/parse.y"
	{
			opts.trusted = 1;
		}
break;
#line 1330 "y.tab.c"
    }
    yystack.s_mark -= yym;
    yystate = *yystack.s_mark;
    yystack.l_mark -= yym;
    yym = yylhs[yyn];
    if (yystate == 0 && yym == 0)
    {
#if YYDEBUG
        if (yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        yystate = YYFINAL;
        *++yystack.s_mark = YYFINAL;
        *++yystack.l_mark = yyval;
        if (yychar < 0)
        {
            yychar = YYLEX;
            if (yychar < 0) yychar = YYEOF;
#if YYDEBUG
            if (yydebug)
            {
                if ((yys = yyname[YYTRANSLATE(yychar)]) == NULL) yys = yyname[YYUNDFTOKEN];
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, yychar, yys);
            }
#endif
        }
        if (yychar == YYEOF) goto yyaccept;
        goto yyloop;
    }
    if (((yyn = yygindex[yym]) != 0) && (yyn += yystate) >= 0 &&
            yyn <= YYTABLESIZE && yycheck[yyn] == (YYINT) yystate)
        yystate = yytable[yyn];
    else
        yystate = yydgoto[yym];
#if YYDEBUG
    if (yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *yystack.s_mark, yystate);
#endif
    if (yystack.s_mark >= yystack.s_last && yygrowstack(&yystack) == YYENOMEM) goto yyoverflow;
    *++yystack.s_mark = (YYINT) yystate;
    *++yystack.l_mark = yyval;
    goto yyloop;

yyoverflow:
    YYERROR_CALL("yacc stack overflow");

yyabort:
    yyfreestack(&yystack);
    return (1);

yyaccept:
    yyfreestack(&yystack);
    return (0);
}
