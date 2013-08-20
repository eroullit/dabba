/**
 * \file pktcnt.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */


#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include <libdabba/pcap.h>

/**
 * \internal
 * \brief Count the number of packet within a pcap file
 * \param[in] pcap path to pcap file
 * \return The number of packets in a pcap, 0 on error
 */

static int pktcnt(const char *const pcap)
{
	size_t a;
	struct pcap_sf_pkthdr sf_hdr;
	int fd = ldab_pcap_open(pcap, 0);

	if (fd < 0)
		return 0;

	for (a = 0; read(fd, &sf_hdr, sizeof(sf_hdr)) == sizeof(sf_hdr); a++)
		if (lseek(fd, sf_hdr.caplen, SEEK_CUR) < 0)
			break;

	close(fd);

	return a;
}

int main(int argc, char **argv)
{
	int a;

	for (a = 1; a < argc; a++)
		printf("%i\n", pktcnt(argv[a]));

	return 0;
}
