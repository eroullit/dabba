/**
 * \file interface-capabilities.c
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
#include <dabbad/interface-capabilities.h>

static void __interface_capabilities_get(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = (struct rtnl_link *)obj;
	Dabba__InterfaceCapabilitiesList *capabilities_list = arg;
	Dabba__InterfaceCapabilities *capabilitiesp, **listpp;
	size_t lsize =
	    sizeof(*capabilities_list->list) * (capabilities_list->n_list + 1);
	struct ethtool_cmd capabilities;

	listpp = realloc(capabilities_list->list, lsize);

	if (!listpp)
		return;

	capabilities_list->list = listpp;
	capabilities_list->list[capabilities_list->n_list] =
	    malloc(sizeof(*capabilities_list->list[capabilities_list->n_list]));
	capabilitiesp = capabilities_list->list[capabilities_list->n_list];

	if (!capabilitiesp)
		return;

	dabba__interface_capabilities__init(capabilitiesp);

	capabilitiesp->id = malloc(sizeof(*capabilitiesp));
	capabilitiesp->supported_opt =
	    malloc(sizeof(*capabilitiesp->supported_opt));
	capabilitiesp->supported_speed =
	    malloc(sizeof(*capabilitiesp->supported_speed));
	capabilitiesp->advertising_opt =
	    malloc(sizeof(*capabilitiesp->advertising_opt));
	capabilitiesp->advertising_speed =
	    malloc(sizeof(*capabilitiesp->advertising_speed));
	capabilitiesp->lp_advertising_opt =
	    malloc(sizeof(*capabilitiesp->lp_advertising_opt));
	capabilitiesp->lp_advertising_speed =
	    malloc(sizeof(*capabilitiesp->lp_advertising_speed));

	if (!capabilitiesp->id || !capabilitiesp->supported_opt
	    || !capabilitiesp->supported_speed
	    || !capabilitiesp->advertising_opt
	    || !capabilitiesp->advertising_speed
	    || !capabilitiesp->lp_advertising_opt
	    || !capabilitiesp->lp_advertising_speed) {
		free(capabilitiesp->id);
		free(capabilitiesp->supported_opt);
		free(capabilitiesp->supported_speed);
		free(capabilitiesp->advertising_opt);
		free(capabilitiesp->advertising_speed);
		free(capabilitiesp->lp_advertising_opt);
		free(capabilitiesp->lp_advertising_speed);
		free(capabilitiesp);
		return;
	}

	dabba__interface_id__init(capabilitiesp->id);
	dabba__interface_speed_capabilites__init
	    (capabilitiesp->supported_speed);
	dabba__interface_option_capabilites__init(capabilitiesp->supported_opt);
	dabba__interface_speed_capabilites__init
	    (capabilitiesp->advertising_speed);
	dabba__interface_option_capabilites__init
	    (capabilitiesp->advertising_opt);
	dabba__interface_speed_capabilites__init
	    (capabilitiesp->lp_advertising_speed);
	dabba__interface_option_capabilites__init
	    (capabilitiesp->lp_advertising_opt);

	capabilitiesp->supported_speed->ethernet =
	    malloc(sizeof(*capabilitiesp->supported_speed->ethernet));
	capabilitiesp->supported_speed->fast_ethernet =
	    malloc(sizeof(*capabilitiesp->supported_speed->fast_ethernet));
	capabilitiesp->supported_speed->gbps_ethernet =
	    malloc(sizeof(*capabilitiesp->supported_speed->gbps_ethernet));
	capabilitiesp->supported_speed->_10gbps_ethernet =
	    malloc(sizeof(*capabilitiesp->supported_speed->_10gbps_ethernet));
	capabilitiesp->advertising_speed->ethernet =
	    malloc(sizeof(*capabilitiesp->advertising_speed->ethernet));
	capabilitiesp->advertising_speed->fast_ethernet =
	    malloc(sizeof(*capabilitiesp->advertising_speed->fast_ethernet));
	capabilitiesp->advertising_speed->gbps_ethernet =
	    malloc(sizeof(*capabilitiesp->advertising_speed->gbps_ethernet));
	capabilitiesp->advertising_speed->_10gbps_ethernet =
	    malloc(sizeof(*capabilitiesp->advertising_speed->_10gbps_ethernet));
	capabilitiesp->lp_advertising_speed->ethernet =
	    malloc(sizeof(*capabilitiesp->lp_advertising_speed->ethernet));
	capabilitiesp->lp_advertising_speed->fast_ethernet =
	    malloc(sizeof(*capabilitiesp->lp_advertising_speed->fast_ethernet));
	capabilitiesp->lp_advertising_speed->gbps_ethernet =
	    malloc(sizeof(*capabilitiesp->lp_advertising_speed->gbps_ethernet));
	capabilitiesp->lp_advertising_speed->_10gbps_ethernet =
	    malloc(sizeof
		   (*capabilitiesp->lp_advertising_speed->_10gbps_ethernet));

	if (!capabilitiesp->supported_speed->ethernet
	    || !capabilitiesp->supported_speed->fast_ethernet
	    || !capabilitiesp->supported_speed->gbps_ethernet
	    || !capabilitiesp->supported_speed->_10gbps_ethernet
	    || !capabilitiesp->advertising_speed->ethernet
	    || !capabilitiesp->advertising_speed->fast_ethernet
	    || !capabilitiesp->advertising_speed->gbps_ethernet
	    || !capabilitiesp->advertising_speed->_10gbps_ethernet
	    || !capabilitiesp->lp_advertising_speed->ethernet
	    || !capabilitiesp->lp_advertising_speed->fast_ethernet
	    || !capabilitiesp->lp_advertising_speed->gbps_ethernet
	    || !capabilitiesp->lp_advertising_speed->_10gbps_ethernet) {
		free(capabilitiesp->supported_speed->ethernet);
		free(capabilitiesp->supported_speed->fast_ethernet);
		free(capabilitiesp->supported_speed->gbps_ethernet);
		free(capabilitiesp->supported_speed->_10gbps_ethernet);
		free(capabilitiesp->advertising_speed->ethernet);
		free(capabilitiesp->advertising_speed->fast_ethernet);
		free(capabilitiesp->advertising_speed->gbps_ethernet);
		free(capabilitiesp->advertising_speed->_10gbps_ethernet);
		free(capabilitiesp->lp_advertising_speed->ethernet);
		free(capabilitiesp->lp_advertising_speed->fast_ethernet);
		free(capabilitiesp->lp_advertising_speed->gbps_ethernet);
		free(capabilitiesp->lp_advertising_speed->_10gbps_ethernet);
		free(capabilitiesp->id);
		free(capabilitiesp->supported_opt);
		free(capabilitiesp->supported_speed);
		free(capabilitiesp->advertising_opt);
		free(capabilitiesp->advertising_speed);
		free(capabilitiesp->lp_advertising_opt);
		free(capabilitiesp->lp_advertising_speed);
		free(capabilitiesp);
	}

	dabba__interface_duplex_capabilites__init
	    (capabilitiesp->supported_speed->ethernet);
	dabba__interface_duplex_capabilites__init
	    (capabilitiesp->supported_speed->fast_ethernet);
	dabba__interface_duplex_capabilites__init
	    (capabilitiesp->supported_speed->gbps_ethernet);
	dabba__interface_duplex_capabilites__init
	    (capabilitiesp->supported_speed->_10gbps_ethernet);
	dabba__interface_duplex_capabilites__init
	    (capabilitiesp->advertising_speed->ethernet);
	dabba__interface_duplex_capabilites__init
	    (capabilitiesp->advertising_speed->fast_ethernet);
	dabba__interface_duplex_capabilites__init
	    (capabilitiesp->advertising_speed->gbps_ethernet);
	dabba__interface_duplex_capabilites__init
	    (capabilitiesp->advertising_speed->_10gbps_ethernet);
	dabba__interface_duplex_capabilites__init
	    (capabilitiesp->lp_advertising_speed->ethernet);
	dabba__interface_duplex_capabilites__init
	    (capabilitiesp->lp_advertising_speed->fast_ethernet);
	dabba__interface_duplex_capabilites__init
	    (capabilitiesp->lp_advertising_speed->gbps_ethernet);
	dabba__interface_duplex_capabilites__init
	    (capabilitiesp->lp_advertising_speed->_10gbps_ethernet);

	capabilitiesp->id->name = rtnl_link_get_name(link);
	dev_settings_get(capabilitiesp->id->name, &capabilities);

	capabilitiesp->has_aui = capabilitiesp->has_bnc =
	    capabilitiesp->has_fibre = capabilitiesp->has_mii =
	    capabilitiesp->has_tp = 1;

	capabilitiesp->supported_opt->has_autoneg =
	    capabilitiesp->supported_opt->has_pause =
	    capabilitiesp->advertising_opt->has_autoneg =
	    capabilitiesp->advertising_opt->has_pause =
	    capabilitiesp->lp_advertising_opt->has_autoneg =
	    capabilitiesp->lp_advertising_opt->has_pause = 1;
	capabilitiesp->supported_speed->ethernet->has_half =
	    capabilitiesp->supported_speed->ethernet->has_full =
	    capabilitiesp->supported_speed->fast_ethernet->has_half =
	    capabilitiesp->supported_speed->fast_ethernet->has_full =
	    capabilitiesp->supported_speed->gbps_ethernet->has_half =
	    capabilitiesp->supported_speed->gbps_ethernet->has_full =
	    capabilitiesp->supported_speed->_10gbps_ethernet->has_half =
	    capabilitiesp->supported_speed->_10gbps_ethernet->has_full =
	    capabilitiesp->advertising_speed->ethernet->has_half =
	    capabilitiesp->advertising_speed->ethernet->has_full =
	    capabilitiesp->advertising_speed->fast_ethernet->has_half =
	    capabilitiesp->advertising_speed->fast_ethernet->has_full =
	    capabilitiesp->advertising_speed->gbps_ethernet->has_half =
	    capabilitiesp->advertising_speed->gbps_ethernet->has_full =
	    capabilitiesp->advertising_speed->_10gbps_ethernet->has_half =
	    capabilitiesp->advertising_speed->_10gbps_ethernet->has_full =
	    capabilitiesp->lp_advertising_speed->ethernet->has_half =
	    capabilitiesp->lp_advertising_speed->ethernet->has_full =
	    capabilitiesp->lp_advertising_speed->fast_ethernet->has_half =
	    capabilitiesp->lp_advertising_speed->fast_ethernet->has_full =
	    capabilitiesp->lp_advertising_speed->gbps_ethernet->has_half =
	    capabilitiesp->lp_advertising_speed->gbps_ethernet->has_full =
	    capabilitiesp->lp_advertising_speed->_10gbps_ethernet->has_half =
	    capabilitiesp->lp_advertising_speed->_10gbps_ethernet->has_full = 1;

	capabilitiesp->aui = (capabilities.supported & SUPPORTED_AUI);
	capabilitiesp->bnc = (capabilities.supported & SUPPORTED_BNC);
	capabilitiesp->fibre = (capabilities.supported & SUPPORTED_FIBRE);
	capabilitiesp->mii = (capabilities.supported & SUPPORTED_MII);
	capabilitiesp->tp = (capabilities.supported & SUPPORTED_TP);

	/* There is no 10Gbps half duplex support */
	capabilitiesp->supported_speed->ethernet->half =
	    (capabilities.supported & SUPPORTED_10baseT_Half);
	capabilitiesp->supported_speed->ethernet->full =
	    (capabilities.supported & SUPPORTED_10baseT_Full);
	capabilitiesp->supported_speed->fast_ethernet->half =
	    (capabilities.supported & SUPPORTED_100baseT_Half);
	capabilitiesp->supported_speed->fast_ethernet->full =
	    (capabilities.supported & SUPPORTED_100baseT_Full);
	capabilitiesp->supported_speed->gbps_ethernet->half =
	    (capabilities.supported & SUPPORTED_1000baseT_Half);
	capabilitiesp->supported_speed->gbps_ethernet->full =
	    (capabilities.supported & SUPPORTED_1000baseT_Full);
	capabilitiesp->supported_speed->_10gbps_ethernet->half = 0;
	capabilitiesp->supported_speed->_10gbps_ethernet->full =
	    (capabilities.supported & SUPPORTED_10000baseT_Full);

	capabilitiesp->advertising_speed->ethernet->half =
	    (capabilities.advertising & ADVERTISED_10baseT_Half);
	capabilitiesp->advertising_speed->ethernet->full =
	    (capabilities.advertising & ADVERTISED_10baseT_Full);
	capabilitiesp->advertising_speed->fast_ethernet->half =
	    (capabilities.advertising & ADVERTISED_100baseT_Half);
	capabilitiesp->advertising_speed->fast_ethernet->full =
	    (capabilities.advertising & ADVERTISED_100baseT_Full);
	capabilitiesp->advertising_speed->gbps_ethernet->half =
	    (capabilities.advertising & ADVERTISED_1000baseT_Half);
	capabilitiesp->advertising_speed->gbps_ethernet->full =
	    (capabilities.advertising & ADVERTISED_1000baseT_Full);
	capabilitiesp->advertising_speed->_10gbps_ethernet->half = 0;
	capabilitiesp->advertising_speed->_10gbps_ethernet->full =
	    (capabilities.advertising & ADVERTISED_10000baseT_Full);

	capabilitiesp->lp_advertising_speed->ethernet->half =
	    (capabilities.lp_advertising & ADVERTISED_10baseT_Half);
	capabilitiesp->lp_advertising_speed->ethernet->full =
	    (capabilities.lp_advertising & ADVERTISED_10baseT_Full);
	capabilitiesp->lp_advertising_speed->fast_ethernet->half =
	    (capabilities.lp_advertising & ADVERTISED_100baseT_Half);
	capabilitiesp->lp_advertising_speed->fast_ethernet->full =
	    (capabilities.lp_advertising & ADVERTISED_100baseT_Full);
	capabilitiesp->lp_advertising_speed->gbps_ethernet->half =
	    (capabilities.lp_advertising & ADVERTISED_1000baseT_Half);
	capabilitiesp->lp_advertising_speed->gbps_ethernet->full =
	    (capabilities.lp_advertising & ADVERTISED_1000baseT_Full);
	capabilitiesp->lp_advertising_speed->_10gbps_ethernet->half = 0;
	capabilitiesp->lp_advertising_speed->_10gbps_ethernet->full =
	    (capabilities.lp_advertising & ADVERTISED_10000baseT_Full);

	capabilities_list->n_list++;
}

void dabbad_interface_capabilities_get(Dabba__DabbaService_Service * service,
				       const Dabba__InterfaceIdList * id_list,
				       Dabba__InterfaceCapabilitiesList_Closure
				       closure, void *closure_data)
{
	Dabba__InterfaceCapabilitiesList capabilities_list =
	    DABBA__INTERFACE_PAUSE_LIST__INIT;
	Dabba__InterfaceCapabilitiesList *capabilities_listp = NULL;
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
						__interface_capabilities_get,
						&capabilities_list);
		}
	} else
		nl_cache_foreach(cache, __interface_capabilities_get,
				 &capabilities_list);

	capabilities_listp = &capabilities_list;

 out:
	closure(capabilities_listp, closure_data);
	for (a = 0; a < capabilities_list.n_list; a++) {
		free(capabilities_list.list[a]->supported_speed->ethernet);
		free(capabilities_list.list[a]->supported_speed->fast_ethernet);
		free(capabilities_list.list[a]->supported_speed->gbps_ethernet);
		free(capabilities_list.list[a]->supported_speed->
		     _10gbps_ethernet);
		free(capabilities_list.list[a]->advertising_speed->ethernet);
		free(capabilities_list.list[a]->advertising_speed->
		     fast_ethernet);
		free(capabilities_list.list[a]->advertising_speed->
		     gbps_ethernet);
		free(capabilities_list.list[a]->advertising_speed->
		     _10gbps_ethernet);
		free(capabilities_list.list[a]->lp_advertising_speed->ethernet);
		free(capabilities_list.list[a]->lp_advertising_speed->
		     fast_ethernet);
		free(capabilities_list.list[a]->lp_advertising_speed->
		     gbps_ethernet);
		free(capabilities_list.list[a]->lp_advertising_speed->
		     _10gbps_ethernet);
		free(capabilities_list.list[a]->supported_opt);
		free(capabilities_list.list[a]->supported_speed);
		free(capabilities_list.list[a]->advertising_opt);
		free(capabilities_list.list[a]->advertising_speed);
		free(capabilities_list.list[a]->lp_advertising_opt);
		free(capabilities_list.list[a]->lp_advertising_speed);
		free(capabilities_list.list[a]->id);
		free(capabilities_list.list[a]);
	}
	free(capabilities_list.list);
	nl_object_free(OBJ_CAST(link));
	link_cache_destroy(sock, cache);
}
