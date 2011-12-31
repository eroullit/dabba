/**
 * \file packet_rx.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2009-2011
 * \date 2011
 */

/* __LICENSE_HEADER_BEGIN__ */

/*
 * Copyright (C) 2009-2011	Emmanuel Roullit <emmanuel.roullit@gmail.com>
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

#define _GNU_SOURCE

#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>
#include <poll.h>

#include <dabbacore/macros.h>
#include <dabbacore/packet_mmap.h>
#include <dabbacore/pcap.h>

/**
 * \brief Receive packets coming from a packet mmap RX ring
 * \param[in] pkt_rx	packet mmap RX ring
 * \param[in] pcap_fd	PCAP file where to dump received packets
 * \return 0 on success, else on failure
 *
 * This function will poll(2) until some packets are received on the configured
 * interface. For now this function does not much but it can be starting point
 * for PCAP function to dump the frames into a file or for a packet dissectors.
 */

int packet_rx(const struct packet_mmap *pkt_rx, const int pcap_fd)
{
	struct pollfd pfd[2];
	size_t index = 0;

	assert(pkt_rx);

	memset(pfd, 0, sizeof(pfd));

	pfd[0].events = POLLIN | POLLRDNORM | POLLERR;
	pfd[0].fd = pkt_rx->pf_sock;

	pfd[1].events = POLLIN;
	pfd[1].fd = STDIN_FILENO;

	for (;;) {
		for (index = 0; index < pkt_rx->layout.tp_frame_nr; index++) {
			struct packet_mmap_header *mmap_hdr =
			    pkt_rx->vec[index].iov_base;

			if ((mmap_hdr->tp_h.tp_status & TP_STATUS_KERNEL) ==
			    TP_STATUS_KERNEL) {
				if (poll(pfd, ARRAY_SIZE(pfd), -1) < 0)
					continue;

				if ((pfd[1].revents & POLLIN) == POLLIN)
					goto out;
			}

			if ((mmap_hdr->tp_h.tp_status & TP_STATUS_USER) ==
			    TP_STATUS_USER) {
				if (pcap_fd > 0) {
					pcap_write(pcap_fd,
						   (uint8_t *) mmap_hdr +
						   mmap_hdr->tp_h.tp_mac,
						   mmap_hdr->tp_h.tp_len,
						   mmap_hdr->tp_h.tp_snaplen,
						   mmap_hdr->tp_h.tp_sec,
						   mmap_hdr->tp_h.tp_usec);
				}

				mmap_hdr->tp_h.tp_status = TP_STATUS_KERNEL;
			}
		}
	}

 out:
	return (0);
}
