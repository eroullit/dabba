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
	    if (thread_id == node->id)
		break;

	return node;
}

int dabbad_thread_start(struct packet_thread *pkt_thread,
			void *(*func) (void *arg), void *arg)
{
	int rc;

	assert(pkt_thread);
	assert(func);

	rc = pthread_create(&pkt_thread->id, &pkt_thread->attributes, func,
			    arg);

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
	    if (pkt_thread->id == node->id)
		break;

	if (!node)
		return EINVAL;

	rc = pthread_cancel(node->id);

	if (!rc)
		TAILQ_REMOVE(&packet_thread_head, node, entry);

	return rc;
}
