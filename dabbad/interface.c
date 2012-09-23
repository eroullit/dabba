/**
 * \file interface.c
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

static void interface_stats_copy(struct dabba_interface_list *iflist,
				 struct rtnl_link_stats *stats)
{
	assert(iflist);
	assert(stats);

	iflist->rx.byte = stats->rx_bytes;
	iflist->rx.packet = stats->rx_packets;
	iflist->rx.error = stats->rx_errors;
	iflist->rx.dropped = stats->rx_dropped;
	iflist->rx.compressed = stats->rx_compressed;

	iflist->rx_error.fifo = stats->rx_fifo_errors;
	iflist->rx_error.frame = stats->rx_frame_errors;
	iflist->rx_error.crc = stats->rx_crc_errors;
	iflist->rx_error.length = stats->rx_length_errors;
	iflist->rx_error.missed = stats->rx_missed_errors;
	iflist->rx_error.over = stats->rx_over_errors;

	iflist->tx.byte = stats->tx_bytes;
	iflist->tx.packet = stats->tx_packets;
	iflist->tx.error = stats->tx_errors;
	iflist->tx.dropped = stats->tx_dropped;
	iflist->tx.compressed = stats->tx_compressed;

	iflist->tx_error.fifo = stats->tx_fifo_errors;
	iflist->tx_error.carrier = stats->tx_carrier_errors;
	iflist->tx_error.heartbeat = stats->tx_heartbeat_errors;
	iflist->tx_error.window = stats->tx_window_errors;
	iflist->tx_error.aborted = stats->tx_aborted_errors;
}

/**
 * \brief Get the list of usable interfaces by dabbad and give it to the user
 * \param[in,out]       msg	        Dabba daemon IPC message
 * \return 0 on success, -1 if the interface list could not be fetched.
 *
 * This function only retrieves interfaces which belong to the packet family.
 */

int dabbad_interface_list_get(struct dabba_ipc_msg *msg)
{
	size_t a, off, ifconf_size;
	struct ifaddrs *ifaddr, *ifa;
	struct dabba_interface_list *ifconf;
	struct rtnl_link_stats *stats;

	if (getifaddrs(&ifaddr) != 0)
		return -1;

	ifconf_size = ARRAY_SIZE(msg->msg_body.msg.interface_list);
	ifa = ifaddr;

	for (off = 0; ifa && off < msg->msg_body.offset; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_PACKET)
			continue;
		off++;
	}

	for (a = 0; ifa && a < ifconf_size; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_PACKET)
			continue;

		ifconf = &msg->msg_body.msg.interface_list[a];
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
			interface_stats_copy(ifconf, stats);

		a++;
	}

	msg->msg_body.elem_nr = a;
	freeifaddrs(ifaddr);
	return 0;
}

int dabbad_interface_driver_get(struct dabba_ipc_msg *msg)
{
	size_t a, off, ifdrv_size;
	struct ifaddrs *ifaddr, *ifa;
	struct dabba_interface_driver *ifdrv;

	if (getifaddrs(&ifaddr) != 0)
		return -1;

	ifdrv_size = ARRAY_SIZE(msg->msg_body.msg.interface_settings);
	ifa = ifaddr;

	for (off = 0; ifa && off < msg->msg_body.offset; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_PACKET)
			continue;
		off++;
	}

	for (a = 0; ifa && a < ifdrv_size; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_PACKET)
			continue;

		ifdrv = &msg->msg_body.msg.interface_driver[a];
		strlcpy(ifdrv->name, ifa->ifa_name, IFNAMSIZ);
		dev_driver_get(ifdrv->name, &ifdrv->driver_info);

		a++;
	}

	msg->msg_body.elem_nr = a;
	freeifaddrs(ifaddr);
	return 0;
}

int dabbad_interface_settings_get(struct dabba_ipc_msg *msg)
{
	size_t a, off, ifsettings_size;
	struct ifaddrs *ifaddr, *ifa;
	struct dabba_interface_settings *ifsettings;

	if (getifaddrs(&ifaddr) != 0)
		return -1;

	ifsettings_size = ARRAY_SIZE(msg->msg_body.msg.interface_settings);
	ifa = ifaddr;

	for (off = 0; ifa && off < msg->msg_body.offset; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_PACKET)
			continue;
		off++;
	}

	for (a = 0; ifa && a < ifsettings_size; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_PACKET)
			continue;

		ifsettings = &msg->msg_body.msg.interface_settings[a];
		strlcpy(ifsettings->name, ifa->ifa_name, IFNAMSIZ);
		dev_settings_get(ifsettings->name, &ifsettings->settings);

		a++;
	}

	msg->msg_body.elem_nr = a;
	freeifaddrs(ifaddr);
	return 0;
}

int dabbad_interface_pause_get(struct dabba_ipc_msg *msg)
{
	size_t a, off, ifpause_size;
	struct ifaddrs *ifaddr, *ifa;
	struct dabba_interface_pause *ifpause;

	if (getifaddrs(&ifaddr) != 0)
		return -1;

	ifpause_size = ARRAY_SIZE(msg->msg_body.msg.interface_pause);
	ifa = ifaddr;

	for (off = 0; ifa && off < msg->msg_body.offset; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_PACKET)
			continue;
		off++;
	}

	for (a = 0; ifa && a < ifpause_size; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_PACKET)
			continue;

		ifpause = &msg->msg_body.msg.interface_pause[a];
		strlcpy(ifpause->name, ifa->ifa_name, IFNAMSIZ);
		dev_pause_get(ifpause->name, &ifpause->pause);

		a++;
	}

	msg->msg_body.elem_nr = a;
	freeifaddrs(ifaddr);
	return 0;
}

int dabbad_interface_coalesce_get(struct dabba_ipc_msg *msg)
{
	size_t a, off, ifcoalesce_size;
	struct ifaddrs *ifaddr, *ifa;
	struct dabba_interface_coalesce *ifcoalesce;

	if (getifaddrs(&ifaddr) != 0)
		return -1;

	ifcoalesce_size = ARRAY_SIZE(msg->msg_body.msg.interface_coalesce);
	ifa = ifaddr;

	for (off = 0; ifa && off < msg->msg_body.offset; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_PACKET)
			continue;
		off++;
	}

	for (a = 0; ifa && a < ifcoalesce_size; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_PACKET)
			continue;

		ifcoalesce = &msg->msg_body.msg.interface_coalesce[a];
		strlcpy(ifcoalesce->name, ifa->ifa_name, IFNAMSIZ);
		dev_coalesce_get(ifcoalesce->name, &ifcoalesce->coalesce);

		a++;
	}

	msg->msg_body.elem_nr = a;
	freeifaddrs(ifaddr);
	return 0;
}

/**
 * \brief Modify a supported interface status
 * \param[in,out]       msg	        Dabba daemon IPC message
 * \return 0 on success, else if the interface status could not be modified.
 */

int dabbad_interface_modify(struct dabba_ipc_msg *msg)
{
	int rc;
	short flags;

	rc = dev_flags_get(msg->msg_body.msg.interface_list[0].name, &flags);

	if (rc)
		return rc;

	if (msg->msg_body.msg.interface_list[0].up == TRUE)
		flags |= IFF_UP;
	else if (msg->msg_body.msg.interface_list[0].up == FALSE)
		flags &= ~IFF_UP;

	if (msg->msg_body.msg.interface_list[0].running == TRUE)
		flags |= IFF_RUNNING;
	else if (msg->msg_body.msg.interface_list[0].running == FALSE)
		flags &= ~IFF_RUNNING;

	if (msg->msg_body.msg.interface_list[0].promisc == TRUE)
		flags |= IFF_PROMISC;
	else if (msg->msg_body.msg.interface_list[0].promisc == FALSE)
		flags &= ~IFF_PROMISC;

	return dev_flags_set(msg->msg_body.msg.interface_list[0].name, flags);
}
