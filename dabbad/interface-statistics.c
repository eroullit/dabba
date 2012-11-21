/**
 * \file interface-statistics.c
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

/* HACK prevent libnl3 include clash between <net/if.h> and <linux/if.h> */
#ifndef _LINUX_IF_H
#define _LINUX_IF_H
#endif				/* _LINUX_IF_H */

#include <assert.h>
#include <errno.h>
#include <netlink/cache.h>
#include <netlink/route/link.h>
#include <dabbad/interface.h>
#include <dabbad/interface-statistics.h>

#include "libdabba-rpc/dabba.pb-c.h"

static void __interface_statistics_get(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = (struct rtnl_link *)obj;
	Dabba__InterfaceStatisticsList *statistics_list = arg;
	Dabba__InterfaceStatistics *statisticsp, **listpp;
	size_t lsize =
	    sizeof(*statistics_list->list) * (statistics_list->n_list + 1);

	listpp = realloc(statistics_list->list, lsize);

	if (!listpp)
		return;

	statistics_list->list = listpp;
	statistics_list->list[statistics_list->n_list] =
	    malloc(sizeof(*statistics_list->list[statistics_list->n_list]));
	statisticsp = statistics_list->list[statistics_list->n_list];

	if (!statisticsp)
		return;

	dabba__interface_statistics__init(statisticsp);

	statisticsp->id = malloc(sizeof(*statisticsp));

	if (!statisticsp->id) {
		free(statisticsp);
		return;
	}

	dabba__interface_id__init(statisticsp->id);

	statisticsp->id->name = rtnl_link_get_name(link);
	statisticsp->rx_byte = rtnl_link_get_stat(link, RTNL_LINK_RX_BYTES);
	statisticsp->rx_packet = rtnl_link_get_stat(link, RTNL_LINK_RX_PACKETS);
	statisticsp->rx_error = rtnl_link_get_stat(link, RTNL_LINK_RX_ERRORS);
	statisticsp->rx_dropped =
	    rtnl_link_get_stat(link, RTNL_LINK_RX_DROPPED);
	statisticsp->rx_compressed =
	    rtnl_link_get_stat(link, RTNL_LINK_RX_COMPRESSED);
	statisticsp->rx_error_fifo =
	    rtnl_link_get_stat(link, RTNL_LINK_RX_FIFO_ERR);
	statisticsp->rx_error_frame =
	    rtnl_link_get_stat(link, RTNL_LINK_RX_FRAME_ERR);
	statisticsp->rx_error_crc =
	    rtnl_link_get_stat(link, RTNL_LINK_RX_CRC_ERR);
	statisticsp->rx_error_length =
	    rtnl_link_get_stat(link, RTNL_LINK_RX_LEN_ERR);
	statisticsp->rx_error_missed =
	    rtnl_link_get_stat(link, RTNL_LINK_RX_MISSED_ERR);
	statisticsp->rx_error_over =
	    rtnl_link_get_stat(link, RTNL_LINK_RX_OVER_ERR);

	statisticsp->tx_byte = rtnl_link_get_stat(link, RTNL_LINK_TX_BYTES);
	statisticsp->tx_packet = rtnl_link_get_stat(link, RTNL_LINK_TX_PACKETS);
	statisticsp->tx_error = rtnl_link_get_stat(link, RTNL_LINK_TX_ERRORS);
	statisticsp->tx_dropped =
	    rtnl_link_get_stat(link, RTNL_LINK_TX_DROPPED);
	statisticsp->tx_compressed =
	    rtnl_link_get_stat(link, RTNL_LINK_TX_COMPRESSED);
	statisticsp->tx_error_fifo =
	    rtnl_link_get_stat(link, RTNL_LINK_TX_FIFO_ERR);
	statisticsp->tx_error_carrier =
	    rtnl_link_get_stat(link, RTNL_LINK_TX_CARRIER_ERR);
	statisticsp->tx_error_heartbeat =
	    rtnl_link_get_stat(link, RTNL_LINK_TX_HBEAT_ERR);
	statisticsp->tx_error_window =
	    rtnl_link_get_stat(link, RTNL_LINK_TX_WIN_ERR);
	statisticsp->tx_error_aborted =
	    rtnl_link_get_stat(link, RTNL_LINK_TX_ABORT_ERR);

	statistics_list->n_list++;
}

void dabbad_interface_statistics_get(Dabba__DabbaService_Service * service,
				     const Dabba__InterfaceIdList * id_list,
				     Dabba__InterfaceStatisticsList_Closure
				     closure, void *closure_data)
{
	Dabba__InterfaceStatisticsList statistics_list =
	    DABBA__INTERFACE_STATISTICS_LIST__INIT;
	Dabba__InterfaceStatisticsList *statistics_listp = NULL;
	struct nl_sock *sock = NULL;
	struct nl_cache *cache;
	struct rtnl_link *link;
	size_t a;

	assert(service);
	assert(closure_data);

	cache = link_cache_alloc(&sock);
	link = rtnl_link_alloc();

	if (!link || !cache)
		goto out;

	if (id_list->n_list) {
		for (a = 0; a < id_list->n_list; a++) {
			rtnl_link_set_name(link, id_list->list[a]->name);
			nl_cache_foreach_filter(cache, OBJ_CAST(link),
						__interface_statistics_get,
						&statistics_list);
		}
	} else
		nl_cache_foreach(cache, __interface_statistics_get,
				 &statistics_list);

	statistics_listp = &statistics_list;

 out:
	closure(statistics_listp, closure_data);
	for (a = 0; a < statistics_list.n_list; a++) {
		free(statistics_list.list[a]->id);
		free(statistics_list.list[a]);
	}
	free(statistics_list.list);
	nl_object_free(OBJ_CAST(link));
	link_cache_destroy(sock, cache);
}
