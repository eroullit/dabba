/**
 * \file list.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2012
 * \date 2012
 */

/* __LICENSE_HEADER_BEGIN__ */

/*
 * Copyright (C) 2012	Emmanuel Roullit <emmanuel.roullit@gmail.com>
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
#include <stdio.h>
#include <net/if.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <linux/if_link.h>

#include <libdabba/macros.h>
#include <libdabba/strlcpy.h>
#include <libdabba/nic.h>
#include <dabbad/interface.h>
#include <netpacket/packet.h>

static void ifconf_stats_copy(struct dabba_ifconf *ifconf,
			      struct rtnl_link_stats *stats)
{
	assert(ifconf);
	assert(stats);

	ifconf->rx.byte = stats->rx_bytes;
	ifconf->rx.packet = stats->rx_packets;
	ifconf->rx.error = stats->rx_errors;
	ifconf->rx.dropped = stats->rx_dropped;

	ifconf->tx.byte = stats->tx_bytes;
	ifconf->tx.packet = stats->tx_packets;
	ifconf->tx.error = stats->tx_errors;
	ifconf->tx.dropped = stats->tx_dropped;
}

/**
 * \brief Get the list of usable interfaces by dabbad and give it to the user
 * \param[in,out]       msg	        Dabba daemon IPC message
 * \return 0 on success, -1 if the interface list could not be fetched.
 *
 * This function only retrieves interfaces which belong to the packet family.
 */

int dabbad_ifconf_get(struct dabba_ipc_msg *msg)
{
	size_t a, off, ifconf_size;
	struct ifaddrs *ifaddr, *ifa;
	struct dabba_ifconf *ifconf;
	struct rtnl_link_stats *stats;

	if (getifaddrs(&ifaddr) != 0)
		return -1;

	ifconf_size = ARRAY_SIZE(msg->msg_body.msg.ifconf);
	ifa = ifaddr;

	for (off = 0; ifa && off < msg->msg_body.offset; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_PACKET)
			continue;
		off++;
	}

	for (a = 0; ifa && a < ifconf_size; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_PACKET)
			continue;

		ifconf = &msg->msg_body.msg.ifconf[a];
		strlcpy(ifconf->name, ifa->ifa_name, IFNAMSIZ);

		ifconf->up = (ifa->ifa_flags & IFF_UP) == IFF_UP ? TRUE : FALSE;
		ifconf->running =
		    (ifa->ifa_flags & IFF_RUNNING) ==
		    IFF_RUNNING ? TRUE : FALSE;
		ifconf->promisc =
		    (ifa->ifa_flags & IFF_PROMISC) ==
		    IFF_PROMISC ? TRUE : FALSE;
		ifconf->loopback =
		    (ifa->ifa_flags & IFF_LOOPBACK) ==
		    IFF_LOOPBACK ? TRUE : FALSE;

		stats = ifa->ifa_data;

		/* FIXME 32-bit counters only... */
		if (stats)
			ifconf_stats_copy(ifconf, stats);

		a++;
	}

	msg->msg_body.elem_nr = a;
	freeifaddrs(ifaddr);
	return 0;
}

int dabbad_interface_modify(struct dabba_ipc_msg *msg)
{
	int rc;
	short flags;

	rc = dev_flags_get(msg->msg_body.msg.ifconf[0].name, &flags);

	if (rc)
		return rc;

	if (msg->msg_body.msg.ifconf[0].up == TRUE)
		flags |= IFF_UP;
	else if (msg->msg_body.msg.ifconf[0].up == FALSE)
		flags &= ~IFF_UP;

	if (msg->msg_body.msg.ifconf[0].running == TRUE)
		flags |= IFF_RUNNING;
	else if (msg->msg_body.msg.ifconf[0].running == FALSE)
		flags &= ~IFF_RUNNING;

	if (msg->msg_body.msg.ifconf[0].promisc == TRUE)
		flags |= IFF_PROMISC;
	else if (msg->msg_body.msg.ifconf[0].promisc == FALSE)
		flags &= ~IFF_PROMISC;

	return dev_flags_set(msg->msg_body.msg.ifconf[0].name, flags);
}
