/* Copyright (C) 2010  Pierre-Marc Fournier
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef _UST_CLOCK_H
#define _UST_CLOCK_H

#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <stddef.h>
#include <ust/core.h>

/* TRACE CLOCK */

/* There are two types of clocks that can be used.
   - TSC based clock
   - gettimeofday() clock

   Microbenchmarks on Linux 2.6.30 on Core2 Duo 3GHz (functions are inlined):
	 Calls (100000000) to tsc(): 4004035641 cycles or 40 cycles/call
	 Calls (100000000) to gettimeofday(): 9723158352 cycles or 97 cycles/call

   For merging traces with the kernel, a time source compatible with that of
   the kernel is necessary.

   Instead of gettimeofday(), we are now using clock_gettime for better
   precision and monotonicity.
*/

/* Only available for x86 arch */
#define CLOCK_TRACE_FREQ  14
#define CLOCK_TRACE  15
union lttng_timespec {
	struct timespec ts;
	uint64_t lttng_ts;
};

extern int ust_clock_source;

/* Choosing correct trace clock */

static __inline__ uint64_t trace_clock_read64(void)
{
	struct timespec ts;
	uint64_t retval;
	union lttng_timespec *lts = (union lttng_timespec *) &ts;

	clock_gettime(ust_clock_source, &ts);
	/*
	 * Clock source can change when loading the binary (tracectl.c)
	 * so we must check if the clock source has changed before
	 * returning the correct value
	 */
	if (likely(ust_clock_source == CLOCK_TRACE)) {
		retval = lts->lttng_ts;
	} else { /* CLOCK_MONOTONIC */
		retval = ts.tv_sec;
		retval *= 1000000000;
		retval += ts.tv_nsec;
	}

	return retval;
}

#if __i386__ || __x86_64__
static __inline__ uint64_t trace_clock_frequency(void)
{
	struct timespec ts;
	union lttng_timespec *lts = (union lttng_timespec *) &ts;

	if (likely(ust_clock_source == CLOCK_TRACE)) {
		clock_gettime(CLOCK_TRACE_FREQ, &ts);
		return lts->lttng_ts;
	}
	return 1000000000LL;
}
#else /* #if __i386__ || __x86_64__ */
static __inline__ uint64_t trace_clock_frequency(void)
{
	return 1000000000LL;
}
#endif /* #else #if __i386__ || __x86_64__ */

static __inline__ uint32_t trace_clock_freq_scale(void)
{
	return 1;
}

#endif /* _UST_CLOCK_H */
