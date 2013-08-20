/**
 * \file packet-tx.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */


#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <sys/uio.h>
#include <poll.h>

#include <libdabba/packet-tx.h>
#include <libdabba/pcap.h>

int ldab_packet_tx_loss_set(const int sock, const int discard)
{
	return (setsockopt
		(sock, SOL_PACKET, PACKET_LOSS, (void *)&discard,
		 sizeof(discard)));
}

/**
 * \brief Transmit packets coming from a packet mmap TX ring
 * \param[in] arg	Pointer to packet tx thread structure
 * \return Always return NULL
 */

void *ldab_packet_tx(void *arg)
{
	struct packet_tx *pkt_tx = arg;
	struct packet_mmap *pkt_mmap = &pkt_tx->pkt_mmap;
	struct packet_mmap_header *mmap_hdr;
	struct pollfd pfd;
	size_t a = 0;
	ssize_t obytes;
	int eof = 0;
	size_t tplen = TPACKET_ALIGN(sizeof(mmap_hdr->tp_h));

	if (!arg)
		return NULL;

	memset(&pfd, 0, sizeof(pfd));

	pfd.events = POLLOUT;
	pfd.fd = pkt_mmap->pf_sock;

	for (;;) {
		do {
			for (a = 0; a < pkt_mmap->layout.tp_frame_nr; a++) {
				mmap_hdr = pkt_mmap->vec[a].iov_base;

				if (mmap_hdr->tp_h.tp_status ==
				    TP_STATUS_AVAILABLE) {
					uint8_t *pkt =
					    (uint8_t *) mmap_hdr + tplen;

					obytes =
					    ldab_pcap_read(pkt_tx->pcap_fd, pkt,
							  pkt_mmap->layout.
							  tp_frame_size);

					if (obytes <= 0) {
						eof = 1;
						break;
					}

					mmap_hdr->tp_h.tp_len = obytes;
					mmap_hdr->tp_h.tp_snaplen = obytes;
					mmap_hdr->tp_h.tp_status =
					    TP_STATUS_SEND_REQUEST;
				}
			}

			send(pkt_mmap->pf_sock, NULL, 0, MSG_DONTWAIT);
		} while (!eof);

		ldab_pcap_rewind(pkt_tx->pcap_fd);
		eof = 0;
	}

	return NULL;
}
