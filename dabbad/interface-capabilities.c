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

	if (!capabilitiesp->id) {
		free(capabilitiesp);
		return;
	}

	dabba__interface_id__init(capabilitiesp->id);
	capabilitiesp->id->name = rtnl_link_get_name(link);
	dev_settings_get(capabilitiesp->id->name, &capabilities);

	capabilitiesp->has_aui = capabilitiesp->has_bnc =
	    capabilitiesp->has_fibre = capabilitiesp->has_mii =
	    capabilitiesp->has_tp = 1;

	capabilitiesp->aui = ((capabilities.supported & SUPPORTED_AUI));
	capabilitiesp->bnc = ((capabilities.supported & SUPPORTED_BNC));
	capabilitiesp->fibre = ((capabilities.supported & SUPPORTED_FIBRE));
	capabilitiesp->mii = ((capabilities.supported & SUPPORTED_MII));
	capabilitiesp->tp = ((capabilities.supported & SUPPORTED_TP));

	capabilities_list->n_list++;
}

void dabbad_interface_capabilities_get(Dabba__DabbaService_Service * service,
				       const Dabba__InterfaceIdList * id_list,
				       Dabba__InterfaceCapabilitiesList_Closure
				       closure, void *closure_data)
{
	Dabba__InterfaceCapabilitiesList pause_list =
	    DABBA__INTERFACE_PAUSE_LIST__INIT;
	Dabba__InterfaceCapabilitiesList *pause_listp = NULL;
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
						&pause_list);
		}
	} else
		nl_cache_foreach(cache, __interface_capabilities_get,
				 &pause_list);

	pause_listp = &pause_list;

 out:
	closure(pause_listp, closure_data);
	for (a = 0; a < pause_list.n_list; a++) {
		free(pause_list.list[a]->id);
		free(pause_list.list[a]);
	}
	free(pause_list.list);
	nl_object_free(OBJ_CAST(link));
	link_cache_destroy(sock, cache);
}
