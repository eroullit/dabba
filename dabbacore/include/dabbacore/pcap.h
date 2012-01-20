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

#ifndef PCAP_H
#define	PCAP_H

#include <stdint.h>
#include <netinet/if_ether.h>
#include <linux/if_packet.h>

#define TCPDUMP_MAGIC               0xa1b2c3d4
#define PCAP_VERSION_MAJOR          2
#define PCAP_VERSION_MINOR          4
#define PCAP_DEFAULT_SNAPSHOT_LEN   65535

/** \brief Enum regrouping all possible PCAP link types */
enum pcap_linktype {
	LINKTYPE_NULL = 0,	/**< BSD loopback encapsulation */
	LINKTYPE_EN10MB = 1,	/**< Ethernet (10Mb) */
};

/** \brief Structure describing a PCAP file header */
struct pcap_file_header {
	uint32_t magic;		/**< if swapped, all fields must be swapped */
	uint16_t version_major;	/**< PCAP file major version */
	uint16_t version_minor;	/**< PCAP file minor version */
	int32_t thiszone;	/**< GMT to local correction leave it zero */
	uint32_t sigfigs;	/**< accuracy of timestamps. Set on 0 */
	uint32_t snaplen;	/**< max length saved portion of each pkt. */
	uint32_t linktype;	/**< data link type (LINKTYPE_*) */
};

/**
 * \brief PCAP specifix timestamp
 *
 * This is a timeval as stored in a savefile.
 * It has to use the same types everywhere, independent of the actual
 * `struct timeval'; `struct timeval' has 32-bit tv_sec values on some
 * platforms and 64-bit tv_sec values on other platforms, and writing
 * out native `struct timeval' values would mean files could only be
 * read on systems with the same tv_sec size as the system on which
 * the file was written.
 */

struct pcap_timeval {
	int32_t tv_sec;		/**< seconds */
	int32_t tv_usec;	/**< microseconds */
};

/**
 * \brief Structure describing per-packet information
 *
 * The time stamp can and should be a "struct timeval", regardless of
 * whether your system supports 32-bit tv_sec in "struct timeval",
 * 64-bit tv_sec in "struct timeval", or both if it supports both 32-bit
 * and 64-bit applications.  The on-disk format of savefiles uses 32-bit
 * tv_sec (and tv_usec); this structure is irrelevant to that.  32-bit
 * and 64-bit versions of libpcap, even if they're on the same platform,
 * should supply the appropriate version of "struct timeval", even if
 * that's not what the underlying packet capture mechanism supplies.
 */

struct pcap_sf_pkthdr {
	struct pcap_timeval ts;	/**< timestamp */
	uint32_t caplen;	/**< length of portion present */
	int32_t len;		/**< length this packet (off wire) */
};

static inline int is_linktype_valid(uint32_t linktype)
{
	return (linktype == LINKTYPE_EN10MB);
}

int pcap_link_type_get(int arp_type, enum pcap_linktype *pcap_link_type);
ssize_t pcap_write(const int fd, const uint8_t * const pkt,
		   const size_t pkt_len, const size_t pkt_snaplen,
		   const uint64_t tv_sec, const uint64_t tv_usec);
void pcap_destroy(const int fd, const char *const pcap_path);
int pcap_create(const char *const pcap_path, const enum pcap_linktype linktype);
int pcap_open(const char *const pcap_path, int flags);
int pcap_close(const int fd);
int pcap_is_valid(const int fd);

#endif				/* PCAP_H */