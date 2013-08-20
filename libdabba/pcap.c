/**
 * \file pcap.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <byteswap.h>

#include <sys/stat.h>
#include <sys/param.h>

#include <libdabba/pcap.h>
#include <libdabba/macros.h>

/**
 * \internal
 * \brief Write the PCAP file header on a file descriptor
 * \param[in] fd PCAP file descriptor
 * \param[in] linktype PCAP link type
 * \param[in] thiszone Timezone where the PCAP is created
 * \param[in] snaplen Maximum length of a captured packet
 * \return 0 on success, -1 if PCAP file header could not be written
 */

static int
pcap_file_header_write(const int fd, const int linktype,
		       const int thiszone, const int snaplen)
{
	struct pcap_file_header hdr;

	assert(fd > 0);

	memset(&hdr, 0, sizeof(hdr));

	hdr.magic = TCPDUMP_MAGIC;
	hdr.version_major = PCAP_VERSION_MAJOR;
	hdr.version_minor = PCAP_VERSION_MINOR;
	hdr.thiszone = thiszone;
	hdr.snaplen = snaplen;
	hdr.sigfigs = 0;
	hdr.linktype = linktype;

	if (write(fd, (char *)&hdr, sizeof(hdr)) != sizeof(hdr)) {
		return (-1);
	}

	return (0);
}

/**
 * \brief Get PCAP link type from NIC ARP type
 * \param[in] arp_type ARP type value
 * \param[out] pcap_link_type Pointer to the PCAP link type
 * \return 0 on success, \c EINVAL when the ARP type is not supported
 */

int ldab_pcap_link_type_get(int arp_type, enum pcap_linktype *pcap_link_type)
{
	int rc = 0;

	assert(pcap_link_type);

	switch (arp_type) {
	case ARPHRD_ETHER:
	case ARPHRD_LOOPBACK:
		*pcap_link_type = LINKTYPE_EN10MB;
		break;
	default:
		rc = EINVAL;
		break;
	}

	return (rc);
}

/**
 * \internal
 * \brief Tells if the input linktype is valid
 * \param[in] linktype Linktype to validate
 * \return 1 if the linktype is valid, 0 if invalid
 */

static int pcap_linktype_is_valid(const uint32_t linktype)
{
	return (linktype == LINKTYPE_EN10MB);
}

/**
 * \internal
 * \brief Validate PCAP file header
 * Every PCAP file has a file header which contains:
 * 	- the PCAP magic (\c 0xa1b2c3d4)
 * 	- the PCAP version major/minor
 * 	- the PCAP linktype
 * 	- the timezone
 * 	- the maximum packet length
 * \param[in] fd PCAP file descriptor
 * \return	0 if the PCAP file header is not valid \n
 * 		1 if it is. \n
 * 		If PCAP file header is invalid, \c errno is set to \n
 * 		\c EINVAL if PCAP file descriptor or file header is invalid \n
 * 		\c EIO if PCAP file header could not be read
 */

static int pcap_is_valid(const int fd)
{
	struct pcap_file_header hdr;

	if (fd < 0) {
		errno = EINVAL;
		return (0);
	}

	if (read(fd, (char *)&hdr, sizeof(hdr)) != sizeof(hdr)) {
		errno = EIO;
		return (0);
	}

	/* PCAP might have been created on a system with another endianness */
	if (hdr.magic != TCPDUMP_MAGIC) {
		hdr.magic = bswap_32(hdr.magic);
		hdr.linktype = bswap_32(hdr.linktype);
		hdr.version_major = bswap_16(hdr.version_major);
		hdr.version_minor = bswap_16(hdr.version_minor);
	}

	if (hdr.magic != TCPDUMP_MAGIC
	    || hdr.version_major != PCAP_VERSION_MAJOR
	    || hdr.version_minor != PCAP_VERSION_MINOR
	    || !pcap_linktype_is_valid(hdr.linktype)) {
		errno = EINVAL;
		return (0);
	}

	return (1);
}

/**
 * \brief Create a PCAP file
 * \param[in] pcap_path	PCAP file path
 * \param[in] linktype PCAP link type
 * \return PCAP file descriptor on success, -1 on failure
 * \note It creates a PCAP file with default permissions
 * \note A created PCAP will have by default a snapshot length of 65535 bytes.
 */

int ldab_pcap_create(const char *const pcap_path,
		    const enum pcap_linktype linktype)
{
	assert(pcap_path);

	int fd;

	if ((fd = creat(pcap_path, DEFFILEMODE)) < 0) {
		return (-1);
	}

	/* TODO make it configurable instead of using default values */
	if (pcap_file_header_write(fd, linktype, 0, PCAP_DEFAULT_SNAPSHOT_LEN)) {
		/* When the PCAP header cannot be written the file
		 * must be closed and then deleted
		 */
		ldab_pcap_destroy(fd, pcap_path);
		fd = -1;
	}

	return (fd);
}

/**
 * \brief Destroy a PCAP file
 * \param[in] fd PCAP file descriptor
 * \param[in] pcap_path	PCAP file path
 */

void ldab_pcap_destroy(const int fd, const char *const pcap_path)
{
	assert(pcap_path);
	assert(fd > 0);

	close(fd);
	unlink(pcap_path);
}

/**
 * \brief Open a PCAP file
 * \param[in] pcap_path	PCAP file path
 * \param[in] flags flags for \c open(2)
 * \return PCAP file descriptor on success, -1 on failure
 * \note The flags given as parameter are directly given to \c open(2)
 */

int ldab_pcap_open(const char *const pcap_path, int flags)
{
	int append = 0;
	int fd;

	assert(pcap_path);

	/* Deactivate append to be able to check pcap validity */
	if ((flags & O_APPEND) == O_APPEND) {
		append = 1;
		flags &= ~O_APPEND;
	}

	if ((fd = open(pcap_path, flags)) < 0) {
		return (-1);
	}

	if (pcap_is_valid(fd) == 0) {
		ldab_pcap_close(fd);
		return (-1);
	}

	if (append) {
		/* Go to EOF */
		if (lseek(fd, 0, SEEK_END) < 0) {
			ldab_pcap_close(fd);
			return (-1);
		}
	}

	return (fd);
}

/**
 * \brief Close a PCAP file
 * \param[in] fd PCAP file descriptor
 * \return same error values as \c close(2)
 */

int ldab_pcap_close(const int fd)
{
	return (close(fd));
}

/**
 * \brief Write the packet payload on a file descriptor
 * \param[in] fd PCAP file descriptor
 * \param[in] pkt Pointer to the packet to write
 * \param[in] pkt_len Valid length of the packet
 * \param[in] pkt_snaplen Total length of the packet
 * \param[in] tv_sec Seconds after Epoch
 * \param[in] tv_usec Microseconds after Epoch
 * \return	Length of written packet on success,
 * 		-1 if either the packet header or packet payload could not be written
 */

ssize_t
ldab_pcap_write(const int fd, const uint8_t * const pkt,
	       const size_t pkt_len, const size_t pkt_snaplen,
	       const uint64_t tv_sec, const uint64_t tv_usec)
{
	struct pcap_sf_pkthdr sf_hdr;
	ssize_t written = 0;

	assert(fd > 0);
	assert(pkt);
	assert(pkt_snaplen);

	memset(&sf_hdr, 0, sizeof(sf_hdr));

	sf_hdr.ts.tv_sec = tv_sec;
	sf_hdr.ts.tv_usec = tv_usec;
	sf_hdr.caplen = pkt_snaplen;
	sf_hdr.len = pkt_len;

	written = write(fd, &sf_hdr, sizeof(sf_hdr));

	if (written != sizeof(sf_hdr)) {
		return (-1);
	}

	written = write(fd, pkt, sf_hdr.caplen);

	if (written != (ssize_t) sf_hdr.caplen) {
		return (-1);
	}

	return (written);
}

/**
 * \brief Get next packet of a PCAP
 * \param[in]  fd 	PCAP file descriptor
 * \param[out] pkt	Pointer to the output packet buffer
 * \param[in]  pkt_len	Length of the packet buffer
 * \return	Length valid date in the fetched packet. \n
 * 		0 if packet header or packet payload could not be read
 */

ssize_t ldab_pcap_read(const int fd, uint8_t * pkt, const uint32_t pkt_len)
{
	struct pcap_sf_pkthdr sf_hdr;

	assert(fd > 0);
	assert(pkt);
	assert(pkt_len);

	if (read(fd, &sf_hdr, sizeof(sf_hdr)) != sizeof(sf_hdr))
		return (0);

	return read(fd, pkt, MIN(sf_hdr.caplen, pkt_len));
}

/**
 * \brief Rewind PCAP file to its first packet
 * \param[in]  fd 	PCAP file descriptor
 * \return 0 on success, else on failure. Check \c errno for error code.
 */

int ldab_pcap_rewind(const int fd)
{
	return lseek(fd, sizeof(struct pcap_file_header), SEEK_SET) == 0;
}
