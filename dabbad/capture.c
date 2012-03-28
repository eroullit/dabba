/* __LICENSE_HEADER_BEGIN__ */

/*
 * Copyright (C) 2009-2012	Emmanuel Roullit <emmanuel.roullit@gmail.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/queue.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>

#include <dabbacore/macros.h>
#include <dabbacore/nic.h>
#include <dabbacore/packet_rx.h>
#include <dabbacore/pcap.h>
#include <dabbad/dabbad.h>
#include <dabbad/misc.h>

struct capture_thread_node {
	struct packet_rx_thread *pkt_capture;
	 SLIST_ENTRY(capture_thread_node) entry;
};

static SLIST_HEAD(capture_thread_head, capture_thread_node) thread_head;

static int capture_msg_is_valid(struct dabba_ipc_msg *msg)
{
	struct dabba_capture *capture_msg = msg->msg_body.msg.capture;

	if (!msg)
		return 0;

	if (strlen(capture_msg->dev_name) >= sizeof(capture_msg->dev_name))
		return 0;

	if (strlen(capture_msg->pcap_name) >= sizeof(capture_msg->pcap_name))
		return 0;

	/* refuse pcap name with '/' to not change directory */
	if (strchr(capture_msg->pcap_name, '/'))
		return 0;

	if (!packet_mmap_frame_size_is_valid(capture_msg->frame_size))
		return 0;

	if (!capture_msg->page_order)
		return 0;

	return 1;
}

int dabbad_capture_start(struct dabba_ipc_msg *msg)
{
	struct packet_rx_thread *pkt_capture;
	struct capture_thread_node *thread_node;
	struct dabba_capture *capture_msg = msg->msg_body.msg.capture;
	int rc, sock;

	if (!capture_msg_is_valid(msg))
		return EINVAL;

	sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	if (sock < 0)
		return errno;

	pkt_capture = calloc(1, sizeof(*pkt_capture));
	thread_node = calloc(1, sizeof(*thread_node));

	if (!pkt_capture || !thread_node) {
		free(pkt_capture);
		free(thread_node);
		close(sock);
		return ENOMEM;
	}

	pkt_capture->pcap_fd =
	    pcap_create(msg->msg_body.msg.capture->pcap_name, LINKTYPE_EN10MB);

	rc = packet_mmap_create(&pkt_capture->pkt_rx, capture_msg->dev_name,
				sock, PACKET_MMAP_RX, capture_msg->frame_size,
				capture_msg->page_order, capture_msg->size);

	if (rc) {
		free(pkt_capture);
		close(sock);
		return rc;
	}

	/* TODO: Add pthread attribute support */
	rc = pthread_create(&pkt_capture->thread, NULL, packet_rx, pkt_capture);

	if (!rc) {
		thread_node->pkt_capture = pkt_capture;
		SLIST_INSERT_HEAD(&thread_head, thread_node, entry);
	} else {
		packet_mmap_destroy(&pkt_capture->pkt_rx);
		free(pkt_capture);
		free(thread_node);
		close(sock);
	}

	return rc;
}

int dabbad_capture_list(struct dabba_ipc_msg *msg)
{
	struct capture_thread_node *node;
	struct dabba_capture *capture;
	size_t a = 0, off = 0, thread_list_size;

	capture = msg->msg_body.msg.capture;
	thread_list_size = ARRAY_SIZE(msg->msg_body.msg.capture);

	SLIST_FOREACH(node, &thread_head, entry) {
		if (off < msg->msg_body.offset) {
			off++;
			continue;
		}

		if (a >= thread_list_size)
			break;

		capture[a].thread_id = node->pkt_capture->thread;
		capture[a].size =
		    node->pkt_capture->pkt_rx.layout.tp_frame_size *
		    node->pkt_capture->pkt_rx.layout.tp_frame_nr;

		/* TODO error handling */
		fd_to_path(node->pkt_capture->pcap_fd, capture[a].pcap_name,
			   sizeof(capture[a].pcap_name));
		ifindex_to_devname(node->pkt_capture->pkt_rx.ifindex,
				   capture[a].dev_name,
				   sizeof(capture[a].dev_name));

		a++;
	}

	msg->msg_body.elem_nr = a;

	return 0;
}

int dabbad_capture_stop(struct dabba_ipc_msg *msg)
{
	struct capture_thread_node *node;
	struct dabba_capture *capture_msg = msg->msg_body.msg.capture;
	int rc = 0;

	SLIST_FOREACH(node, &thread_head, entry) {
		if (capture_msg->thread_id == node->pkt_capture->thread)
			break;
	}

	if (node) {
		SLIST_REMOVE(&thread_head, node, capture_thread_node, entry);
		rc = pthread_cancel(node->pkt_capture->thread);
		close(node->pkt_capture->pcap_fd);
		packet_mmap_destroy(&node->pkt_capture->pkt_rx);
		free(node->pkt_capture);
		free(node);
	}

	/* TODO pthread_cancel() error is should issue a warning
	 * even if the user cannot do anything about it...
	 */

	return rc;
}
