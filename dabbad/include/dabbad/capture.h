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
#include <libdabba-rpc/dabba.pb-c.h>

struct packet_capture_thread {
	struct packet_rx rx;
	struct packet_thread thread;
};

struct packet_thread *dabbad_capture_thread_data_get(const pthread_t thread_id);

//int dabbad_capture_start(struct dabba_ipc_msg *msg);
int dabbad_capture_list(struct dabba_ipc_msg *msg);
//int dabbad_capture_stop(struct dabba_ipc_msg *msg);

void dabbad_capture_stop(Dabba__DabbaService_Service * service,
			 const Dabba__ThreadId * idp,
			 Dabba__Dummy_Closure closure, void *closure_data);

void dabbad_capture_start(Dabba__DabbaService_Service * service,
			  const Dabba__CaptureSettings * capturep,
			  Dabba__Dummy_Closure closure, void *closure_data);

void dabbad_capture_settings_get(Dabba__DabbaService_Service * service,
				 const Dabba__ThreadIdList * id_listp,
				 Dabba__CaptureSettingsList_Closure closure,
				 void *closure_data);

#endif				/* CAPTURE_H */
