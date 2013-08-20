/**
 * \file replay.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */


/* HACK prevent libnl3 include clash between <net/if.h> and <linux/if.h> */
#ifndef _LINUX_IF_H
#define _LINUX_IF_H
#endif				/* _LINUX_IF_H */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/queue.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>

#include <libdabba/macros.h>
#include <libdabba/interface.h>
#include <libdabba/packet-tx.h>
#include <libdabba/pcap.h>
#include <dabbad/interface.h>
#include <dabbad/replay.h>
#include <dabbad/misc.h>

/**
 * \internal
 * \brief Replay thread management list
 */

static struct replay_queue {
	TAILQ_HEAD(head, packet_replay) head;
	size_t length;
} replay_queue = {
.head = TAILQ_HEAD_INITIALIZER(replay_queue.head),.length = 0};

/**
 * \internal
 * \brief Get the amount of replay thread in the replay list
 * \return Thread list length
 */

static size_t dabbad_replay_length_get(void)
{
	return replay_queue.length;
}

/**
 * \internal
 * \brief Insert a new replay to the replay list tail
 */

static void dabbad_replay_insert(struct packet_replay *const node)
{
	assert(node);
	TAILQ_INSERT_TAIL(&replay_queue.head, node, entry);
	replay_queue.length++;
}

/**
 * \internal
 * \brief Remove existing replay entry from the replay list
 */

static void dabbad_replay_remove(struct packet_replay *const node)
{
	assert(node);
	assert(replay_queue.length > 0);
	TAILQ_REMOVE(&replay_queue.head, node, entry);
	replay_queue.length--;
}

/**
 * \internal
 * \brief Returns replay matching thread id present in the replay list
 * \return Pointer to the replay matching the replay id
 */

static struct packet_replay *dabbad_replay_find(const pthread_t id)
{
	struct packet_replay *node;

	TAILQ_FOREACH(node, &replay_queue.head, entry)
	    if (node->thread.id == id)
		break;

	return node;
}

/**
 * \internal
 * \brief Replay thread message validator
 * \param[in] msg Replay thread message to check
 * \return 0 if the message is invalid, 1 if it is valid.
 * 
 * A valid replay message must fulfill these requirements:
 *      - Interface name length longer than zero, shorter than \c IFNAMESIZ
 *      - PCAP file name length longer than zero
 *      - Frame size must be a supported size
 *      - Frame number must be greater than zero
 */

static int replay_settings_are_valid(const Dabba__Replay * replayp)
{

	assert(replayp);

	/* Names are empty */
	if (!replayp->interface || strlen(replayp->interface) == 0)
		return 0;

	if (!replayp->pcap || strlen(replayp->pcap) == 0)
		return 0;

	if (!packet_mmap_frame_size_is_valid(replayp->frame_size))
		return 0;

	if (!replayp->frame_nr)
		return 0;

	return 1;
}

/**
 * \brief RPC to stop a running replay
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           idp             Pointer to the thread id to stop
 * \param[in]           closure         Pointer to protobuf closure function pointer
 * \param[in]           closure_data	Pointer to protobuf closure data
 * \return Returns 0 on success, else on failure via its closure function.
 */

void dabbad_replay_stop(Dabba__DabbaService_Service * service,
			const Dabba__ThreadId * idp,
			Dabba__ErrorCode_Closure closure, void *closure_data)
{
	Dabba__ErrorCode err = DABBA__ERROR_CODE__INIT;
	struct packet_replay *pkt_replay;
	int rc;

	assert(service);
	assert(idp);

	pkt_replay = dabbad_replay_find((pthread_t) idp->id);

	if (!pkt_replay) {
		rc = EINVAL;
		goto out;
	}

	rc = dabbad_thread_stop(&pkt_replay->thread);

	if (!rc) {
		dabbad_replay_remove(pkt_replay);
		close(pkt_replay->tx.pcap_fd);
		ldab_packet_mmap_destroy(&pkt_replay->tx.pkt_mmap);
		free(pkt_replay);
	}

 out:
	err.code = rc;
	closure(&err, closure_data);
}

/**
 * \brief RPC to stop all running replays
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           dummyp          Pointer to unused dummy rpc message
 * \param[in]           closure         Pointer to protobuf closure function pointer
 * \param[in]           closure_data	Pointer to protobuf closure data
 * \return Returns 0 on success, else on failure via its closure function.
 */

void dabbad_replay_stop_all(Dabba__DabbaService_Service * service,
			    const Dabba__Dummy * dummyp,
			    Dabba__ErrorCode_Closure closure,
			    void *closure_data)
{
	Dabba__ErrorCode err = DABBA__ERROR_CODE__INIT;
	struct packet_replay *pkt_replay, *tmp;
	int rc = 0;

	assert(service);
	assert(dummyp);

	for (pkt_replay = TAILQ_FIRST(&replay_queue.head); pkt_replay;
	     pkt_replay = tmp) {
		tmp = TAILQ_NEXT(pkt_replay, entry);

		rc = dabbad_thread_stop(&pkt_replay->thread);

		if (rc)
			break;

		dabbad_replay_remove(pkt_replay);
		close(pkt_replay->tx.pcap_fd);
		ldab_packet_mmap_destroy(&pkt_replay->tx.pkt_mmap);
		free(pkt_replay);
	}

	err.code = rc;
	closure(&err, closure_data);
}

/**
 * \brief RPC to start a new replay
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           replayp        Pointer to new replay thread settings
 * \param[in]           closure         Pointer to protobuf closure function pointer
 * \param[in,out]       closure_data	Pointer to protobuf closure data
 * \return Returns 0 on success, else on failure via its closure function.
 */

void dabbad_replay_start(Dabba__DabbaService_Service * service,
			 const Dabba__Replay * replayp,
			 Dabba__ErrorCode_Closure closure, void *closure_data)
{
	struct packet_replay *pkt_replay;
	int sock, rc;

	assert(service);
	assert(replayp);

	if (!replay_settings_are_valid(replayp)) {
		rc = EINVAL;
		goto out;
	}

	sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	if (sock < 0) {
		rc = errno;
		goto out;
	}

	pkt_replay = calloc(1, sizeof(*pkt_replay));

	if (!pkt_replay) {
		free(pkt_replay);
		close(sock);
		rc = ENOMEM;
		goto out;
	}

	pkt_replay->thread.type = REPLAY_THREAD;
	pkt_replay->tx.pcap_fd = ldab_pcap_open(replayp->pcap, O_RDONLY);

	if (pkt_replay->tx.pcap_fd < 0) {
		rc = errno;
		free(pkt_replay);
		close(sock);
		goto out;
	}

	rc = ldab_packet_mmap_create(&pkt_replay->tx.pkt_mmap,
				    replayp->interface, sock, PACKET_MMAP_TX,
				    replayp->frame_size, replayp->frame_nr);

	if (rc) {
		free(pkt_replay);
		close(sock);
		goto out;
	}

	rc = dabbad_thread_start(&pkt_replay->thread, ldab_packet_tx,
				 pkt_replay);

	if (rc) {
		ldab_packet_mmap_destroy(&pkt_replay->tx.pkt_mmap);
		free(pkt_replay);
		close(sock);
	} else
		dabbad_replay_insert(pkt_replay);

 out:
	replayp->status->code = rc;
	closure(replayp->status, closure_data);
}

/**
 * \brief RPC to list requested running replays
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           id_listp        Pointer to the thread id list to get
 * \param[in]           closure         Pointer to protobuf closure function pointer
 * \param[in,out]       closure_data	Pointer to protobuf closure data
 * \return Returns 0 on success, else on failure via its closure function.
 */

void dabbad_replay_get(Dabba__DabbaService_Service * service,
		       const Dabba__ThreadIdList * id_listp,
		       Dabba__ReplayList_Closure closure, void *closure_data)
{
	Dabba__ReplayList replay_list = DABBA__REPLAY_LIST__INIT;
	Dabba__ReplayList *replayp = NULL;
	struct packet_replay *pkt_replay;
	size_t a = dabbad_replay_length_get();

	assert(service);
	assert(id_listp);

	if (a == 0)
		goto out;

	replay_list.list = calloc(a, sizeof(*replay_list.list));

	if (!replay_list.list)
		goto out;

	replay_list.n_list = a;

	for (a = 0; a < replay_list.n_list; a++) {
		replay_list.list[a] = malloc(sizeof(*replay_list.list[a]));

		if (!replay_list.list[a])
			goto out;

		dabba__replay__init(replay_list.list[a]);

		replay_list.list[a]->id =
		    malloc(sizeof(*replay_list.list[a]->id));
		replay_list.list[a]->status =
		    malloc(sizeof(*replay_list.list[a]->status));

		replay_list.list[a]->pcap =
		    calloc(NAME_MAX, sizeof(*replay_list.list[a]->pcap));
		replay_list.list[a]->interface =
		    calloc(IFNAMSIZ, sizeof(*replay_list.list[a]->interface));

		if (!replay_list.list[a]->id || !replay_list.list[a]->status
		    || !replay_list.list[a]->pcap
		    || !replay_list.list[a]->interface)
			goto out;

		dabba__thread_id__init(replay_list.list[a]->id);
		dabba__error_code__init(replay_list.list[a]->status);
	}

	a = 0;

	TAILQ_FOREACH(pkt_replay, &replay_queue.head, entry) {
		replay_list.list[a]->has_frame_nr =
		    replay_list.list[a]->has_frame_size = 1;
		replay_list.list[a]->frame_nr =
		    pkt_replay->tx.pkt_mmap.layout.tp_frame_nr;
		replay_list.list[a]->frame_size =
		    pkt_replay->tx.pkt_mmap.layout.tp_frame_size;
		replay_list.list[a]->id->id = (uint64_t) pkt_replay->thread.id;

		/* TODO report replay health: disk full, link down etc... */
		replay_list.list[a]->status->code = 0;

		fd_to_path(pkt_replay->tx.pcap_fd, replay_list.list[a]->pcap,
			   NAME_MAX * sizeof(*replay_list.list[a]->pcap));

		ldab_ifindex_to_devname(pkt_replay->tx.pkt_mmap.ifindex,
				       replay_list.list[a]->interface,
				       IFNAMSIZ);

		a++;
	}

	replayp = &replay_list;

 out:
	closure(replayp, closure_data);

	for (a = 0; a < replay_list.n_list; a++) {
		if (replay_list.list[a]) {
			free(replay_list.list[a]->id);
			free(replay_list.list[a]->status);
			free(replay_list.list[a]->pcap);
			free(replay_list.list[a]->interface);
		}

		free(replay_list.list[a]);
	}

	free(replay_list.list);
}
