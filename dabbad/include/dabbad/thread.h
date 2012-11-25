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

#ifndef DABBAD_THREAD_H
#define	DABBAD_THREAD_H

#include <assert.h>
#include <sched.h>
#include <pthread.h>
#include <sys/queue.h>
#include <libdabba-rpc/dabba.pb-c.h>

enum packet_thread_type {
	CAPTURE_THREAD
};

/**
 * \brief Packet thread structure
 */

struct packet_thread {
	pthread_t id;
	enum packet_thread_type type;
	 TAILQ_ENTRY(packet_thread) entry;
};

struct packet_thread *dabbad_thread_type_first(const enum packet_thread_type
					       type);
struct packet_thread *dabbad_thread_type_next(struct packet_thread *pkt_thread,
					      const enum packet_thread_type
					      type);
struct packet_thread *dabbad_thread_data_get(const pthread_t thread_id);
int dabbad_thread_sched_param_set(struct packet_thread *pkt_thread,
				  const int16_t sched_prio,
				  const int16_t sched_policy);
int dabbad_thread_sched_param_get(struct packet_thread *pkt_thread,
				  int16_t * sched_prio, int16_t * sched_policy);
int dabbad_thread_sched_affinity_set(struct packet_thread *pkt_thread,
				     cpu_set_t * run_on);
int dabbad_thread_sched_affinity_get(struct packet_thread *pkt_thread,
				     cpu_set_t * run_on);
int dabbad_thread_start(struct packet_thread *pkt_thread,
			void *(*func) (void *arg), void *arg);
int dabbad_thread_stop(struct packet_thread *pkt_thread);
int dabbad_thread_list(struct dabba_ipc_msg *msg);
int dabbad_thread_modify(struct dabba_ipc_msg *msg);
int dabbad_thread_cap_list(struct dabba_ipc_msg *msg);

void dabbad_thread_id_get(Dabba__DabbaService_Service * service,
			  const Dabba__Dummy * dummy,
			  Dabba__ThreadIdList_Closure closure,
			  void *closure_data);

void dabbad_thread_settings_get(Dabba__DabbaService_Service * service,
				const Dabba__ThreadIdList * id_listp,
				Dabba__ThreadSettingsList_Closure closure,
				void *closure_data);

void dabbad_thread_capabilities_get(Dabba__DabbaService_Service * service,
				    const Dabba__Dummy * dummy,
				    Dabba__ThreadCapabilitiesList_Closure
				    closure, void *closure_data);

#endif				/* DABBAD_THREAD_H */
