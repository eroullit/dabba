/**
 * \file packet-rx.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2011
 * \date 2011
 */

/* __LICENSE_HEADER_BEGIN__ */

/*
 * Copyright (C) 2011	Emmanuel Roullit <emmanuel.roullit@gmail.com>
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

#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/param.h>
#include <poll.h>

#include <libdabba/packet-rx.h>
#include <libdabba/pcap.h>
#include <libdabba/macros.h>

/**
 * \brief Receive packets coming from a packet mmap RX ring
 * \param[in] arg	Pointer to packet rx thread structure
 * \return Always return NULL
 *
 * This function will \c poll(2) until some packets are received on the configured
 * interface. For now this function does not much but it can be starting point
 * for PCAP function to dump the frames into a file or for a packet dissectors.
 */

void *ldab_packet_rx(void *arg)
{
	struct packet_rx *pkt_rx = arg;
	struct packet_mmap *pkt_mmap = &pkt_rx->pkt_mmap;
	struct pollfd pfd;
	size_t index = 0;

	if (!arg)
		return NULL;

	memset(&pfd, 0, sizeof(pfd));

	pfd.events = POLLIN | POLLRDNORM | POLLERR;
	pfd.fd = pkt_mmap->pf_sock;

	for (;;) {
		for (index = 0; index < pkt_mmap->layout.tp_frame_nr; index++) {
			struct packet_mmap_header *mmap_hdr =
			    pkt_mmap->vec[index].iov_base;

			if (mmap_hdr->tp_h.tp_status == TP_STATUS_KERNEL) {
				if (poll(&pfd, 1, -1) < 0)
					continue;
			}

			if ((mmap_hdr->tp_h.tp_status & TP_STATUS_USER) ==
			    TP_STATUS_USER) {
				if (pkt_rx->pcap_fd > 0) {
					ldab_pcap_write(pkt_rx->pcap_fd,
						       (uint8_t *) mmap_hdr +
						       mmap_hdr->tp_h.tp_mac,
						       mmap_hdr->tp_h.tp_len,
						       MIN(mmap_hdr->tp_h.
							   tp_snaplen,
							   pkt_mmap->layout.
							   tp_frame_size),
						       mmap_hdr->tp_h.tp_sec,
						       mmap_hdr->tp_h.tp_usec);
				}

				mmap_hdr->tp_h.tp_status = TP_STATUS_KERNEL;
			}
		}
	}

	return NULL;
}
