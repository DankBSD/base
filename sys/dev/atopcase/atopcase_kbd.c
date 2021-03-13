
#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/conf.h>
#include <sys/uio.h>
#include <sys/bus.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/module.h>
#include <sys/lock.h>
#include <sys/taskqueue.h>
#include <sys/selinfo.h>
#include <sys/poll.h>
#include <sys/proc.h>
#include <sys/queue.h>
#include <sys/kthread.h>
#include <sys/syscallsubr.h>
#include <sys/sysproto.h>
#include <sys/signal.h>
#include <sys/syslog.h>
#include <sys/systm.h>
#include <sys/mutex.h>
#include <sys/callout.h>

#include <sys/kbio.h>
#include <dev/kbd/kbdreg.h>
#include <dev/kbd/kbdtables.h>

