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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/queue.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>

#include <libdabba/macros.h>
#include <libdabba/nic.h>
#include <libdabba/packet_rx.h>
#include <libdabba/pcap.h>
#include <dabbad/dabbad.h>
#include <dabbad/thread.h>
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

static int capture_msg_is_valid(struct dabba_ipc_msg *msg)
{
	struct dabba_capture *capture_msg = msg->msg_body.msg.capture;

	if (!msg)
		return 0;

	/* Names are empty */
	if (strlen(capture_msg->dev_name) == 0)
		return 0;

	if (strlen(capture_msg->pcap_name) == 0)
		return 0;

	/* Names are too long / not terminated properly */
	if (strlen(capture_msg->dev_name) >= sizeof(capture_msg->dev_name))
		return 0;

	if (strlen(capture_msg->pcap_name) >= sizeof(capture_msg->pcap_name))
		return 0;

	if (!packet_mmap_frame_size_is_valid(capture_msg->frame_size))
		return 0;

	if (!capture_msg->frame_nr)
		return 0;

	return 1;
}

static struct packet_rx_thread *dabbad_capture_thread_get(struct packet_thread
							  *pkt_thread)
{
	return container_of(pkt_thread, struct packet_rx_thread, thread);
}

/**
 * \brief Start a capture thread 
 * \param[in,out] msg Capture thread message
 * \return 0 on success, else on failure.
 * \warning This function requires the CAP_NET_RAW capability
 */

int dabbad_capture_start(struct dabba_ipc_msg *msg)
{
	struct packet_rx_thread *pkt_capture;
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

	pkt_capture->pcap_fd =
	    pcap_create(msg->msg_body.msg.capture->pcap_name, LINKTYPE_EN10MB);

	rc = packet_mmap_create(&pkt_capture->pkt_rx, capture_msg->dev_name,
				sock, PACKET_MMAP_RX, capture_msg->frame_size,
				capture_msg->frame_nr);

	if (rc) {
		free(pkt_capture);
		close(sock);
		return rc;
	}

	rc = dabbad_thread_start(&pkt_capture->thread, packet_rx, pkt_capture);

	thread_sched_policy_set(&pkt_capture->thread,
				capture_msg->thread.sched_policy);
	thread_sched_prio_set(&pkt_capture->thread,
			      capture_msg->thread.sched_prio);

	if (rc) {
		packet_mmap_destroy(&pkt_capture->pkt_rx);
		free(pkt_capture);
		close(sock);
	}

	return rc;
}

/**
 * \brief List currently running capture thread
 * \param[in,out] msg Capture thread message
 * \return 0 on success, else on failure.
 */

int dabbad_capture_list(struct dabba_ipc_msg *msg)
{
	struct packet_rx_thread *pkt_capture;
	struct dabba_capture *capture;
	struct packet_thread *pkt_thread;
	struct tpacket_req *layout;
	size_t a = 0, off = 0, thread_list_size;

	capture = msg->msg_body.msg.capture;
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

		layout = &pkt_capture->pkt_rx.layout;

		capture[a].thread.id = pkt_capture->thread.id;
		thread_sched_policy_get(&pkt_capture->thread,
					&capture[a].thread.sched_policy);
		thread_sched_prio_get(&pkt_capture->thread,
				      &capture[a].thread.sched_prio);
		capture[a].thread.id = pkt_capture->thread.id;
		capture[a].frame_size = layout->tp_frame_size;
		capture[a].frame_nr = layout->tp_frame_nr;

		/* TODO error handling */
		fd_to_path(pkt_capture->pcap_fd, capture[a].pcap_name,
			   sizeof(capture[a].pcap_name));
		ifindex_to_devname(pkt_capture->pkt_rx.ifindex,
				   capture[a].dev_name,
				   sizeof(capture[a].dev_name));

		a++;
	}

	msg->msg_body.elem_nr = a;

	return 0;
}

/**
 * \brief Stop a running capture thread
 * \param[in,out] msg Capture thread message
 * \return 0 on success, else on failure.
 */

int dabbad_capture_stop(struct dabba_ipc_msg *msg)
{
	struct packet_rx_thread *pkt_capture;
	struct packet_thread *pkt_thread;
	struct dabba_capture *capture_msg = msg->msg_body.msg.capture;
	int rc = 0;

	pkt_thread = dabbad_thread_data_get(capture_msg->thread.id);

	if (!pkt_thread)
		return EINVAL;

	rc = dabbad_thread_stop(pkt_thread);

	if (!rc) {
		pkt_capture = dabbad_capture_thread_get(pkt_thread);
		close(pkt_capture->pcap_fd);
		packet_mmap_destroy(&pkt_capture->pkt_rx);
		free(pkt_capture);
	}

	return rc;
}
