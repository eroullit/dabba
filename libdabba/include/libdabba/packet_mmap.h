/**
 * \file packet_mmap.h
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

#ifndef PACKET_MMAP_H
#define	PACKET_MMAP_H

#include <stdint.h>
#include <linux/if_packet.h>

/**
 * \brief Supported packet mmap types
 */

enum packet_mmap_type {
	PACKET_MMAP_RX = PACKET_RX_RING,
	PACKET_MMAP_TX = PACKET_TX_RING
};

/**
 * \brief Supported packet mmap frame sizes
 */

enum packet_mmap_frame_size {
	PACKET_MMAP_ETH_FRAME_LEN = 16 << 7,
	PACKET_MMAP_JUMBO_FRAME_LEN = 16 << 10,
	PACKET_MMAP_SUPER_JUMBO_FRAME_LEN = 16 << 12
};

/**
 * \brief Packet mmap structure
 */

struct packet_mmap {
	enum packet_mmap_type type; /**< Packet mmap type */
	int pf_sock; /**< Packet family socket */
	int ifindex; /**< Interface index */
	struct tpacket_req layout; /**< Packet mmap layout */
	uint64_t used_mask; /**< Used packet bitmask */
	struct iovec *vec; /**< Packet I/O vector */
	uint8_t *buf; /**< Raw packet mmap buffer */
};

/**
 * \brief Packet mmap header
 */

struct packet_mmap_header {
	struct tpacket_hdr tp_h __attribute__ ((aligned(TPACKET_ALIGNMENT))); /**< Packet metadata structure */
	struct sockaddr_ll s_ll __attribute__ ((aligned(TPACKET_ALIGNMENT))); /**< Pocket metadata structure */
};

int packet_mmap_create(struct packet_mmap *pkt_mmap,
		       const char *const dev, const int pf_sock,
		       const enum packet_mmap_type type,
		       const enum packet_mmap_frame_size frame_size,
		       const size_t frame_nr);

void packet_mmap_destroy(struct packet_mmap *pkt_mmap);

/**
 * \brief Check if a frame size is valid
 * \param[in] frame_size	Frame size to check in bytes
 * \return 1 if valid, 0 if invalid
 *
 * The frame size must match one the of supported frame size to be considered
 * valid.
 */

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
