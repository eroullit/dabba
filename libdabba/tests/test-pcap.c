
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <fcntl.h>
#include <byteswap.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <libdabba/pcap.h>
#include <libdabba/macros.h>
#include <linux/if_link.h>

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

void swapped_pcap_file_header_init(struct pcap_file_header *pcap_hdr)
{
	assert(pcap_hdr);

	memset(pcap_hdr, 0, sizeof(*pcap_hdr));

	pcap_hdr->magic = bswap_32(TCPDUMP_MAGIC);
	pcap_hdr->version_major = bswap_16(PCAP_VERSION_MAJOR);
	pcap_hdr->version_minor = bswap_16(PCAP_VERSION_MINOR);
	pcap_hdr->thiszone = 0;
	pcap_hdr->sigfigs = 0;
	pcap_hdr->snaplen = bswap_32(PCAP_DEFAULT_SNAPSHOT_LEN);
	pcap_hdr->linktype = bswap_32(LINKTYPE_EN10MB);
}

int test_pcap_write(const int fd, const uint8_t * const payload,
		    const ssize_t len)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);

	if (ldab_pcap_write(fd, payload, len, len, tv.tv_sec, tv.tv_usec) != len) {
		return (-1);
	}

	return (0);
}

int main(void)
{
	struct pcap_file_header pcap_hdr;
	int fd;

	swapped_pcap_file_header_init(&pcap_hdr);

	assert((fd = ldab_pcap_create(test_path, LINKTYPE_EN10MB)) > 0);
	assert(test_pcap_write(fd, icmp_dns, sizeof(icmp_dns)) == 0);
	assert(ldab_pcap_close(fd) == 0);

	assert((fd = ldab_pcap_open(test_path, O_RDONLY)) > 0);
	assert(ldab_pcap_close(fd) == 0);

	assert((fd = ldab_pcap_open(test_path, O_RDWR | O_APPEND)) > 0);
	assert(test_pcap_write(fd, icmp_dns, sizeof(icmp_dns)) == 0);
	assert(ldab_pcap_close(fd) == 0);

	assert((fd = ldab_pcap_open(test_path, O_RDONLY)) > 0);
	assert(ldab_pcap_close(fd) == 0);

	/* Test swapped PCAP file header support */
	assert((fd = open(test_path, O_WRONLY)) > 0);
	assert(write(fd, &pcap_hdr, sizeof(pcap_hdr)) == sizeof(pcap_hdr));
	assert(test_pcap_write(fd, icmp_dns, sizeof(icmp_dns)) == 0);
	assert(close(fd) == 0);

	assert((fd = ldab_pcap_open(test_path, O_RDONLY)) > 0);
	assert(ldab_pcap_close(fd) == 0);

	ldab_pcap_destroy(fd, test_path);

	return (EXIT_SUCCESS);
}
