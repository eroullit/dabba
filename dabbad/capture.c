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
#include <errno.h>
#include <assert.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <dabbacore/packet_rx.h>
#include <dabbacore/pcap.h>
#include <dabbad/dabbad.h>

static int capture_msg_is_valid(struct dabba_ipc_msg *msg)
{
	struct dabba_capture *capture_msg = &msg->msg_body.msg.capture;

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
	struct dabba_capture *capture_msg = &msg->msg_body.msg.capture;
	int rc;
	int sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

	if (!capture_msg_is_valid(msg)) {
		return EINVAL;
	}

	pkt_capture = calloc(1, sizeof(*pkt_capture));

	if (!pkt_capture) {
		return ENOMEM;
	}

	pkt_capture->pcap_fd =
	    pcap_create(msg->msg_body.msg.capture.pcap_name, LINKTYPE_EN10MB);

	rc = packet_mmap_create(&pkt_capture->pkt_rx, capture_msg->dev_name,
				sock, PACKET_MMAP_RX, capture_msg->frame_size,
				capture_msg->page_order, capture_msg->size);

	if (rc) {
		free(pkt_capture);
		goto out;
	}

	/* TODO: Add pthread attribute support */
	rc = pthread_create(&pkt_capture->thread, NULL, packet_rx, pkt_capture);

	if (rc) {
		packet_mmap_destroy(&pkt_capture->pkt_rx);
		free(pkt_capture);
	}

 out:
	return rc;
}
