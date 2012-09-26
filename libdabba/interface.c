/**
 * \file nic.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2011
 * \date 2011
 */

/* __LICENSE_HEADER_BEGIN__ */

/*
 * Copyright (C) 2011	Emmanuel Roullit <emmanuel.roullit@gmail.com>
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
#include <libdabba/strlcpy.h>

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
 * \brief Get the interface index of a specific interface
 * \param[in]	dev	Device name
 * \param[out]	index	Interface index of the interface
 * \return 0 on success, errno from \c socket(2), \c ioctl(2) 
 * \return or \c close(2) on failure
 *
 * This function queries the kernel about the interface
 * index related to the interface name.
 */

int devname_to_ifindex(const char *const dev, int *index)
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
	strlcpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));

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
 * \return 0 on success, errno from \c socket(2), \c ioctl(2) 
 * \return or \c close(2) on failure
 *
 * This function queries the kernel about the interface
 * index related to the interface name.
 */

int ifindex_to_devname(const int index, char *dev, size_t dev_len)
{
	const char alldev[] = ANY_INTERFACE;
	struct ifreq ifr;
	int rc;

	assert(index >= 0);
	assert(dev);
	assert(dev_len >= IFNAMSIZ);

	if (index == 0) {
		strlcpy(dev, alldev, dev_len);
		return (0);
	}

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_ifindex = index;

	rc = dev_kernel_request(&ifr, SIOCGIFNAME);

	if (!rc)
		strlcpy(dev, ifr.ifr_name, dev_len);

	return rc;
}

/**
 * \brief Get the interface status flags
 * \param[in]       dev	        interface name
 * \param[out]      flags	current interface status flags
 * \return 0 on success, -1 if the interface status flags could not be fetched.
 */

int dev_flags_get(const char *const dev, short *flags)
{
	int rc;
	struct ifreq ifr;

	assert(dev);
	assert(flags);

	memset(&ifr, 0, sizeof(ifr));
	strlcpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));

	rc = dev_kernel_request(&ifr, SIOCGIFFLAGS);

	if (!rc)
		*flags = ifr.ifr_flags;

	return rc;
}

/**
 * \brief Set the interface status flags
 * \param[in]       dev	        interface name
 * \param[un]       flags	new interface status flags
 * \return 0 on success, else if the interface status flags could not be changed.
 */

int dev_flags_set(const char *const dev, const short flags)
{
	struct ifreq ifr;

	assert(dev);

	memset(&ifr, 0, sizeof(ifr));
	strlcpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));
	ifr.ifr_flags = flags;

	return dev_kernel_request(&ifr, SIOCSIFFLAGS);
}

static int dev_ethtool_value_get(const char *const dev, const int cmd,
				 uint32_t * value)
{
	int rc;
	struct ethtool_value e;
	struct ifreq ifr;

	assert(dev);
	assert(value);

	memset(&e, 0, sizeof(e));
	memset(&ifr, 0, sizeof(ifr));

	strlcpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));
	e.cmd = cmd;
	ifr.ifr_data = (caddr_t) & e;

	rc = dev_kernel_request(&ifr, SIOCETHTOOL);

	if (!rc)
		*value = e.data;

	return rc;
}

/**
 * \brief Get the interface driver information
 * \param[in]       dev	        interface name
 * \param[out]      driver	pointer to the driver info
 * \return 0 on success, -1 if the interface driver information could not be fetched.
 */

int dev_driver_get(const char *const dev, struct ethtool_drvinfo *driver_info)
{
	struct ifreq ifr;

	assert(dev);
	assert(driver_info);

	memset(&ifr, 0, sizeof(ifr));
	memset(driver_info, 0, sizeof(*driver_info));
	strlcpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));
	driver_info->cmd = ETHTOOL_GDRVINFO;
	ifr.ifr_data = (caddr_t) driver_info;

	return dev_kernel_request(&ifr, SIOCETHTOOL);
}

/**
 * \brief Get the interface hardware settings
 * \param[in]       dev	        interface name
 * \param[out]      driver	pointer to the interface hardware settings
 * \return 0 on success, -1 if the interface hardware settings could not be fetched.
 */

int dev_settings_get(const char *const dev, struct ethtool_cmd *settings)
{
	struct ifreq ifr;

	assert(dev);
	assert(settings);

	memset(&ifr, 0, sizeof(ifr));
	memset(settings, 0, sizeof(*settings));
	strlcpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));
	settings->cmd = ETHTOOL_GSET;
	ifr.ifr_data = (caddr_t) settings;

	return dev_kernel_request(&ifr, SIOCETHTOOL);
}

/**
 * \brief Get the interface pause settings
 * \param[in]       dev	        interface name
 * \param[out]      driver	pointer to the interface pause settings
 * \return 0 on success, -1 if the interface pause settings could not be fetched.
 */

int dev_pause_get(const char *const dev, struct ethtool_pauseparam *pause)
{
	struct ifreq ifr;

	assert(dev);
	assert(pause);

	memset(&ifr, 0, sizeof(ifr));
	memset(pause, 0, sizeof(*pause));
	strlcpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));
	pause->cmd = ETHTOOL_GPAUSEPARAM;
	ifr.ifr_data = (caddr_t) pause;

	return dev_kernel_request(&ifr, SIOCETHTOOL);
}

/**
 * \brief Get the interface coalesce settings
 * \param[in]       dev	        interface name
 * \param[out]      coalesce	pointer to the interface coalesce settings
 * \return 0 on success, -1 if the interface pause settings could not be fetched.
 */

int dev_coalesce_get(const char *const dev, struct ethtool_coalesce *coalesce)
{
	struct ifreq ifr;

	assert(dev);
	assert(coalesce);

	memset(&ifr, 0, sizeof(ifr));
	memset(coalesce, 0, sizeof(*coalesce));
	strlcpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));
	coalesce->cmd = ETHTOOL_GCOALESCE;
	ifr.ifr_data = (caddr_t) coalesce;

	return dev_kernel_request(&ifr, SIOCETHTOOL);
}

int dev_rx_csum_offload_get(const char *const dev, uint32_t * rx_csum)
{
	return dev_ethtool_value_get(dev, ETHTOOL_GRXCSUM, rx_csum);
}

int dev_tx_csum_offload_get(const char *const dev, uint32_t * tx_csum)
{
	return dev_ethtool_value_get(dev, ETHTOOL_GTXCSUM, tx_csum);
}

int dev_scatter_gather_get(const char *const dev, uint32_t * sg)
{
	return dev_ethtool_value_get(dev, ETHTOOL_GSG, sg);
}

int dev_tcp_seg_offload_get(const char *const dev, uint32_t * tso)
{
	return dev_ethtool_value_get(dev, ETHTOOL_GTSO, tso);
}

int dev_udp_frag_offload_get(const char *const dev, uint32_t * ufo)
{
	return dev_ethtool_value_get(dev, ETHTOOL_GUFO, ufo);
}

int dev_generic_seg_offload_get(const char *const dev, uint32_t * gso)
{
	return dev_ethtool_value_get(dev, ETHTOOL_GGSO, gso);
}

int dev_generic_rcv_offload_get(const char *const dev, uint32_t * gro)
{
	return dev_ethtool_value_get(dev, ETHTOOL_GGRO, gro);
}

int dev_large_rcv_offload_get(const char *const dev, uint32_t * lro)
{
	int rc;
	uint32_t flags = 0;

	rc = dev_ethtool_value_get(dev, ETHTOOL_GFLAGS, &flags);
	*lro = flags & ETH_FLAG_LRO;

	return rc;
}

int dev_rx_hash_offload_get(const char *const dev, uint32_t * rxhash)
{
	int rc;
	uint32_t flags = 0;

	rc = dev_ethtool_value_get(dev, ETHTOOL_GFLAGS, &flags);
	*rxhash = flags & ETH_FLAG_RXHASH;

	return rc;
}

/**
 * \brief Get the interface link status
 * \param[in]       dev	        interface name
 * \param[out]      link	pointer to the interface link status
 * \return 0 on success, -1 if the interface link status could not be fetched.
 */

int dev_link_get(const char *const dev, uint32_t * link)
{
	return dev_ethtool_value_get(dev, ETHTOOL_GLINK, link);
}
