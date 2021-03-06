#pragma once

/* Project level definitions and paths */
#define ETC_PREFIX "/etc"
#define VERSION "2.1.3"

/* Define our key hints to the source tree...  */
#define HASDIRENT 1
#define HASPOLL 1
#define HASMKFIFO 1
#define HASFLOCK 1
#define HASSIGPROCMASK 1
#define HASSIGACTION 1
#define HASWAITPID 1
#define HASSYSSELECT 1
/* #undef HASUTMP */
#define HASUTMPX 1

/* For now, declare what uint64 is...  (Shouldn't really NEED to do this
   with any *MODERN* os, but...) */
typedef unsigned long uint64;
