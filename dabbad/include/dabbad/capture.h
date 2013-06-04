/**
 * \file capture.h
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

#ifndef CAPTURE_H
#define	CAPTURE_H

#include <dabbad/thread.h>
#include <libdabba/packet_rx.h>
#include <libdabba-rpc/rpc.h>

/**
 * \brief Structure representing a capture thread
 */

struct packet_capture {
	struct packet_rx rx; /**< packet capture structure */
	struct packet_thread thread; /**< thread structure */
	 TAILQ_ENTRY(packet_capture) entry;/**< capture entry */
};

struct packet_thread *dabbad_capture_thread_data_get(const pthread_t thread_id);

void dabbad_capture_stop(Dabba__DabbaService_Service * service,
			 const Dabba__ThreadId * idp,
			 Dabba__ErrorCode_Closure closure, void *closure_data);

void dabbad_capture_start(Dabba__DabbaService_Service * service,
			  const Dabba__Capture * capturep,
			  Dabba__ErrorCode_Closure closure, void *closure_data);

void dabbad_capture_get(Dabba__DabbaService_Service * service,
			const Dabba__ThreadIdList * id_listp,
			Dabba__CaptureList_Closure closure, void *closure_data);

void dabbad_capture_stop_all(Dabba__DabbaService_Service * service,
			     const Dabba__Dummy * dummyp,
			     Dabba__ErrorCode_Closure closure,
			     void *closure_data);

#endif				/* CAPTURE_H */
