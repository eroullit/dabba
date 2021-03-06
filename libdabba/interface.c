/**
 * \file interface.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */


#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <linux/if_ether.h>
#include <linux/ethtool.h>
#include <linux/sockios.h>

#include <libdabba/interface.h>

/**
 * \internal
 * \brief Perform an \c ioctl(2) call on an interface request structure
 * \param[in]	ifr	Pointer to interface request structure to use
 * \param[out]	request Ioctl command to perform
 * \return 0 on success, \c errno from \c socket(2), \c ioctl(2)
 * \return or \c close(2) on failure
 */

static int dev_kernel_request(struct ifreq *ifr, const int request)
{
	int rc, sock;

	assert(ifr);

	sock = socket(AF_INET, SOCK_DGRAM, 0);

	if (sock < 0)
		return errno;

	rc = ioctl(sock, request, ifr);

	close(sock);

	return rc ? errno : rc;
}

/**
 * \internal
 * \brief Perform an ethtool request via \c ioctl(2)
 * \param[in]       dev	        interface name
 * \param[in]       cmd 	ethtool command to run
 * \param[in,out]   value 	Pointer to the value to use for the command
 * \return 0 on success, non zero if the ethtool command reports an error
 * \see dev_kernel_request()
 */

static int dev_ethtool_request(const char *const dev, const int cmd, int *value)
{
	int rc;
	struct ethtool_value e;
	struct ifreq ifr;

	assert(dev);
	assert(value);

	memset(&e, 0, sizeof(e));
	memset(&ifr, 0, sizeof(ifr));

	strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name) - 1);
	e.cmd = cmd;
	e.data = *value;
	ifr.ifr_data = (caddr_t) & e;

	rc = dev_kernel_request(&ifr, SIOCETHTOOL);

	if (!rc)
		*value = e.data;

	return rc;
}

/**
 * \brief Get the interface index of a specific interface
 * \param[in]	dev	Device name
 * \param[out]	index	Interface index of the interface
 * \return 0 on success, non zero if the index could not be fetched
 * \see dev_kernel_request()
 *
 * This function queries the kernel about the interface
 * index related to the interface name.
 */

int ldab_devname_to_ifindex(const char *const dev, int *index)
{
	int rc;
	struct ifreq ifr;

	assert(dev);
	assert(index);

	if (strcmp(dev, ANY_INTERFACE) == 0) {
		*index = 0;
		return 0;
	}

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name) - 1);

	rc = dev_kernel_request(&ifr, SIOCGIFINDEX);

	if (!rc)
		*index = ifr.ifr_ifindex;

	return rc;
}

/**
 * \brief Get the interface name from an interface index
 * \param[in]	index	Interface index of the interface
 * \param[out]	dev	Device name
 * \param[in]	dev_len	Device name buffer length
 * \return 0 on success, non zero if the name could not be fetched
 * \see dev_kernel_request()
 *
 * This function queries the kernel about the interface
 * index related to the interface name.
 */

int ldab_ifindex_to_devname(const int index, char *dev, size_t dev_len)
{
	const char alldev[] = ANY_INTERFACE;
	struct ifreq ifr;
	int rc;

	assert(index >= 0);
	assert(dev);
	assert(dev_len >= IFNAMSIZ);

	if (index == 0) {
		strncpy(dev, alldev, dev_len - 1);
		return (0);
	}

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_ifindex = index;

	rc = dev_kernel_request(&ifr, SIOCGIFNAME);

	if (!rc)
		strncpy(dev, ifr.ifr_name, dev_len - 1);

	return rc;
}

/**
 * \brief Get the interface status flags
 * \param[in]       dev	        interface name
 * \param[out]      flags	current interface status flags
 * \return 0 on success, non zero if the interface status flags could not be fetched.
 * \see dev_kernel_request()
 */

int ldab_dev_flags_get(const char *const dev, short *flags)
{
	int rc;
	struct ifreq ifr;

	assert(dev);
	assert(flags);

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name) - 1);

	rc = dev_kernel_request(&ifr, SIOCGIFFLAGS);

	if (!rc)
		*flags = ifr.ifr_flags;

	return rc;
}

/**
 * \brief Set the interface status flags
 * \param[in]       dev	        interface name
 * \param[in]       flags	new interface status flags
 * \return 0 on success, non zero if the interface status flags could not be set.
 * \see dev_kernel_request()
 */

int ldab_dev_flags_set(const char *const dev, const short flags)
{
	struct ifreq ifr;

	assert(dev);

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name) - 1);
	ifr.ifr_flags = flags;

	return dev_kernel_request(&ifr, SIOCSIFFLAGS);
}

/**
 * \brief Get the interface driver information
 * \param[in]       dev	        interface name
 * \param[out]      driver	pointer to the driver info
 * \return 0 on success, non zero if the interface driver information could not be fetched.
 * \see dev_kernel_request()
 */

int ldab_dev_driver_get(const char *const dev, struct ethtool_drvinfo *driver)
{
	struct ifreq ifr;

	assert(dev);
	assert(driver);

	memset(&ifr, 0, sizeof(ifr));
	memset(driver, 0, sizeof(*driver));
	strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name) - 1);
	driver->cmd = ETHTOOL_GDRVINFO;
	ifr.ifr_data = (caddr_t) driver;

	return dev_kernel_request(&ifr, SIOCETHTOOL);
}

/**
 * \brief Get the interface hardware settings
 * \param[in]       dev	        interface name
 * \param[out]      settings	pointer to the interface hardware settings
 * \return 0 on success, non zero if the interface hardware settings could not be fetched.
 * \see dev_kernel_request()
 */

int ldab_dev_settings_get(const char *const dev, struct ethtool_cmd *settings)
{
	struct ifreq ifr;

	assert(dev);
	assert(settings);

	memset(&ifr, 0, sizeof(ifr));
	memset(settings, 0, sizeof(*settings));
	strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name) - 1);
	settings->cmd = ETHTOOL_GSET;
	ifr.ifr_data = (caddr_t) settings;

	return dev_kernel_request(&ifr, SIOCETHTOOL);
}

/**
 * \brief Set the interface hardware settings
 * \param[in]       dev	        interface name
 * \param[in]       settings	pointer to the interface hardware settings
 * \return 0 on success, non zero if the interface hardware settings could not be set.
 * \see dev_kernel_request()
 */

int ldab_dev_settings_set(const char *const dev, struct ethtool_cmd *settings)
{
	struct ifreq ifr;

	assert(dev);
	assert(settings);

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name) - 1);
	settings->cmd = ETHTOOL_SSET;
	ifr.ifr_data = (caddr_t) settings;

	return dev_kernel_request(&ifr, SIOCETHTOOL);
}

/**
 * \brief Get the interface pause settings
 * \param[in]       dev	        interface name
 * \param[out]      pause	pointer to the interface pause settings
 * \return 0 on success, non zero if the interface pause settings could not be fetched.
 * \see dev_kernel_request()
 */

int ldab_dev_pause_get(const char *const dev, struct ethtool_pauseparam *pause)
{
	struct ifreq ifr;

	assert(dev);
	assert(pause);

	memset(&ifr, 0, sizeof(ifr));
	memset(pause, 0, sizeof(*pause));
	strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name) - 1);
	pause->cmd = ETHTOOL_GPAUSEPARAM;
	ifr.ifr_data = (caddr_t) pause;

	return dev_kernel_request(&ifr, SIOCETHTOOL);
}

/**
 * \brief Set the interface pause settings
 * \param[in]       dev	        interface name
 * \param[in]      pause	pointer to the interface pause settings
 * \return 0 on success, non zero if the interface pause settings could not be set.
 * \see dev_kernel_request()
 */

int ldab_dev_pause_set(const char *const dev, struct ethtool_pauseparam *pause)
{
	struct ifreq ifr;

	assert(dev);
	assert(pause);

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name) - 1);
	pause->cmd = ETHTOOL_SPAUSEPARAM;
	ifr.ifr_data = (caddr_t) pause;

	return dev_kernel_request(&ifr, SIOCETHTOOL);
}

/**
 * \brief Get the interface coalesce settings
 * \param[in]       dev	        interface name
 * \param[out]      coalesce	pointer to the interface coalesce settings
 * \return 0 on success, non zero if the interface coalesce settings could not be fetched.
 * \see dev_kernel_request()
 */

int ldab_dev_coalesce_get(const char *const dev,
			 struct ethtool_coalesce *coalesce)
{
	struct ifreq ifr;

	assert(dev);
	assert(coalesce);

	memset(&ifr, 0, sizeof(ifr));
	memset(coalesce, 0, sizeof(*coalesce));
	strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name) - 1);
	coalesce->cmd = ETHTOOL_GCOALESCE;
	ifr.ifr_data = (caddr_t) coalesce;

	return dev_kernel_request(&ifr, SIOCETHTOOL);
}

/**
 * \brief Get the interface coalesce settings
 * \param[in]       dev	        interface name
 * \param[in]      coalesce	pointer to the interface coalesce settings
 * \return 0 on success, non zero if the interface coalesce settings could not be set.
 * \see dev_kernel_request()
 */

int ldab_dev_coalesce_set(const char *const dev,
			 struct ethtool_coalesce *coalesce)
{
	struct ifreq ifr;

	assert(dev);
	assert(coalesce);

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name) - 1);
	coalesce->cmd = ETHTOOL_SCOALESCE;
	ifr.ifr_data = (caddr_t) coalesce;

	return dev_kernel_request(&ifr, SIOCETHTOOL);
}

/**
 * \brief Get the interface receive checksum offload status
 * \param[in]       dev	        interface name
 * \param[out]      rx_csum	pointer to the receive checksum offload status
 * \return 0 on success, non zero if the interface receive checksum offload status could not be fetched.
 * \see dev_kernel_request()
 */

int ldab_dev_rx_csum_offload_get(const char *const dev, int *rx_csum)
{
	return dev_ethtool_request(dev, ETHTOOL_GRXCSUM, rx_csum);
}

/**
 * \brief Get the interface transmit checksum offload status
 * \param[in]       dev	        interface name
 * \param[out]      tx_csum	pointer to the transmit checksum offload status
 * \return 0 on success, non zero if the interface transmit checksum offload status could not be fetched.
 * \see dev_kernel_request()
 */

int ldab_dev_tx_csum_offload_get(const char *const dev, int *tx_csum)
{
	return dev_ethtool_request(dev, ETHTOOL_GTXCSUM, tx_csum);
}

/**
 * \brief Get the interface scatter gather status
 * \param[in]       dev	        interface name
 * \param[out]      sg		pointer to the transmit scatter gather status
 * \return 0 on success, non zero if the interface scatter gather status could not be fetched.
 * \see dev_kernel_request()
 */

int ldab_dev_scatter_gather_get(const char *const dev, int *sg)
{
	return dev_ethtool_request(dev, ETHTOOL_GSG, sg);
}

/**
 * \brief Get the interface tcp segment offload status
 * \param[in]       dev	        interface name
 * \param[out]      tso		pointer to the tcp segment offload status
 * \return 0 on success, non zero if the interface tcp segment offload status could not be fetched.
 * \see dev_kernel_request()
 */

int ldab_dev_tcp_seg_offload_get(const char *const dev, int *tso)
{
	return dev_ethtool_request(dev, ETHTOOL_GTSO, tso);
}

/**
 * \brief Get the interface udp fragment offload status
 * \param[in]       dev	        interface name
 * \param[out]      ufo		pointer to the udp fragment offload status
 * \return 0 on success, non zero if the interface udp fragment offload status could not be fetched.
 * * \see dev_kernel_request()
 */

int ldab_dev_udp_frag_offload_get(const char *const dev, int *ufo)
{
	return dev_ethtool_request(dev, ETHTOOL_GUFO, ufo);
}

/**
 * \brief Get the interface generic segmentation offload status
 * \param[in]       dev	        interface name
 * \param[out]      gso		pointer to the generic segmentation offload status
 * \return 0 on success, non zero if the interface generic segmentation offload status could not be fetched.
 * \see dev_kernel_request()
 */

int ldab_dev_generic_seg_offload_get(const char *const dev, int *gso)
{
	return dev_ethtool_request(dev, ETHTOOL_GGSO, gso);
}

/**
 * \brief Get the interface generic receive offload status
 * \param[in]       dev	        interface name
 * \param[out]      gro		pointer to the generic receive offload status
 * \return 0 on success, non zero if the interface generic receive offload status could not be fetched.
 * \see dev_kernel_request()
 */

int ldab_dev_generic_rcv_offload_get(const char *const dev, int *gro)
{
	return dev_ethtool_request(dev, ETHTOOL_GGRO, gro);
}

/**
 * \brief Get the interface large receive offload status
 * \param[in]       dev	        interface name
 * \param[out]      lro		pointer to the large receive offload status
 * \return 0 on success, non zero if the interface large receive offload status could not be fetched.
 * \see dev_kernel_request()
 */

int ldab_dev_large_rcv_offload_get(const char *const dev, int *lro)
{
	int rc;
	int flags = 0;

	rc = dev_ethtool_request(dev, ETHTOOL_GFLAGS, &flags);
	*lro = flags & ETH_FLAG_LRO;

	return rc;
}

/**
 * \brief Get the interface receive hashing offload status
 * \param[in]       dev	        interface name
 * \param[out]      rxhash	pointer to the receive hashing offload status
 * \return 0 on success, non zero if the interface receive hashing offload status could not be fetched.
 * \see dev_kernel_request()
 */

int ldab_dev_rx_hash_offload_get(const char *const dev, int *rxhash)
{
	int rc;
	int flags = 0;

	rc = dev_ethtool_request(dev, ETHTOOL_GFLAGS, &flags);
	*rxhash = flags & ETH_FLAG_RXHASH;

	return rc;
}

/**
 * \brief Set the interface receive checksum offload status
 * \param[in]       dev	        interface name
 * \param[in]       rx_csum	pointer to the receive checksum offload status
 * \return 0 on success, non zero if the interface receive checksum offload status could not be set.
 * \see dev_kernel_request()
 */

int ldab_dev_rx_csum_offload_set(const char *const dev, int rx_csum)
{
	return dev_ethtool_request(dev, ETHTOOL_SRXCSUM, &rx_csum);
}

/**
 * \brief Set the interface transmit checksum offload status
 * \param[in]       dev	        interface name
 * \param[in]       tx_csum	pointer to the transmit checksum offload status
 * \return 0 on success, non zero if the interface transmit checksum offload status could not be set.
 * \see dev_kernel_request()
 */

int ldab_dev_tx_csum_offload_set(const char *const dev, int tx_csum)
{
	return dev_ethtool_request(dev, ETHTOOL_STXCSUM, &tx_csum);
}

/**
 * \brief Set the interface scatter gather status
 * \param[in]       dev	        interface name
 * \param[in]       sg		pointer to the transmit scatter gather status
 * \return 0 on success, non zero if the interface scatter gather status could not be set.
 * \see dev_kernel_request()
 */

int ldab_dev_scatter_gather_set(const char *const dev, int sg)
{
	return dev_ethtool_request(dev, ETHTOOL_SSG, &sg);
}

/**
 * \brief Set the interface tcp segment offload status
 * \param[in]       dev	        interface name
 * \param[in]       tso		pointer to the tcp segment offload status
 * \return 0 on success, non zero if the interface tcp segment offload status could not be set.
 * \see dev_kernel_request()
 */

int ldab_dev_tcp_seg_offload_set(const char *const dev, int tso)
{
	return dev_ethtool_request(dev, ETHTOOL_STSO, &tso);
}

/**
 * \brief Set the interface udp fragment offload status
 * \param[in]       dev	        interface name
 * \param[in]       ufo		pointer to the udp fragment offload status
 * \return 0 on success, non zero if the interface udp fragment offload status could not be set.
 * \see dev_kernel_request()
 */

int ldab_dev_udp_frag_offload_set(const char *const dev, int ufo)
{
	return dev_ethtool_request(dev, ETHTOOL_SUFO, &ufo);
}

/**
 * \brief Set the interface generic segmentation offload status
 * \param[in]       dev	        interface name
 * \param[in]       gso		pointer to the generic segmentation offload status
 * \return 0 on success, non zero if the interface generic segmentation offload status could not be set.
 * \see dev_kernel_request()
 */

int ldab_dev_generic_seg_offload_set(const char *const dev, int gso)
{
	return dev_ethtool_request(dev, ETHTOOL_SGSO, &gso);
}

/**
 * \brief Set the interface generic receive offload status
 * \param[in]       dev	        interface name
 * \param[in]       gro		pointer to the generic receive offload status
 * \return 0 on success, non zero if the interface generic receive offload status could not be set.
 * \see dev_kernel_request()
 */

int ldab_dev_generic_rcv_offload_set(const char *const dev, int gro)
{
	return dev_ethtool_request(dev, ETHTOOL_SGRO, &gro);
}

/**
 * \brief Set the interface large receive offload status
 * \param[in]       dev	        interface name
 * \param[in]       lro		pointer to the large receive offload status
 * \return 0 on success, non zero if the interface large receive offload status could not be set.
 * \see dev_kernel_request()
 */

int ldab_dev_large_rcv_offload_set(const char *const dev, int lro)
{
	int rc;
	int flags = 0;

	rc = dev_ethtool_request(dev, ETHTOOL_GFLAGS, &flags);

	if (rc)
		return rc;

	if (lro)
		flags |= ETH_FLAG_LRO;
	else
		flags &= ~ETH_FLAG_LRO;

	return dev_ethtool_request(dev, ETHTOOL_SFLAGS, &flags);
}

/**
 * \brief Set the interface receive hashing offload status
 * \param[in]       dev	        interface name
 * \param[in]       rxhash	pointer to the receive hashing offload status
 * \return 0 on success, non zero if the interface receive hashing offload status could not be set.
 * \see dev_kernel_request()
 */

int ldab_dev_rx_hash_offload_set(const char *const dev, int rxhash)
{
	int rc;
	int flags = 0;

	rc = dev_ethtool_request(dev, ETHTOOL_GFLAGS, &flags);

	if (rc)
		return rc;

	if (rxhash)
		flags |= ETH_FLAG_RXHASH;
	else
		flags &= ~ETH_FLAG_RXHASH;

	return dev_ethtool_request(dev, ETHTOOL_SFLAGS, &flags);
}

/**
 * \brief Get the interface link status
 * \param[in]       dev	        interface name
 * \param[out]      link	pointer to the interface link status
 * \return 0 on success, non zero if the interface link status could not be fetched.
 * \see dev_kernel_request()
 */

int ldab_dev_link_get(const char *const dev, int *link)
{
	return dev_ethtool_request(dev, ETHTOOL_GLINK, link);
}
