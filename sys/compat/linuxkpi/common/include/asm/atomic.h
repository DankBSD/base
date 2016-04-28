/*-
 * Copyright (c) 2010 Isilon Systems, Inc.
 * Copyright (c) 2010 iX Systems, Inc.
 * Copyright (c) 2010 Panasas, Inc.
 * Copyright (c) 2013-2016 Mellanox Technologies, Ltd.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $FreeBSD$
 */
#ifndef	_ASM_ATOMIC_H_
#define	_ASM_ATOMIC_H_

#include <sys/cdefs.h>
#include <sys/types.h>
#include <machine/atomic.h>

#define	ATOMIC_INIT(x)	{ .counter = (x) }

typedef struct {
	volatile u_int counter;
} atomic_t;

#define ATOMIC64_INIT(i)	{ (i) }

/*------------------------------------------------------------------------*
 *	32-bit atomic operations
 *------------------------------------------------------------------------*/

#define	atomic_add(i, v)		atomic_add_return((i), (v))
#define	atomic_sub(i, v)		atomic_sub_return((i), (v))
#define	atomic_inc_return(v)		atomic_add_return(1, (v))
#define	atomic_add_negative(i, v)	(atomic_add_return((i), (v)) < 0)
#define	atomic_add_and_test(i, v)	(atomic_add_return((i), (v)) == 0)
#define	atomic_sub_and_test(i, v)	(atomic_sub_return((i), (v)) == 0)
#define	atomic_dec_and_test(v)		(atomic_sub_return(1, (v)) == 0)
#define	atomic_inc_and_test(v)		(atomic_add_return(1, (v)) == 0)
#define	atomic_dec_return(v)		atomic_sub_return(1, (v))
#define	atomic_inc_not_zero(v)		atomic_add_unless((v), 1, 0)

static inline int
atomic_add_return(int i, atomic_t *v)
{
	return i + atomic_fetchadd_int(&v->counter, i);
}

static inline int
atomic_sub_return(int i, atomic_t *v)
{
	return atomic_fetchadd_int(&v->counter, -i) - i;
}

static inline void
atomic_set(atomic_t *v, int i)
{
	atomic_store_rel_int(&v->counter, i);
}


static inline void
atomic_set_mask(int mask, atomic_t *v)
{
	atomic_set_int(&v->counter, mask);
}


static inline int
atomic_read(atomic_t *v)
{
	return atomic_load_acq_int(&v->counter);
}

static inline long
atomic_long_read(atomic64_t *v)
{
	return atomic_load_acq_long(&v->counter);
}

static inline int
atomic_inc(atomic_t *v)
{
	return atomic_fetchadd_int(&v->counter, 1) + 1;
}

static inline int
atomic_dec(atomic_t *v)
{
	return atomic_fetchadd_int(&v->counter, -1) - 1;
}

static inline int
atomic_add_unless(atomic_t *v, int a, int u)
{
	int c;

	for (;;) {
		c = atomic_read(v);
		if (unlikely(c == u))
			break;
		if (likely(atomic_cmpset_int(&v->counter, c, c + a)))
			break;
	}
	return (c != u);
}

static inline void
atomic_clear_mask(unsigned int mask, atomic_t *v)
{

	atomic_clear_int(&v->counter, mask);
}

static inline int
atomic_xchg(atomic_t *v, int i)
{

	return (atomic_swap_int(&v->counter, i));
}


static inline uint64_t
atomic64_read(atomic64_t *v)
{
	return atomic_load_acq_long(&v->counter);
}


#define	cmpxchg(ptr, old, new) \
    (atomic_cmpset_int((volatile u_int *)(ptr),(old),(new)) ? (old) : (0))

#define	atomic64_cmpxchg(ptr, old, new) \
    (atomic_cmpset_long((volatile uint64_t *)(ptr),(old),(new)) ? (old) : (0))

static inline int atomic64_add_unless(atomic64_t *v, long a, long u)
{
	long c, old;
	c = atomic64_read(v);
	for (;;) {
		if (unlikely(c == (u)))
			break;
		old = atomic64_cmpxchg((v), c, c + (a));
		if (likely(old == c))
			break;
		c = old;
	}
	return c != (u);
}

#define atomic64_inc_not_zero(v) atomic64_add_unless((v), 1, 0)

#define atomic_long_inc_not_zero atomic64_inc_not_zero
#include <asm-generic/atomic.h>
#endif					/* _ASM_ATOMIC_H_ */
