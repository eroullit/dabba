
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/socket.h>
#include <sys/user.h>
#include <arpa/inet.h>

#include <linux/if_ether.h>

#include <libdabba/macros.h>
#include <libdabba/interface.h>
#include <libdabba/packet-mmap.h>

#define MIN_FRAME_NR (1<<3)
#define MAX_FRAME_NR (1<<16)

int main(int argc, char **argv)
{
	int rc;
	size_t a, i, fnr;
	int pf_sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	struct packet_mmap pkt_rx;
	enum packet_mmap_type types[] = { PACKET_MMAP_RX, PACKET_MMAP_TX };
	enum packet_mmap_frame_size fsize[] =
	    { PACKET_MMAP_ETH_FRAME_LEN, PACKET_MMAP_JUMBO_FRAME_LEN,
		PACKET_MMAP_SUPER_JUMBO_FRAME_LEN
	};

	assert(argc);
	assert(argv);

	assert(pf_sock > 0);

	for (a = 0; a < ARRAY_SIZE(types); a++)
		for (i = 0; i < ARRAY_SIZE(fsize); i++)
			for (fnr = MIN_FRAME_NR; fnr < MAX_FRAME_NR; fnr <<= 1) {
				rc = ldab_packet_mmap_create(&pkt_rx,
							    ANY_INTERFACE,
							    pf_sock, types[a],
							    fsize[i], fnr);

				printf("packet mmap type: %i frame number=%zu",
				       types[a], fnr);
				printf(" frame size=%i rc=%s\n", fsize[i],
				       strerror(rc));
				/*
				 * Tolerate ENOMEM as we cannot accurately foresee
				 * if the allocation will fail or succeed
				 */
				assert(rc == 0 || rc == ENOMEM);
				ldab_packet_mmap_destroy(&pkt_rx);
			}

	return (EXIT_SUCCESS);
}
