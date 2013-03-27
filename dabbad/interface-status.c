/**
 * \file interface-status.c
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
#include <net/if.h>
#include <netlink/cache.h>
#include <netlink/route/link.h>
#include <libdabba/interface.h>
#include <dabbad/interface.h>
#include <dabbad/interface-status.h>

static void __interface_status_get(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = (struct rtnl_link *)obj;
	Dabba__InterfaceStatusList *status_list = arg;
	Dabba__InterfaceStatus *statusp, **listpp;
	size_t lsize = sizeof(*status_list->list) * (status_list->n_list + 1);
	uint16_t flags;

	listpp = realloc(status_list->list, lsize);

	if (!listpp)
		return;

	status_list->list = listpp;
	status_list->list[status_list->n_list] =
	    malloc(sizeof(*status_list->list[status_list->n_list]));
	statusp = status_list->list[status_list->n_list];

	if (!statusp)
		return;

	dabba__interface_status__init(statusp);

	statusp->id = malloc(sizeof(*statusp->id));

	if (!statusp->id) {
		free(statusp);
		return;
	}

	dabba__interface_id__init(statusp->id);

	statusp->has_connectivity = statusp->has_loopback = statusp->has_up = 1;
	statusp->has_promiscous = statusp->has_running = 1;

	statusp->id->name = rtnl_link_get_name(link);
	flags = rtnl_link_get_flags(link);

	dev_link_get(statusp->id->name, (uint32_t *) & statusp->connectivity);
	statusp->loopback = (flags & IFF_LOOPBACK) == IFF_LOOPBACK;
	statusp->up = (flags & IFF_UP) == IFF_UP;
	statusp->running = (flags & IFF_RUNNING) == IFF_RUNNING;
	statusp->promiscous = (flags & IFF_PROMISC) == IFF_PROMISC;

	status_list->n_list++;
}

void dabbad_interface_status_get(Dabba__DabbaService_Service * service,
				 const Dabba__InterfaceIdList * id_list,
				 Dabba__InterfaceStatusList_Closure
				 closure, void *closure_data)
{
	Dabba__InterfaceStatusList status_list =
	    DABBA__INTERFACE_STATUS_LIST__INIT;
	Dabba__InterfaceStatusList *status_listp = NULL;
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
						__interface_status_get,
						&status_list);
		}
	} else
		nl_cache_foreach(cache, __interface_status_get, &status_list);

	status_listp = &status_list;

 out:
	closure(status_listp, closure_data);
	for (a = 0; a < status_list.n_list; a++) {
		free(status_list.list[a]->id);
		free(status_list.list[a]);
	}
	free(status_list.list);
	nl_object_free(OBJ_CAST(link));
	link_cache_destroy(sock, cache);
}

void dabbad_interface_status_modify(Dabba__DabbaService_Service * service,
				    const Dabba__InterfaceStatus * statusp,
				    Dabba__ErrorCode_Closure closure,
				    void *closure_data)
{
	Dabba__ErrorCode err = DABBA__ERROR_CODE__INIT;
	struct nl_sock *sock = NULL;
	struct nl_cache *cache;
	struct rtnl_link *link, *change;
	int rc;

	assert(service);
	assert(closure_data);

	cache = link_cache_alloc(&sock);
	change = rtnl_link_alloc();

	if (!cache || !change)
		goto out;

	link = rtnl_link_get_by_name(cache, statusp->id->name);

	if (!link) {
		rc = ENODEV;
		goto out;
	}

	if (statusp->has_promiscous) {
		if (statusp->promiscous)
			rtnl_link_set_flags(change, IFF_PROMISC);
		else
			rtnl_link_unset_flags(change, IFF_PROMISC);
	}

	if (statusp->has_up) {
		if (statusp->up)
			rtnl_link_set_flags(change, IFF_UP);
		else
			rtnl_link_unset_flags(change, IFF_UP);
	}

	rc = rtnl_link_change(sock, link, change, 0);
	rtnl_link_put(link);
 out:
	err.code = rc;
	closure(&err, closure_data);
	nl_object_free(OBJ_CAST(change));
	link_cache_destroy(sock, cache);
}
