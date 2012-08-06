/**
 * \file thread.h
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

#ifndef THREAD_H
#define	THREAD_H

#include <stdint.h>
#include <sys/queue.h>
#include <pthread.h>

enum packet_thread_type {
	CAPTURE_THREAD
};

/**
 * \brief Packet thread structure
 */

struct packet_thread {
	pthread_t id;
	pthread_attr_t attributes;
	enum packet_thread_type type;
	 TAILQ_ENTRY(packet_thread) entry;
};

int thread_sched_prio_set(struct packet_thread *pkt_thread,
			  const int16_t sched_prio);
int thread_sched_prio_get(struct packet_thread *pkt_thread,
			  int16_t * sched_prio);
int thread_sched_policy_set(struct packet_thread *pkt_thread,
			    const int16_t sched_policy);
int thread_sched_policy_get(struct packet_thread *pkt_thread,
			    int16_t * sched_policy);
int thread_sched_affinity_set(struct packet_thread *pkt_thread, cpu_set_t *run_on);
int thread_sched_affinity_get(struct packet_thread *pkt_thread, cpu_set_t *run_on);

#endif				/* THREAD_H */
