/**
 * \file replay.h
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */


#ifndef REPLAY_H
#define	REPLAY_H

#include <dabbad/thread.h>
#include <libdabba/packet-tx.h>
#include <libdabba-rpc/rpc.h>

/**
 * \brief Structure representing a replay thread
 */

struct packet_replay {
	struct packet_tx tx; /**< packet replay structure */
	struct packet_thread thread; /**< thread structure */
	 TAILQ_ENTRY(packet_replay) entry;/**< replay entry */
};

struct packet_thread *dabbad_replay_thread_data_get(const pthread_t thread_id);

void dabbad_replay_stop(Dabba__DabbaService_Service * service,
			const Dabba__ThreadId * idp,
			Dabba__ErrorCode_Closure closure, void *closure_data);

void dabbad_replay_start(Dabba__DabbaService_Service * service,
			 const Dabba__Replay * replayp,
			 Dabba__ErrorCode_Closure closure, void *closure_data);

void dabbad_replay_get(Dabba__DabbaService_Service * service,
		       const Dabba__ThreadIdList * id_listp,
		       Dabba__ReplayList_Closure closure, void *closure_data);

void dabbad_replay_stop_all(Dabba__DabbaService_Service * service,
			    const Dabba__Dummy * dummyp,
			    Dabba__ErrorCode_Closure closure,
			    void *closure_data);

#endif				/* REPLAY_H */
