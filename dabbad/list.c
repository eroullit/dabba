
/* __LICENSE_HEADER_BEGIN__ */

/*
 * Copyright (C) 2009-2012	Emmanuel Roullit <emmanuel.roullit@gmail.com>
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
#include <net/if.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <dabbacore/macros.h>
#include <dabbacore/strlcpy.h>
#include <dabbad/list.h>

int dabbad_ifconf_get(struct dabba_ipc_msg *msg)
{
	size_t a = 0;
	struct ifaddrs *ifaddr, *ifa;

	if (getifaddrs(&ifaddr) != 0)
		return -1;

	for (ifa = ifaddr;
	     ifa != NULL && a < ARRAY_SIZE(msg->msg_body.msg.ifconf);
	     ifa = ifa->ifa_next) {
		if (ifa->ifa_addr->sa_family != AF_PACKET)
			continue;

		strlcpy(msg->msg_body.msg.ifconf[a].name, ifa->ifa_name,
			IFNAMSIZ);
		a++;
	}

	msg->msg_body.elem_nr = a;

	freeifaddrs(ifaddr);

	return 0;
}
