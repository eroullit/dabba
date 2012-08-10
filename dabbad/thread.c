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

struct packet_thread *dabbad_thread_data_get(const pthread_t thread_id)
{
	struct packet_thread *node;

	TAILQ_FOREACH(node, &packet_thread_head, entry)
	    if (pthread_equal(thread_id, node->id))
		break;

	return node;
}

int dabbad_thread_sched_param_set(struct packet_thread *pkt_thread,
				  const int16_t sched_prio,
				  const int16_t sched_policy)
{
	struct sched_param sp = { 0 };

	assert(pkt_thread);

	sp.sched_priority = sched_prio;

	return pthread_setschedparam(pkt_thread->id, sched_policy, &sp);
}

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

int dabbad_thread_sched_affinity_set(struct packet_thread *pkt_thread,
				     cpu_set_t * run_on)
{
	assert(pkt_thread);
	assert(run_on);

	return pthread_setaffinity_np(pkt_thread->id, sizeof(*run_on), run_on);
}

int dabbad_thread_sched_affinity_get(struct packet_thread *pkt_thread,
				     cpu_set_t * run_on)
{
	assert(pkt_thread);
	assert(run_on);

	return pthread_getaffinity_np(pkt_thread->id, sizeof(*run_on), run_on);
}

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
