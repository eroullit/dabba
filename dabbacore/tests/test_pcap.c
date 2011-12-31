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

#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <dabbacore/pcap.h>
#include <dabbacore/macros.h>

static const uint8_t icmp_dns[] = {
	0x00, 0x1e, 0x65, 0x93, 0x1b, 0x6c, 0x00, 0x1d,
	0x19, 0x84, 0x9c, 0xdc, 0x08, 0x00, 0x45, 0x00,
	0x00, 0x54, 0xdb, 0x46, 0x00, 0x00, 0x38, 0x01,
	0x4d, 0x41, 0x08, 0x08, 0x08, 0x08, 0xc0, 0xa8,
	0x89, 0x69, 0x00, 0x00, 0xce, 0x1a, 0x12, 0x2d,
	0x00, 0x02, 0xb7, 0xeb, 0xba, 0x4c, 0x00, 0x00,
	0x00, 0x00, 0xee, 0xaa, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
	0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
	0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
	0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,
	0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
	0x36, 0x37
};

static const char test_path[] = "res.pcap";

int test_pcap_write(const int fd, const uint8_t * const payload,
		    const ssize_t len)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	if (pcap_write(fd, payload, len, len, tv.tv_sec, tv.tv_usec) != len) {
		return (-1);
	}

	return (0);
}

int main(void)
{
	int fd;

	/* Create PCAP file */
	assert((fd = pcap_create(test_path, LINKTYPE_EN10MB)) > 0);

	/* Write payload */
	assert(test_pcap_write(fd, icmp_dns, sizeof(icmp_dns)) == 0);

	assert(pcap_close(fd) == 0);

	assert((fd = pcap_open(test_path, O_RDONLY)) > 0);

	assert(pcap_close(fd) == 0);

	assert((fd = pcap_open(test_path, O_RDWR | O_APPEND)) > 0);

	/* Write payload */
	assert(test_pcap_write(fd, icmp_dns, sizeof(icmp_dns)) == 0);

	assert(pcap_close(fd) == 0);

	assert((fd = pcap_open(test_path, O_RDONLY)) > 0);
	assert(pcap_is_valid(fd) == 0);
	assert(pcap_close(fd) == 0);

	pcap_destroy(fd, test_path);

	return (EXIT_SUCCESS);
}
