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
#include <assert.h>
#include <sys/socket.h>
#include <sys/user.h>
#include <arpa/inet.h>

#include <linux/if_ether.h>

#include <dabbacore/macros.h>
#include <dabbacore/nic.h>
#include <dabbacore/packet_mmap.h>

/* Find a way to know the target's MAX_ORDER */
#ifndef MAX_ORDER
#define MAX_ORDER 11
#endif

#define MIN_SIZE 8 * 1024
#define MAX_SIZE 1024 * 1024

int main(int argc, char **argv)
{
	int rc;
	size_t a, i, size;
	int pf_sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	int page_size = getpagesize();
	struct packet_mmap pkt_rx;

	enum packet_mmap_frame_size test_size[] = {
		PACKET_MMAP_ETH_FRAME_LEN,
		PACKET_MMAP_JUMBO_FRAME_LEN,
		PACKET_MMAP_SUPER_JUMBO_FRAME_LEN
	};

	assert(argc);
	assert(argv);

	assert(pf_sock > 0);

	for (size = MIN_SIZE; size <= MAX_SIZE; size *= 2)
		for (a = 0; a < ARRAY_SIZE(test_size); a++)
			for (i = 0; i <= MAX_ORDER; i++) {
				if ((page_size << i) < (int)test_size[a])
					continue;

				if ((int)size < (page_size << i))
					continue;

				rc = packet_mmap_create(&pkt_rx, ANY_INTERFACE,
							pf_sock, PACKET_MMAP_RX,
							test_size[a], i, size);

				printf("RX packet mmap: frame size=%i",
				       test_size[a]);
				printf(" page order=%zu size=%zu rc=%i\n", i,
				       size, rc);

				assert(rc == 0);
				packet_mmap_destroy(&pkt_rx);
			}

	return (EXIT_SUCCESS);
}
