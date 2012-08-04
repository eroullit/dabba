/**
 * \file thread.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2012
 * \date 2012
 */

/* __LICENSE_HEADER_BEGIN__ */

/*
 * Copyright (C) 2012	Emmanuel Roullit <emmanuel.roullit@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110, USA
 *
 */

 /* __LICENSE_HEADER_END__ */

#include <assert.h>
#include <sched.h>
#include <pthread.h>

#include <libdabba/thread.h>

int thread_sched_prio_set(struct packet_thread *pkt_thread,
			  const int16_t sched_prio)
{
	struct sched_param sp = {.sched_priority = sched_prio };

	assert(pkt_thread);

	return pthread_attr_setschedparam(&pkt_thread->attributes, &sp);
}

int thread_sched_prio_get(struct packet_thread *pkt_thread,
			  int16_t * sched_prio)
{
	int rc;
	struct sched_param sp = { 0 };

	assert(pkt_thread);
	assert(sched_prio);

	rc = pthread_attr_getschedparam(&pkt_thread->attributes, &sp);

	*sched_prio = sp.sched_priority;

	return rc;
}

int thread_sched_policy_set(struct packet_thread *pkt_thread,
			    const int16_t sched_policy)
{
	assert(pkt_thread);

	return pthread_attr_setschedpolicy(&pkt_thread->attributes,
					   sched_policy);
}

int thread_sched_policy_get(struct packet_thread *pkt_thread,
			    int16_t * sched_policy)
{
	assert(pkt_thread);
	assert(sched_policy);

	return pthread_attr_getschedpolicy(&pkt_thread->attributes,
					   (int *)sched_policy);
}
