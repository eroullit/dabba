/**
 * \file capture.c
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
#include <libdabba/packet-rx.h>
#include <libdabba/pcap.h>
#include <libdabba/sock-filter.h>
#include <dabbad/interface.h>
#include <dabbad/sock-filter.h>
#include <dabbad/capture.h>
#include <dabbad/misc.h>

/**
 * \internal
 * \brief Capture thread management list
 */

static struct capture_queue {
	TAILQ_HEAD(head, packet_capture) head;
	size_t length;
} capture_queue = {
.head = TAILQ_HEAD_INITIALIZER(capture_queue.head),.length = 0};

/**
 * \internal
 * \brief Get the amount of capture in the capture list
 * \return Thread list length
 */

static size_t dabbad_capture_length_get(void)
{
	return capture_queue.length;
}

/**
 * \internal
 * \brief Insert a new capture to the capture list tail
 */

static void dabbad_capture_insert(struct packet_capture *const node)
{
	assert(node);
	TAILQ_INSERT_TAIL(&capture_queue.head, node, entry);
	capture_queue.length++;
}

/**
 * \internal
 * \brief Remove existing capture entry from the capture list
 */

static void dabbad_capture_remove(struct packet_capture *const node)
{
	assert(node);
	assert(capture_queue.length > 0);
	TAILQ_REMOVE(&capture_queue.head, node, entry);
	capture_queue.length--;
}

/**
 * \internal
 * \brief Returns capture matching thread id present in the capture list
 * \return Pointer to the capture matching the capture id
 */

static struct packet_capture *dabbad_capture_find(const pthread_t id)
{
	struct packet_capture *node;

	TAILQ_FOREACH(node, &capture_queue.head, entry)
	    if (node->thread.id == id)
		break;

	return node;
}

/**
 * \internal
 * \brief Capture thread message validator
 * \param[in] msg Capture thread message to check
 * \return 0 if the message is invalid, 1 if it is valid.
 * 
 * A valid capture message must fulfill these requirements:
 *      - Interface name length longer than zero, shorter than \c IFNAMESIZ
 *      - PCAP file name length longer than zero
 *      - Frame size must be a supported size
 *      - The memory page order must be greater than zero
 */

static int capture_settings_are_valid(const Dabba__Capture * capturep)
{

	assert(capturep);

	/* Names are empty */
	if (!capturep->interface || strlen(capturep->interface) == 0)
		return 0;

	if (!capturep->pcap || strlen(capturep->pcap) == 0)
		return 0;

	if (!packet_mmap_frame_size_is_valid(capturep->frame_size))
		return 0;

	if (!capturep->frame_nr)
		return 0;

	return 1;
}

/**
 * \brief RPC to stop a running capture
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           idp             Pointer to the thread id to stop
 * \param[in]           closure         Pointer to protobuf closure function pointer
 * \param[in]           closure_data	Pointer to protobuf closure data
 * \return Returns 0 on success, else on failure via its closure function.
 */

void dabbad_capture_stop(Dabba__DabbaService_Service * service,
			 const Dabba__ThreadId * idp,
			 Dabba__ErrorCode_Closure closure, void *closure_data)
{
	Dabba__ErrorCode err = DABBA__ERROR_CODE__INIT;
	struct packet_capture *pkt_capture;
	int rc;

	assert(service);
	assert(idp);

	pkt_capture = dabbad_capture_find((pthread_t) idp->id);

	if (!pkt_capture) {
		rc = EINVAL;
		goto out;
	}

	rc = dabbad_thread_stop(&pkt_capture->thread);

	if (!rc) {
		dabbad_capture_remove(pkt_capture);
		ldab_sock_filter_detach(pkt_capture->rx.pkt_mmap.pf_sock);
		dabbad_sfp_destroy(&pkt_capture->rx.sfp);
		close(pkt_capture->rx.pcap_fd);
		ldab_packet_mmap_destroy(&pkt_capture->rx.pkt_mmap);
		free(pkt_capture);
	}

 out:
	err.code = rc;
	closure(&err, closure_data);
}

/**
 * \brief RPC to stop all running captures
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           dummyp          Pointer to unused dummy rpc message
 * \param[in]           closure         Pointer to protobuf closure function pointer
 * \param[in]           closure_data	Pointer to protobuf closure data
 * \return Returns 0 on success, else on failure via its closure function.
 */

void dabbad_capture_stop_all(Dabba__DabbaService_Service * service,
			     const Dabba__Dummy * dummyp,
			     Dabba__ErrorCode_Closure closure,
			     void *closure_data)
{
	Dabba__ErrorCode err = DABBA__ERROR_CODE__INIT;
	struct packet_capture *pkt_capture, *tmp;
	int rc = 0;

	assert(service);
	assert(dummyp);

	for (pkt_capture = TAILQ_FIRST(&capture_queue.head); pkt_capture;
	     pkt_capture = tmp) {
		tmp = TAILQ_NEXT(pkt_capture, entry);

		rc = dabbad_thread_stop(&pkt_capture->thread);

		if (rc)
			break;

		dabbad_capture_remove(pkt_capture);
		ldab_sock_filter_detach(pkt_capture->rx.pkt_mmap.pf_sock);
		dabbad_sfp_destroy(&pkt_capture->rx.sfp);
		close(pkt_capture->rx.pcap_fd);
		ldab_packet_mmap_destroy(&pkt_capture->rx.pkt_mmap);
		free(pkt_capture);
	}

	err.code = rc;
	closure(&err, closure_data);
}

/**
 * \brief RPC to start a new capture
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           capturep        Pointer to new capture thread settings
 * \param[in]           closure         Pointer to protobuf closure function pointer
 * \param[in,out]       closure_data	Pointer to protobuf closure data
 * \return Returns 0 on success, else on failure via its closure function.
 */

void dabbad_capture_start(Dabba__DabbaService_Service * service,
			  const Dabba__Capture * capturep,
			  Dabba__ErrorCode_Closure closure, void *closure_data)
{
	struct packet_capture *pkt_capture;
	int sock, rc;

	assert(service);
	assert(capturep);

	if (!capture_settings_are_valid(capturep)) {
		rc = EINVAL;
		goto out;
	}

	sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	if (sock < 0) {
		rc = errno;
		goto out;
	}

	pkt_capture = calloc(1, sizeof(*pkt_capture));

	if (!pkt_capture) {
		free(pkt_capture);
		close(sock);
		rc = ENOMEM;
		goto out;
	}

	pkt_capture->thread.type = CAPTURE_THREAD;

	if (capturep->append)
		pkt_capture->rx.pcap_fd =
		    ldab_pcap_open(capturep->pcap, O_RDWR | O_APPEND);
	else
		pkt_capture->rx.pcap_fd =
		    ldab_pcap_create(capturep->pcap, LINKTYPE_EN10MB);

	if (pkt_capture->rx.pcap_fd < 0) {
		rc = errno;
		free(pkt_capture);
		close(sock);
		goto out;
	}

	if (capturep->sfp && capturep->sfp->n_filter) {
		rc = dabbad_pbuf_sfp_2_sfp(capturep->sfp, &pkt_capture->rx.sfp);

		if (rc) {
			free(pkt_capture);
			close(sock);
			goto out;
		}

		rc = ldab_sock_filter_attach(sock, &pkt_capture->rx.sfp);

		if (rc) {
			dabbad_sfp_destroy(&pkt_capture->rx.sfp);
			free(pkt_capture);
			close(sock);
			goto out;
		}
	}

	rc = ldab_packet_mmap_create(&pkt_capture->rx.pkt_mmap,
				    capturep->interface, sock, PACKET_MMAP_RX,
				    capturep->frame_size, capturep->frame_nr);

	if (rc) {
		dabbad_sfp_destroy(&pkt_capture->rx.sfp);
		free(pkt_capture);
		close(sock);
		goto out;
	}

	rc = dabbad_thread_start(&pkt_capture->thread, ldab_packet_rx,
				 pkt_capture);

	if (rc) {
		ldab_packet_mmap_destroy(&pkt_capture->rx.pkt_mmap);
		dabbad_sfp_destroy(&pkt_capture->rx.sfp);
		free(pkt_capture);
		close(sock);
	} else
		dabbad_capture_insert(pkt_capture);

 out:
	capturep->status->code = rc;
	closure(capturep->status, closure_data);
}

/**
 * \brief RPC to list requested running captures
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           id_listp        Pointer to the thread id list to get
 * \param[in]           closure         Pointer to protobuf closure function pointer
 * \param[in,out]       closure_data	Pointer to protobuf closure data
 * \return Returns 0 on success, else on failure via its closure function.
 */

void dabbad_capture_get(Dabba__DabbaService_Service * service,
			const Dabba__ThreadIdList * id_listp,
			Dabba__CaptureList_Closure closure, void *closure_data)
{
	Dabba__CaptureList capture_list = DABBA__CAPTURE_LIST__INIT;
	Dabba__CaptureList *capturep = NULL;
	struct packet_capture *pkt_capture;
	size_t a = dabbad_capture_length_get();

	assert(service);
	assert(id_listp);

	if (a == 0)
		goto out;

	capture_list.list = calloc(a, sizeof(*capture_list.list));

	if (!capture_list.list)
		goto out;

	capture_list.n_list = a;

	for (a = 0; a < capture_list.n_list; a++) {
		capture_list.list[a] = malloc(sizeof(*capture_list.list[a]));

		if (!capture_list.list[a])
			goto out;

		dabba__capture__init(capture_list.list[a]);

		capture_list.list[a]->id =
		    malloc(sizeof(*capture_list.list[a]->id));
		capture_list.list[a]->status =
		    malloc(sizeof(*capture_list.list[a]->status));
		capture_list.list[a]->sfp =
		    malloc(sizeof(*capture_list.list[a]->sfp));

		capture_list.list[a]->pcap =
		    calloc(NAME_MAX, sizeof(*capture_list.list[a]->pcap));
		capture_list.list[a]->interface =
		    calloc(IFNAMSIZ, sizeof(*capture_list.list[a]->interface));

		if (!capture_list.list[a]->id || !capture_list.list[a]->status
		    || !capture_list.list[a]->sfp || !capture_list.list[a]->pcap
		    || !capture_list.list[a]->interface)
			goto out;

		dabba__thread_id__init(capture_list.list[a]->id);
		dabba__error_code__init(capture_list.list[a]->status);
		dabba__sock_fprog__init(capture_list.list[a]->sfp);
	}

	a = 0;

	TAILQ_FOREACH(pkt_capture, &capture_queue.head, entry) {
		capture_list.list[a]->has_frame_nr =
		    capture_list.list[a]->has_frame_size = 1;
		capture_list.list[a]->frame_nr =
		    pkt_capture->rx.pkt_mmap.layout.tp_frame_nr;
		capture_list.list[a]->frame_size =
		    pkt_capture->rx.pkt_mmap.layout.tp_frame_size;
		capture_list.list[a]->id->id =
		    (uint64_t) pkt_capture->thread.id;

		/* TODO report capture health: disk full, link down etc... */
		capture_list.list[a]->status->code = 0;

		fd_to_path(pkt_capture->rx.pcap_fd, capture_list.list[a]->pcap,
			   NAME_MAX * sizeof(*capture_list.list[a]->pcap));

		ldab_ifindex_to_devname(pkt_capture->rx.pkt_mmap.ifindex,
				       capture_list.list[a]->interface,
				       IFNAMSIZ);

		dabbad_sfp_2_pbuf_sfp(&pkt_capture->rx.sfp,
				      capture_list.list[a]->sfp);

		a++;
	}

	capturep = &capture_list;

 out:
	closure(capturep, closure_data);

	for (a = 0; a < capture_list.n_list; a++) {
		if (capture_list.list[a]) {
			dabbad_pbuf_sfp_destroy(capture_list.list[a]->sfp);
			free(capture_list.list[a]->sfp);
			free(capture_list.list[a]->id);
			free(capture_list.list[a]->status);
			free(capture_list.list[a]->pcap);
			free(capture_list.list[a]->interface);
		}

		free(capture_list.list[a]);
	}

	free(capture_list.list);
}
