/**
 * \file packet-mmap.h
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */


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

int ldab_packet_mmap_create(struct packet_mmap *pkt_mmap,
			   const char *const dev, const int pf_sock,
			   const enum packet_mmap_type type,
			   const enum packet_mmap_frame_size frame_size,
			   const size_t frame_nr);

void ldab_packet_mmap_destroy(struct packet_mmap *pkt_mmap);

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
