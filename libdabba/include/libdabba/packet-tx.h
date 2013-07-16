/**
 * \file packet-tx.h
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

#ifndef PACKET_TX_H
#define	PACKET_TX_H

#include <libdabba/packet-mmap.h>

/**
 * \brief Packet replay structure
 */

struct packet_tx {
	struct packet_mmap pkt_mmap; /**< transmit packet mmap structure */
	int pcap_fd; /**< pcap file descriptor */
};

int packet_tx_loss_set(const int sock, const int discard);
void *packet_tx(void *arg);

#endif				/* PACKET_TX_H */