/**
 * \file pktcnt.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */

/* __LICENSE_HEADER_BEGIN__ */

/*
 * Copyright (C) 2013	Emmanuel Roullit <emmanuel.roullit@gmail.com>
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
	int fd = pcap_open(pcap, 0);

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
