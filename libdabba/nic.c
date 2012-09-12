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

#include <libdabba/nic.h>
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
