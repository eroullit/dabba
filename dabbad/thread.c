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
#include <sys/queue.h>

#include <dabbad/thread.h>
#include <libdabba/macros.h>

/**
 * \brief Capture thread management element
 */

struct packet_thread_node {
	TAILQ_ENTRY(packet_thread_node) entry;	/**< tail queue list entry */
	struct packet_thread *thread;  /**< Packet thread */
};

/**
 * \internal
 * \brief Packet thread management list
 */

static TAILQ_HEAD(packet_thread_head, packet_thread_node) packet_thread_head =
TAILQ_HEAD_INITIALIZER(packet_thread_head);

struct packet_thread *dabbad_thread_first(void)
{
	struct packet_thread_node *node = TAILQ_FIRST(&packet_thread_head);
	return node ? node->thread : NULL;
}

struct packet_thread *dabbad_thread_next(struct packet_thread *pkt_thread)
{
	struct packet_thread_node *node = NULL, *next = NULL;
	node = container_of(&pkt_thread, struct packet_thread_node, thread);

	if (node)
		next = TAILQ_NEXT(node, entry);

	return next ? next->thread : NULL;
}

struct packet_thread *dabbad_thread_data_get(const pthread_t thread_id)
{
	struct packet_thread_node *node;

	TAILQ_FOREACH(node, &packet_thread_head, entry)
	    if (thread_id == node->thread->id)
		break;

	return node ? node->thread : NULL;
}

int dabbad_thread_start(struct packet_thread *pkt_thread,
			void *(*func) (void *arg), void *arg)
{
	struct packet_thread_node *node;
	int rc;

	assert(pkt_thread);
	assert(func);

	node = calloc(1, sizeof(*node));

	if (!node)
		return ENOMEM;

	rc = pthread_create(&pkt_thread->id, &pkt_thread->attributes, func,
			    arg);

	if (rc) {
		free(node);
	} else {
		node->thread = pkt_thread;
		TAILQ_INSERT_TAIL(&packet_thread_head, node, entry);
	}

	return rc;
}

int dabbad_thread_stop(struct packet_thread *pkt_thread)
{
	struct packet_thread_node *node;
	int rc;

	assert(pkt_thread);

	TAILQ_FOREACH(node, &packet_thread_head, entry)
	    if (pkt_thread->id == node->thread->id)
		break;

	if (!node)
		return EINVAL;

	rc = pthread_cancel(node->thread->id);

	if (!rc) {
		TAILQ_REMOVE(&packet_thread_head, node, entry);
		free(node);
	}

	return rc;
}
