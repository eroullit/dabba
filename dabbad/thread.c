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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif				/* _GNU_SOURCE */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

#include <dabbad/dabbad.h>
#include <dabbad/thread.h>
#include <libdabba/macros.h>

/**
 * \internal
 * \brief Packet thread management list
 */

static TAILQ_HEAD(packet_thread_head, packet_thread) packet_thread_head =
TAILQ_HEAD_INITIALIZER(packet_thread_head);

static struct packet_thread *dabbad_thread_first(void)
{
	return TAILQ_FIRST(&packet_thread_head);
}

static struct packet_thread *dabbad_thread_next(struct packet_thread
						*pkt_thread)
{
	return pkt_thread ? TAILQ_NEXT(pkt_thread, entry) : NULL;
}

/**
 * \brief Get the first thread from the thread list matching the input thread type
 * \param[in] type type of the next running thread to get
 * \return 	Pointer to the first thread element,
 * 		NULL when no thread are currently running
 */

struct packet_thread *dabbad_thread_type_first(const enum packet_thread_type
					       type)
{
	struct packet_thread *node;

	for (node = dabbad_thread_first(); node;
	     node = dabbad_thread_next(node))
		if (node->type == type)
			break;

	return node;
}

/**
 * \brief Get the next thread from the thread list matching the input thread type
 * \param[in] pkt_thread xurrent thread entry
 * \param[in] type type of the next running thread to get
 * \return 	Pointer to the next thread element,
 * 		NULL when \c pkt_thread was the last element
 */

struct packet_thread *dabbad_thread_type_next(struct packet_thread *pkt_thread,
					      const enum packet_thread_type
					      type)
{
	for (pkt_thread = dabbad_thread_next(pkt_thread); pkt_thread;
	     pkt_thread = dabbad_thread_next(pkt_thread))
		if (pkt_thread->type == type)
			break;

	return pkt_thread;
}

/**
 * \brief Get the thread information from an existing pthread id
 * \param[in] thread_id pthread id to search
 * \return Pointer to the corresponding thread element, NULL when not found.
 */

struct packet_thread *dabbad_thread_data_get(const pthread_t thread_id)
{
	struct packet_thread *node;

	TAILQ_FOREACH(node, &packet_thread_head, entry)
	    if (pthread_equal(thread_id, node->id))
		break;

	return node;
}

/**
 * \brief Set the thread scheduling policy and priority
 * \param[in] pkt_thread thread to modify
 * \param[in] sched_prio new schduling priority to set
 * \param[in] sched_policy new schduling policy to set
 * \return return value of \c pthread_setschedparam(3)
 */

int dabbad_thread_sched_param_set(struct packet_thread *pkt_thread,
				  const int16_t sched_prio,
				  const int16_t sched_policy)
{
	struct sched_param sp = { 0 };

	assert(pkt_thread);

	sp.sched_priority = sched_prio;

	return pthread_setschedparam(pkt_thread->id, sched_policy, &sp);
}

/**
 * \brief Get the thread current scheduling policy and priority
 * \param[in] pkt_thread thread to get information from
 * \param[out] sched_prio thread's schduling priority
 * \param[out] sched_policy thread's schduling policy
 * \return return value of \c pthread_getschedparam(3)
 */

int dabbad_thread_sched_param_get(struct packet_thread *pkt_thread,
				  int16_t * sched_prio, int16_t * sched_policy)
{
	int rc;
	struct sched_param sp = { 0 };

	assert(pkt_thread);
	assert(sched_prio);
	assert(sched_policy);

	rc = pthread_getschedparam(pkt_thread->id, (int *)sched_policy, &sp);

	*sched_prio = sp.sched_priority;

	return rc;
}

/**
 * \brief Set the thread new CPU affinity
 * \param[in] pkt_thread thread to modify
 * \param[in] run_on thread's CPU affinity
 * \return return value of \c pthread_setaffinity_np(3)
 */

int dabbad_thread_sched_affinity_set(struct packet_thread *pkt_thread,
				     cpu_set_t * run_on)
{
	assert(pkt_thread);
	assert(run_on);

	return pthread_setaffinity_np(pkt_thread->id, sizeof(*run_on), run_on);
}

/**
 * \brief Get the thread current CPU affinity
 * \param[in] pkt_thread thread to get information from
 * \param[out] run_on thread's CPU affinity
 * \return return value of \c pthread_getaffinity_np(3)
 */

int dabbad_thread_sched_affinity_get(struct packet_thread *pkt_thread,
				     cpu_set_t * run_on)
{
	assert(pkt_thread);
	assert(run_on);

	return pthread_getaffinity_np(pkt_thread->id, sizeof(*run_on), run_on);
}

/**
 * \brief Start a new thread
 * \param[in] pkt_thread thread information
 * \param[in] func function to start as a thread
 * \return return value of \c pthread_create(3) or \¢ pthread_detach(3)
 */

int dabbad_thread_start(struct packet_thread *pkt_thread,
			void *(*func) (void *arg), void *arg)
{
	int rc;

	assert(pkt_thread);
	assert(func);

	rc = pthread_create(&pkt_thread->id, NULL, func, arg);

	if (!rc)
		rc = pthread_detach(pkt_thread->id);

	if (!rc)
		TAILQ_INSERT_TAIL(&packet_thread_head, pkt_thread, entry);

	return rc;
}

/**
 * \brief Stop a running thread
 * \param[in] pkt_thread running thread to stop
 * \return return value of \c pthread_cancel(3) or \¢ EINVAL if thread could not be found
 */

int dabbad_thread_stop(struct packet_thread *pkt_thread)
{
	struct packet_thread *node;
	int rc;

	assert(pkt_thread);

	TAILQ_FOREACH(node, &packet_thread_head, entry)
	    if (pthread_equal(pkt_thread->id, node->id))
		break;

	if (!node)
		return EINVAL;

	rc = pthread_cancel(node->id);

	if (!rc) {
		TAILQ_REMOVE(&packet_thread_head, node, entry);
	}

	return rc;
}

/**
 * \brief List currently running thread
 * \param[in,out] msg Thread message
 * \return 0 on success, else on failure.
 */

int dabbad_thread_list(struct dabba_ipc_msg *msg)
{
	struct dabba_thread *thread_msg;
	struct packet_thread *pkt_thread;
	size_t a = 0, off = 0, thread_list_size;

	thread_msg = msg->msg_body.msg.thread;
	thread_list_size = ARRAY_SIZE(msg->msg_body.msg.thread);

	for (pkt_thread = dabbad_thread_first(); pkt_thread;
	     pkt_thread = dabbad_thread_next(pkt_thread)) {
		if (off < msg->msg_body.offset) {
			off++;
			continue;
		}

		if (a >= thread_list_size)
			break;

		thread_msg[a].id = pkt_thread->id;
		thread_msg[a].type = pkt_thread->type;
		dabbad_thread_sched_param_get(pkt_thread,
					      &thread_msg[a].sched_prio,
					      &thread_msg[a].sched_policy);
		dabbad_thread_sched_affinity_get(pkt_thread,
						 &thread_msg[a].cpu);

		a++;
	}

	msg->msg_body.elem_nr = a;

	return 0;
}

/**
 * \brief Modify a currently running thread
 * \param[in,out] msg Thread message
 * \return 0 on success, else on failure.
 */

int dabbad_thread_modify(struct dabba_ipc_msg *msg)
{
	struct packet_thread *pkt_thread;
	struct dabba_thread *thread_msg = msg->msg_body.msg.thread;
	int rc = 0;
	int16_t cur_sched_prio, cur_sched_policy;

	pkt_thread = dabbad_thread_data_get(thread_msg->id);

	if (!pkt_thread)
		return EINVAL;

	/* Get current scheduling parameters */
	dabbad_thread_sched_param_get(pkt_thread, &cur_sched_prio,
				      &cur_sched_policy);

	/* Update the new values the user gave use */
	if ((thread_msg->usage_flags & USE_SCHED_PRIO) == USE_SCHED_PRIO)
		cur_sched_prio = thread_msg->sched_prio;

	if ((thread_msg->usage_flags & USE_SCHED_POLICY) == USE_SCHED_POLICY)
		cur_sched_policy = thread_msg->sched_policy;

	/* Set the scheduling values if the user wanted to update one of them */
	if ((thread_msg->usage_flags & USE_SCHED_PRIO) == USE_SCHED_PRIO
	    || (thread_msg->usage_flags & USE_SCHED_POLICY) ==
	    USE_SCHED_POLICY) {
		rc = dabbad_thread_sched_param_set(pkt_thread, cur_sched_prio,
						   cur_sched_policy);

		if (rc)
			return rc;
	}

	if ((thread_msg->usage_flags & USE_CPU_MASK) == USE_CPU_MASK) {
		rc = dabbad_thread_sched_affinity_set(pkt_thread,
						      &thread_msg->cpu);

		if (rc)
			return rc;
	}

	return 0;
}

/**
 * \brief Get a currently thread scheduling capabilities
 * \param[in,out] msg Thread message
 * \return Always return 0.
 */

int dabbad_thread_cap_list(struct dabba_ipc_msg *msg)
{
	struct dabba_thread_cap *thread_cap_msg = msg->msg_body.msg.thread_cap;

	if (msg->msg_body.offset < 3) {
		thread_cap_msg[0].policy = SCHED_FIFO;
		thread_cap_msg[0].prio_min = sched_get_priority_min(SCHED_FIFO);
		thread_cap_msg[0].prio_max = sched_get_priority_max(SCHED_FIFO);

		thread_cap_msg[1].policy = SCHED_RR;
		thread_cap_msg[1].prio_min = sched_get_priority_min(SCHED_RR);
		thread_cap_msg[1].prio_max = sched_get_priority_max(SCHED_RR);

		thread_cap_msg[2].policy = SCHED_OTHER;
		thread_cap_msg[2].prio_min =
		    sched_get_priority_min(SCHED_OTHER);
		thread_cap_msg[2].prio_max =
		    sched_get_priority_max(SCHED_OTHER);

		msg->msg_body.elem_nr = 3;
	} else {
		msg->msg_body.elem_nr = 0;
	}

	return 0;
}