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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/socket.h>
#include <sys/user.h>
#include <arpa/inet.h>

#include <linux/if_ether.h>

#include <dabbacore/macros.h>
#include <dabbacore/nic.h>
#include <dabbacore/packet_mmap.h>

#define MIN_FRAME_NR 1<<3
#define MAX_FRAME_NR 1<<16

int main(int argc, char **argv)
{
	int rc;
	size_t a;
	int pf_sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	struct packet_mmap pkt_rx;

	assert(argc);
	assert(argv);

	assert(pf_sock > 0);

	for (a = 1 << 3; a < 1 << 16; a <<= 1) {
		rc = packet_mmap_create(&pkt_rx, ANY_INTERFACE,
					pf_sock, PACKET_MMAP_RX,
					PACKET_MMAP_ETH_FRAME_LEN, a);

		printf("RX packet mmap: frame number=%zu", a);
		printf(" frame size=%i rc=%s\n", PACKET_MMAP_ETH_FRAME_LEN,
		       strerror(rc));

		assert(rc == 0);
		packet_mmap_destroy(&pkt_rx);
	}

	for (a = 1 << 3; a < 1 << 15; a <<= 1) {
		rc = packet_mmap_create(&pkt_rx, ANY_INTERFACE,
					pf_sock, PACKET_MMAP_RX,
					PACKET_MMAP_JUMBO_FRAME_LEN, a);

		printf("RX packet mmap: frame number=%zu", a);
		printf(" frame size=%i rc=%s\n", PACKET_MMAP_JUMBO_FRAME_LEN,
		       strerror(rc));

		assert(rc == 0);
		packet_mmap_destroy(&pkt_rx);
	}

	for (a = 1 << 3; a < 1 << 14; a <<= 1) {
		rc = packet_mmap_create(&pkt_rx, ANY_INTERFACE,
					pf_sock, PACKET_MMAP_RX,
					PACKET_MMAP_SUPER_JUMBO_FRAME_LEN, a);

		printf("RX packet mmap: frame number=%zu", a);
		printf(" frame size=%i rc=%s\n",
		       PACKET_MMAP_SUPER_JUMBO_FRAME_LEN, strerror(rc));

		assert(rc == 0);
		packet_mmap_destroy(&pkt_rx);

	}

	return (EXIT_SUCCESS);
}
