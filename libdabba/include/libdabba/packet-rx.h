/**
 * \file packet-rx.h
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */


#ifndef PACKET_RX_H
#define	PACKET_RX_H

#include <linux/filter.h>
#include <libdabba/packet-mmap.h>

/**
 * \brief Packet capture structure
 */

struct packet_rx {
	struct packet_mmap pkt_mmap; /**< capture packet mmap structure */
	struct sock_fprog sfp; /**< socket program for the capture packet mmap */
	int pcap_fd; /**< pcap file descriptor */
};

void *ldab_packet_rx(void *arg);

#endif				/* PACKET_RX_H */
