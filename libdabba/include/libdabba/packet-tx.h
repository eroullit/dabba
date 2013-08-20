/**
 * \file packet-tx.h
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */


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

int ldab_packet_tx_loss_set(const int sock, const int discard);
void *ldab_packet_tx(void *arg);

#endif				/* PACKET_TX_H */
