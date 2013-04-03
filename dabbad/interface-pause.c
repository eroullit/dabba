/**
 * \file interface-pause.c
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
#include <linux/ethtool.h>
#include <netlink/cache.h>
#include <netlink/route/link.h>
#include <libdabba/interface.h>
#include <dabbad/interface.h>
#include <dabbad/interface-pause.h>

static void __interface_pause_get(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = (struct rtnl_link *)obj;
	Dabba__InterfacePauseList *pause_list = arg;
	Dabba__InterfacePause *pausep, **listpp;
	size_t lsize = sizeof(*pause_list->list) * (pause_list->n_list + 1);
	struct ethtool_pauseparam pause;

	listpp = realloc(pause_list->list, lsize);

	if (!listpp)
		return;

	pause_list->list = listpp;
	pause_list->list[pause_list->n_list] =
	    malloc(sizeof(*pause_list->list[pause_list->n_list]));
	pausep = pause_list->list[pause_list->n_list];

	if (!pausep)
		return;

	dabba__interface_pause__init(pausep);

	pausep->id = malloc(sizeof(*pausep->id));
	pausep->status = malloc(sizeof(*pausep->status));

	if (!pausep->id || !pausep->status) {
		free(pausep->id);
		free(pausep->status);
		free(pausep);
		return;
	}

	dabba__interface_id__init(pausep->id);
	dabba__error_code__init(pausep->status);

	pausep->id->name = rtnl_link_get_name(link);
	pausep->status->code = dev_pause_get(pausep->id->name, &pause);

	pausep->has_autoneg = pausep->has_rx_pause = pausep->has_tx_pause = 1;
	pausep->autoneg = pause.autoneg;
	pausep->rx_pause = pause.rx_pause;
	pausep->tx_pause = pause.tx_pause;

	pause_list->n_list++;
}

void dabbad_interface_pause_get(Dabba__DabbaService_Service * service,
				const Dabba__InterfaceIdList * id_list,
				Dabba__InterfacePauseList_Closure
				closure, void *closure_data)
{
	Dabba__InterfacePauseList pause_list =
	    DABBA__INTERFACE_PAUSE_LIST__INIT;
	Dabba__InterfacePauseList *pause_listp = NULL;
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
						__interface_pause_get,
						&pause_list);
		}
	} else
		nl_cache_foreach(cache, __interface_pause_get, &pause_list);

	pause_listp = &pause_list;

 out:
	closure(pause_listp, closure_data);
	for (a = 0; a < pause_list.n_list; a++) {
		free(pause_list.list[a]->status);
		free(pause_list.list[a]->id);
		free(pause_list.list[a]);
	}
	free(pause_list.list);
	link_destroy(link);
	link_cache_destroy(sock, cache);
}

void dabbad_interface_pause_modify(Dabba__DabbaService_Service * service,
				   const Dabba__InterfacePause * pause,
				   Dabba__ErrorCode_Closure
				   closure, void *closure_data)
{
	struct ethtool_pauseparam p;
	int rc;

	assert(service);
	assert(closure_data);

	rc = dev_pause_get(pause->id->name, &p);

	if (rc)
		goto out;

	if (pause->has_rx_pause)
		p.rx_pause = pause->rx_pause;

	if (pause->has_tx_pause)
		p.rx_pause = pause->tx_pause;

	if (pause->has_autoneg)
		p.rx_pause = pause->has_autoneg;

	rc = dev_pause_set(pause->id->name, &p);

 out:
	pause->status->code = rc;
	closure(pause->status, closure_data);
}
