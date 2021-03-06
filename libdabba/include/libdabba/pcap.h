/**
 * \file pcap.h
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */


#ifndef PCAP_H
#define	PCAP_H

#include <stdint.h>
#include <netinet/if_ether.h>
#include <linux/if_packet.h>

/** \brief pcap file magic value */
#ifndef TCPDUMP_MAGIC
#define TCPDUMP_MAGIC               0xa1b2c3d4
#endif				/* TCPDUMP_MAGIC */

/** \brief pcap version major */
#ifndef PCAP_VERSION_MAJOR
#define PCAP_VERSION_MAJOR          2
#endif				/* PCAP_VERSION_MAJOR */

/** \brief pcap version minor */
#ifndef PCAP_VERSION_MINOR
#define PCAP_VERSION_MINOR          4
#endif				/* PCAP_VERSION_MINOR */

/** \brief pcap default snapshot packet length */
#ifndef PCAP_DEFAULT_SNAPSHOT_LEN
#define PCAP_DEFAULT_SNAPSHOT_LEN   65535
#endif				/* PCAP_DEFAULT_SNAPSHOT_LEN */

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
	uint32_t linktype;	/**< data link type (\c LINKTYPE_*) */
};

/**
 * \brief PCAP specific timestamp
 *
 * This is a timeval as stored in a savefile.
 * It has to use the same types everywhere, independent of the actual
 * <tt>struct timeval</tt>; <tt>struct timeval</tt> has 32-bit \c tv_sec
 * values on some platforms and 64-bit \c tv_sec values on other platforms,
 * and writing out native <tt>struct timeval</tt> values would mean files
 * could only be read on systems with the same \c tv_sec size as the system
 * on which the file was written.
 */

struct pcap_timeval {
	int32_t tv_sec;		/**< seconds */
	int32_t tv_usec;	/**< microseconds */
};

/**
 * \brief Structure describing per-packet information
 *
 * The time stamp can and should be a <tt>struct timeval</tt>, regardless of
 * whether your system supports 32-bit \c tv_sec in <tt>struct timeval</tt>,
 * 64-bit \ ctv_sec in <tt>struct timeval</tt>, or both if it supports both 32-bit
 * and 64-bit applications.  The on-disk format of savefiles uses 32-bit
 * \c tv_sec (and \c tv_usec); this structure is irrelevant to that.  32-bit
 * and 64-bit versions of libpcap, even if they're on the same platform,
 * should supply the appropriate version of <tt>struct timeval</tt>, even if
 * that's not what the underlying packet capture mechanism supplies.
 */

struct pcap_sf_pkthdr {
	struct pcap_timeval ts;	/**< timestamp */
	uint32_t caplen;	/**< length of portion captured */
	uint32_t len;		/**< length this packet (off wire) */
};

int ldab_pcap_link_type_get(int arp_type, enum pcap_linktype *pcap_link_type);
ssize_t ldab_pcap_write(const int fd, const uint8_t * const pkt,
		       const size_t pkt_len, const size_t pkt_snaplen,
		       const uint64_t tv_sec, const uint64_t tv_usec);
ssize_t ldab_pcap_read(const int fd, uint8_t * pkt, const uint32_t pkt_len);
void ldab_pcap_destroy(const int fd, const char *const pcap_path);
int ldab_pcap_create(const char *const pcap_path,
		    const enum pcap_linktype linktype);
int ldab_pcap_open(const char *const pcap_path, int flags);
int ldab_pcap_close(const int fd);
int ldab_pcap_rewind(const int fd);

#endif				/* PCAP_H */
