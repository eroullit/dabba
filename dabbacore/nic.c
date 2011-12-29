/**
 * \file nic.c
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

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <dabbacore/strlcpy.h>

/**
 * \brief Get the interface index of a specific device
 * \param[in]	dev	Device name
 * \param[out]	index	Interface index of the device
 * \return 0 on success, errno from socket(2), ioctl(2) or close(2) on failure
 *
 * This function queries the kernel about the interface
 * index related to the device name.
 */

int devname_to_ifindex(const char *const dev, int *index)
{
	int ret;
	int sock;
	struct ifreq ethreq;

	assert(dev);

	sock = socket(AF_INET, SOCK_DGRAM, 0);

	if (sock < 0)
		return (errno);

	memset(&ethreq, 0, sizeof(ethreq));
	strlcpy(ethreq.ifr_name, dev, sizeof(ethreq.ifr_name));

	ret = ioctl(sock, SIOCGIFINDEX, &ethreq);

	close(sock);

	if (ret < 0)
		return (errno);

	*index = ethreq.ifr_ifindex;

	return (0);
}
