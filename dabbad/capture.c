/**
 * \file capture.c
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
#include <sys/queue.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <netlink/cache.h>
#include <netlink/route/link.h>

#include <libdabba/macros.h>
#include <libdabba/interface.h>
#include <libdabba/packet_rx.h>
#include <libdabba/pcap.h>
#include <dabbad/interface.h>
#include <dabbad/capture.h>
#include <dabbad/misc.h>

/**
 * \internal
 * \brief Capture thread management list
 */

static TAILQ_HEAD(capture_thread_head,
		  packet_capture) capture_thread_head =
TAILQ_HEAD_INITIALIZER(capture_thread_head);

/**
 * \internal
 * \brief Returns the first capture of the capture list
 * \return Pointer to the first capture of the capture list
 */

static struct packet_capture *dabbad_capture_first(void)
{
	return TAILQ_FIRST(&capture_thread_head);
}

/**
 * \internal
 * \brief Returns next capture present in the capture list
 * \return Pointer to the next capture of the capture list
 */

static struct packet_capture *dabbad_capture_next(struct packet_capture
						  *capture_thread)
{
	return capture_thread ? TAILQ_NEXT(capture_thread, entry) : NULL;
}

/**
 * \internal
 * \brief Returns capture matching thread id present in the capture list
 * \return Pointer to the capture matching the capture id
 */

static struct packet_capture *dabbad_capture_find(const pthread_t id)
{
	struct packet_capture *node;

	TAILQ_FOREACH(node, &capture_thread_head, entry)
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
 *      - Interface name length longer than zero, shorter than IFNAMESIZ
 *      - PCAP file name length longer than zero, shorter than IFNAMESIZ
 *      - PCAP file must not contain '/' character
 *      - Frame size must be in supported
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
		TAILQ_REMOVE(&capture_thread_head, pkt_capture, entry);
		close(pkt_capture->rx.pcap_fd);
		packet_mmap_destroy(&pkt_capture->rx.pkt_mmap);
		free(pkt_capture);
	}

 out:
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

	pkt_capture->rx.pcap_fd = pcap_create(capturep->pcap, LINKTYPE_EN10MB);

	rc = packet_mmap_create(&pkt_capture->rx.pkt_mmap, capturep->interface,
				sock, PACKET_MMAP_RX, capturep->frame_size,
				capturep->frame_nr);

	if (rc) {
		free(pkt_capture);
		close(sock);
		goto out;
	}

	rc = dabbad_thread_start(&pkt_capture->thread, packet_rx, pkt_capture);

	if (rc) {
		packet_mmap_destroy(&pkt_capture->rx.pkt_mmap);
		free(pkt_capture);
		close(sock);
	} else
		TAILQ_INSERT_TAIL(&capture_thread_head, pkt_capture, entry);

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
	struct nl_sock *sock;
	struct nl_cache *cache;
	size_t a = 0;

	assert(service);
	assert(id_listp);

	cache = link_cache_alloc(&sock);

	if (!cache)
		goto out;

	for (pkt_capture = dabbad_capture_first(); pkt_capture;
	     pkt_capture = dabbad_capture_next(pkt_capture))
		a++;

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

		capture_list.list[a]->pcap =
		    calloc(NAME_MAX, sizeof(*capture_list.list[a]->pcap));
		capture_list.list[a]->interface =
		    calloc(IFNAMSIZ, sizeof(*capture_list.list[a]->interface));

		if (!capture_list.list[a]->id || !capture_list.list[a]->status
		    || !capture_list.list[a]->pcap
		    || !capture_list.list[a]->interface)
			goto out;

		dabba__thread_id__init(capture_list.list[a]->id);
		dabba__error_code__init(capture_list.list[a]->status);
	}

	for (a = 0, pkt_capture = dabbad_capture_first(); pkt_capture;
	     a++, pkt_capture = dabbad_capture_next(pkt_capture)) {
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

		ifindex_to_devname(pkt_capture->rx.pkt_mmap.ifindex,
				   capture_list.list[a]->interface, IFNAMSIZ);
	}

	capturep = &capture_list;

 out:
	closure(capturep, closure_data);

	for (a = 0; a < capture_list.n_list; a++) {
		if (capture_list.list[a]) {
			free(capture_list.list[a]->id);
			free(capture_list.list[a]->status);
			free(capture_list.list[a]->pcap);
			free(capture_list.list[a]->interface);
		}

		free(capture_list.list[a]);
	}

	free(capture_list.list);
	link_cache_destroy(sock, cache);
}
