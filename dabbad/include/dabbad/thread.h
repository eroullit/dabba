/**
 * \file thread.h
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */


#ifndef DABBAD_THREAD_H
#define	DABBAD_THREAD_H

#include <assert.h>
#include <sched.h>
#include <pthread.h>
#include <sys/queue.h>
#include <libdabba-rpc/rpc.h>

/**
 * \brief Supported thread types
 */

enum packet_thread_type {
	CAPTURE_THREAD,
	REPLAY_THREAD
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

void dabbad_thread_modify(Dabba__DabbaService_Service * service,
			  const Dabba__Thread * thread,
			  Dabba__ErrorCode_Closure closure, void *closure_data);

void dabbad_thread_get(Dabba__DabbaService_Service * service,
		       const Dabba__ThreadIdList * id_listp,
		       Dabba__ThreadList_Closure closure, void *closure_data);

void dabbad_thread_capabilities_get(Dabba__DabbaService_Service * service,
				    const Dabba__Dummy * dummy,
				    Dabba__ThreadCapabilitiesList_Closure
				    closure, void *closure_data);

#endif				/* DABBAD_THREAD_H */
