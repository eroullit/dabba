/**
 * \file interface-coalesce.c
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
#include <dabbad/interface-coalesce.h>

static void __interface_coalesce_get(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = (struct rtnl_link *)obj;
	Dabba__InterfaceCoalesceList *coalesce_list = arg;
	Dabba__InterfaceCoalesce *coalescep, **listpp;
	size_t lsize =
	    sizeof(*coalesce_list->list) * (coalesce_list->n_list + 1);
	struct ethtool_coalesce coalesce;

	listpp = realloc(coalesce_list->list, lsize);

	if (!listpp)
		return;

	coalesce_list->list = listpp;
	coalesce_list->list[coalesce_list->n_list] =
	    malloc(sizeof(*coalesce_list->list[coalesce_list->n_list]));
	coalescep = coalesce_list->list[coalesce_list->n_list];

	if (!coalescep)
		return;

	dabba__interface_coalesce__init(coalescep);

	coalescep->id = malloc(sizeof(*coalescep));

	if (!coalescep->id) {
		free(coalescep);
		return;
	}

	dabba__interface_id__init(coalescep->id);
	coalescep->id->name = rtnl_link_get_name(link);
	dev_coalesce_get(coalescep->id->name, &coalesce);

	coalescep->has_pkt_rate_high = coalescep->has_pkt_rate_low =
	    coalescep->has_rate_sample_interval =
	    coalescep->has_stats_block_coalesce_usecs =
	    coalescep->has_use_adaptive_rx_coalesce =
	    coalescep->has_rx_coalesce_usecs =
	    coalescep->has_rx_coalesce_usecs_irq =
	    coalescep->has_rx_coalesce_usecs_high =
	    coalescep->has_rx_coalesce_usecs_low =
	    coalescep->has_rx_max_coalesced_frames =
	    coalescep->has_rx_max_coalesced_frames_irq =
	    coalescep->has_rx_max_coalesced_frames_high =
	    coalescep->has_rx_max_coalesced_frames_low =
	    coalescep->has_use_adaptive_tx_coalesce =
	    coalescep->has_tx_coalesce_usecs =
	    coalescep->has_tx_coalesce_usecs_irq =
	    coalescep->has_tx_coalesce_usecs_high =
	    coalescep->has_tx_coalesce_usecs_low =
	    coalescep->has_tx_max_coalesced_frames =
	    coalescep->has_tx_max_coalesced_frames_irq =
	    coalescep->has_tx_max_coalesced_frames_high =
	    coalescep->has_tx_max_coalesced_frames_low = 1;

	coalescep->pkt_rate_high = coalesce.pkt_rate_high;
	coalescep->pkt_rate_low = coalesce.pkt_rate_low;
	coalescep->rate_sample_interval = coalesce.rate_sample_interval;
	coalescep->stats_block_coalesce_usecs =
	    coalesce.stats_block_coalesce_usecs;
	coalescep->use_adaptive_rx_coalesce = coalesce.use_adaptive_rx_coalesce;
	coalescep->rx_coalesce_usecs = coalesce.rx_coalesce_usecs;
	coalescep->rx_coalesce_usecs_irq = coalesce.rx_coalesce_usecs_irq;
	coalescep->rx_coalesce_usecs_high = coalesce.rx_coalesce_usecs_high;
	coalescep->rx_coalesce_usecs_low = coalesce.rx_coalesce_usecs_low;
	coalescep->rx_max_coalesced_frames = coalesce.rx_max_coalesced_frames;
	coalescep->rx_max_coalesced_frames_irq =
	    coalesce.rx_max_coalesced_frames;
	coalescep->rx_max_coalesced_frames_high =
	    coalesce.rx_max_coalesced_frames;
	coalescep->rx_max_coalesced_frames_low =
	    coalesce.rx_max_coalesced_frames;
	coalescep->use_adaptive_tx_coalesce = coalesce.use_adaptive_tx_coalesce;
	coalescep->tx_coalesce_usecs = coalesce.tx_coalesce_usecs;
	coalescep->tx_coalesce_usecs_irq = coalesce.tx_coalesce_usecs_irq;
	coalescep->tx_coalesce_usecs_high = coalesce.tx_coalesce_usecs_high;
	coalescep->tx_coalesce_usecs_low = coalesce.tx_coalesce_usecs_low;
	coalescep->tx_max_coalesced_frames = coalesce.tx_max_coalesced_frames;
	coalescep->tx_max_coalesced_frames_irq =
	    coalesce.tx_max_coalesced_frames;
	coalescep->tx_max_coalesced_frames_high =
	    coalesce.tx_max_coalesced_frames;
	coalescep->tx_max_coalesced_frames_low =
	    coalesce.tx_max_coalesced_frames;

	coalesce_list->n_list++;
}

void dabbad_interface_coalesce_get(Dabba__DabbaService_Service * service,
				   const Dabba__InterfaceIdList * id_list,
				   Dabba__InterfaceCoalesceList_Closure
				   closure, void *closure_data)
{
	Dabba__InterfaceCoalesceList coalesce_list =
	    DABBA__INTERFACE_PAUSE_LIST__INIT;
	Dabba__InterfaceCoalesceList *coalesce_listp = NULL;
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
						__interface_coalesce_get,
						&coalesce_list);
		}
	} else
		nl_cache_foreach(cache, __interface_coalesce_get,
				 &coalesce_list);

	coalesce_listp = &coalesce_list;

 out:
	closure(coalesce_listp, closure_data);
	for (a = 0; a < coalesce_list.n_list; a++) {
		free(coalesce_list.list[a]->id);
		free(coalesce_list.list[a]);
	}
	free(coalesce_list.list);
	nl_object_free(OBJ_CAST(link));
	link_cache_destroy(sock, cache);
}
