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

#ifndef PACKET_RX_H
#define	PACKET_RX_H

#include <pthread.h>

#include <dabbacore/packet_mmap.h>

struct packet_rx_thread {
	struct packet_mmap pkt_rx;
	pthread_t thread;
	int pcap_fd;
};

void *packet_rx(void *arg);

#endif				/* PACKET_RX_H */
