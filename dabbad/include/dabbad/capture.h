/**
 * \file capture.h
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */


#ifndef CAPTURE_H
#define	CAPTURE_H

#include <dabbad/thread.h>
#include <libdabba/packet-rx.h>
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
