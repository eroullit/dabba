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
#include <sys/queue.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <netlink/cache.h>
#include <netlink/route/link.h>

#include <libdabba/macros.h>
#include <libdabba/interface.h>
#include <libdabba/packet_rx.h>
#include <libdabba/pcap.h>
#include <dabbad/dabbad.h>
#include <dabbad/interface.h>
#include <dabbad/capture.h>
#include <dabbad/misc.h>

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

static int capture_settings_are_valid(const Dabba__CaptureSettings * capturep)
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
 * \internal
 * \brief Get capture thread data from thread dabba
 * \param[in] pkt_thread pointer to thread data structure
 * \return Pointer to the corresponding capture thread information.
 */

static struct packet_capture_thread *dabbad_capture_thread_get(struct packet_thread
							       *pkt_thread)
{
	return container_of(pkt_thread, struct packet_capture_thread, thread);
}

/**
 * \brief Start a capture thread 
 * \param[in,out] msg Capture thread message
 * \return 0 on success, else on failure.
 * \warning This function requires the CAP_NET_RAW capability
 */

#if 0
int dabbad_capture_start(struct dabba_ipc_msg *msg)
{
	struct packet_capture_thread *pkt_capture;
	struct dabba_capture *capture_msg = msg->msg_body.msg.capture;
	int rc, sock;

	if (!capture_msg_is_valid(msg))
		return EINVAL;

	sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	if (sock < 0)
		return errno;

	pkt_capture = calloc(1, sizeof(*pkt_capture));

	if (!pkt_capture) {
		free(pkt_capture);
		close(sock);
		return ENOMEM;
	}

	pkt_capture->rx.pcap_fd =
	    pcap_create(msg->msg_body.msg.capture->pcap_name, LINKTYPE_EN10MB);

	rc = packet_mmap_create(&pkt_capture->rx.pkt_mmap,
				capture_msg->dev_name, sock, PACKET_MMAP_RX,
				capture_msg->frame_size, capture_msg->frame_nr);

	if (rc) {
		free(pkt_capture);
		close(sock);
		return rc;
	}

	rc = dabbad_thread_start(&pkt_capture->thread, packet_rx, pkt_capture);

	if (rc) {
		packet_mmap_destroy(&pkt_capture->rx.pkt_mmap);
		free(pkt_capture);
		close(sock);
	}

	return rc;
}
#endif

/**
 * \brief List currently running capture thread
 * \param[in,out] msg Capture thread message
 * \return 0 on success, else on failure.
 */

int dabbad_capture_list(struct dabba_ipc_msg *msg)
{
	struct packet_capture_thread *pkt_capture;
	struct dabba_capture *capture_msg;
	struct packet_thread *pkt_thread;
	struct tpacket_req *layout;
	size_t a = 0, off = 0, thread_list_size;

	capture_msg = msg->msg_body.msg.capture;
	thread_list_size = ARRAY_SIZE(msg->msg_body.msg.capture);

	for (pkt_thread = dabbad_thread_type_first(CAPTURE_THREAD); pkt_thread;
	     pkt_thread = dabbad_thread_type_next(pkt_thread, CAPTURE_THREAD)) {
		if (off < msg->msg_body.offset) {
			off++;
			continue;
		}

		if (a >= thread_list_size)
			break;

		pkt_capture = dabbad_capture_thread_get(pkt_thread);

		layout = &pkt_capture->rx.pkt_mmap.layout;

		capture_msg[a].id = pkt_capture->thread.id;
		capture_msg[a].frame_size = layout->tp_frame_size;
		capture_msg[a].frame_nr = layout->tp_frame_nr;

		/* TODO error handling */
		fd_to_path(pkt_capture->rx.pcap_fd, capture_msg[a].pcap_name,
			   sizeof(capture_msg[a].pcap_name));
		ifindex_to_devname(pkt_capture->rx.pkt_mmap.ifindex,
				   capture_msg[a].dev_name,
				   sizeof(capture_msg[a].dev_name));

		a++;
	}

	msg->msg_body.elem_nr = a;

	return 0;
}

#if 0
/**
 * \brief Stop a running capture thread
 * \param[in,out] msg Capture thread message
 * \return 0 on success, else on failure.
 */

int dabbad_capture_stop(struct dabba_ipc_msg *msg)
{
	struct packet_capture_thread *pkt_capture;
	struct packet_thread *pkt_thread;
	struct dabba_capture *capture_msg = msg->msg_body.msg.capture;
	int rc = 0;

	pkt_thread = dabbad_thread_data_get(capture_msg->id);

	if (!pkt_thread)
		return EINVAL;

	rc = dabbad_thread_stop(pkt_thread);

	if (!rc) {
		pkt_capture = dabbad_capture_thread_get(pkt_thread);
		close(pkt_capture->rx.pcap_fd);
		packet_mmap_destroy(&pkt_capture->rx.pkt_mmap);
		free(pkt_capture);
	}

	return rc;
}
#endif

void dabbad_capture_stop(Dabba__DabbaService_Service * service,
			 const Dabba__ThreadId * idp,
			 Dabba__Dummy_Closure closure, void *closure_data)
{
	Dabba__Dummy dummy = DABBA__DUMMY__INIT;
	struct packet_capture_thread *pkt_capture;
	struct packet_thread *pkt_thread;
	int rc;

	assert(service);
	assert(idp);

	pkt_thread = dabbad_thread_data_get((pthread_t) idp->id);

	if (!pkt_thread)
		goto out;

	rc = dabbad_thread_stop(pkt_thread);

	if (!rc) {
		pkt_capture = dabbad_capture_thread_get(pkt_thread);
		close(pkt_capture->rx.pcap_fd);
		packet_mmap_destroy(&pkt_capture->rx.pkt_mmap);
		free(pkt_capture);
	}

 out:
	closure(&dummy, closure_data);
}

void dabbad_capture_start(Dabba__DabbaService_Service * service,
			  const Dabba__CaptureSettings * capturep,
			  Dabba__Dummy_Closure closure, void *closure_data)
{
	Dabba__Dummy dummy = DABBA__DUMMY__INIT;
	struct packet_capture_thread *pkt_capture;
	int sock, rc;

	assert(service);
	assert(capturep);

	if (!capture_settings_are_valid(capturep))
		goto out;

	sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	if (sock < 0)
		goto out;

	pkt_capture = calloc(1, sizeof(*pkt_capture));

	if (!pkt_capture) {
		free(pkt_capture);
		close(sock);
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
	}

 out:
	closure(&dummy, closure_data);
}

void dabbad_capture_settings_get(Dabba__DabbaService_Service * service,
				 const Dabba__ThreadIdList * id_listp,
				 Dabba__CaptureSettingsList_Closure closure,
				 void *closure_data)
{
	Dabba__CaptureSettingsList capture_list =
	    DABBA__CAPTURE_SETTINGS_LIST__INIT;
	Dabba__CaptureSettingsList *capturep = NULL;
	struct packet_capture_thread *pkt_capture;
	struct packet_thread *pkt_thread;
	struct nl_sock *sock;
	struct nl_cache *cache;
	size_t a = 0;

	assert(service);
	assert(id_listp);

	cache = link_cache_alloc(&sock);

	if (!cache)
		goto out;

	for (pkt_thread = dabbad_thread_type_first(CAPTURE_THREAD); pkt_thread;
	     pkt_thread = dabbad_thread_type_next(pkt_thread, CAPTURE_THREAD)) {
		a++;
	}

	capture_list.list = calloc(a, sizeof(*capture_list.list));

	if (!capture_list.list)
		goto out;

	capture_list.n_list = a;

	for (a = 0; a < capture_list.n_list; a++) {
		capture_list.list[a] = malloc(sizeof(*capture_list.list[a]));

		if (!capture_list.list[a])
			goto out;

		dabba__capture_settings__init(capture_list.list[a]);

		capture_list.list[a]->id =
		    malloc(sizeof(*capture_list.list[a]->id));
		capture_list.list[a]->pcap =
		    calloc(NAME_MAX, sizeof(*capture_list.list[a]->pcap));
		capture_list.list[a]->interface =
		    calloc(IFNAMSIZ, sizeof(*capture_list.list[a]->interface));

		if (!capture_list.list[a]->id || !capture_list.list[a]->pcap
		    || !capture_list.list[a]->interface)
			goto out;

		dabba__thread_id__init(capture_list.list[a]->id);
	}

	for (a = 0, pkt_thread = dabbad_thread_type_first(CAPTURE_THREAD);
	     pkt_thread;
	     a++, pkt_thread =
	     dabbad_thread_type_next(pkt_thread, CAPTURE_THREAD)) {
		pkt_capture = dabbad_capture_thread_get(pkt_thread);

		capture_list.list[a]->has_frame_nr =
		    capture_list.list[a]->has_frame_size = 1;
		capture_list.list[a]->frame_nr =
		    pkt_capture->rx.pkt_mmap.layout.tp_frame_nr;
		capture_list.list[a]->frame_size =
		    pkt_capture->rx.pkt_mmap.layout.tp_frame_size;
		capture_list.list[a]->id->id =
		    (uint64_t) pkt_capture->thread.id;

		fd_to_path(pkt_capture->rx.pcap_fd, capture_list.list[a]->pcap,
			   NAME_MAX * sizeof(*capture_list.list[a]->pcap));
		rtnl_link_i2name(cache, pkt_capture->rx.pkt_mmap.ifindex,
				 capture_list.list[a]->interface,
				 IFNAMSIZ *
				 sizeof(*capture_list.list[a]->interface));
	}

	capturep = &capture_list;

 out:
	closure(capturep, closure_data);

	for (a = 0; a < capture_list.n_list; a++) {
		if (capture_list.list[a]) {
			free(capture_list.list[a]->id);
			free(capture_list.list[a]->pcap);
			free(capture_list.list[a]->interface);
		}

		free(capture_list.list[a]);
	}

	free(capture_list.list);
	link_cache_destroy(sock, cache);
}
