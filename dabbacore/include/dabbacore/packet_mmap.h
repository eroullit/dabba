/**
 * \file packet_mmap.c
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

#ifndef PACKET_MMAP_H
#define	PACKET_MMAP_H

#include <stdint.h>
#include <linux/if_packet.h>

enum packet_mmap_type {
	PACKET_MMAP_RX = PACKET_RX_RING
};

enum packet_mmap_frame_size {
	PACKET_MMAP_ETH_FRAME_LEN = 16 << 7,
	PACKET_MMAP_JUMBO_FRAME_LEN = 16 << 10,
	PACKET_MMAP_SUPER_JUMBO_FRAME_LEN = 16 << 12
};

struct packet_mmap {
	enum packet_mmap_type type;
	int pf_sock;
	int ifindex;
	struct tpacket_req layout;
	uint64_t used_mask;
	struct iovec *vec;
	uint8_t *buf;
};

struct packet_mmap_header {
	struct tpacket_hdr tp_h __attribute__ ((aligned(TPACKET_ALIGNMENT)));
	struct sockaddr_ll s_ll __attribute__ ((aligned(TPACKET_ALIGNMENT)));
};

int packet_mmap_create(struct packet_mmap *pkt_mmap,
		       const char *const dev, const int pf_sock,
		       const enum packet_mmap_type type,
		       const enum packet_mmap_frame_size frame_size,
		       const uint8_t page_order, const size_t size);

void packet_mmap_destroy(struct packet_mmap *pkt_mmap);

static inline int packet_mmap_frame_size_is_valid(const uint64_t frame_size)
{
	switch (frame_size) {
	case PACKET_MMAP_ETH_FRAME_LEN:
	case PACKET_MMAP_JUMBO_FRAME_LEN:
	case PACKET_MMAP_SUPER_JUMBO_FRAME_LEN:
		return 1;
		break;
	default:
		return 0;
	}
}

#endif				/* PACKET_MMAP_H */
