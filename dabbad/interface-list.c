/**
 * \file interface-list.c
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
#include <netlink/cache.h>
#include <netlink/route/link.h>
#include <dabbad/interface.h>
#include <dabbad/interface-list.h>

static void __interface_id_get(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = (struct rtnl_link *)obj;
	Dabba__InterfaceIdList *id_listp = (Dabba__InterfaceIdList *) arg;
	Dabba__InterfaceId **idpp;
	size_t lsize = sizeof(*id_listp->list) * (id_listp->n_list + 1);

	idpp = realloc(id_listp->list, lsize);

	if (!idpp)
		return;

	id_listp->list = idpp;
	id_listp->list[id_listp->n_list] =
	    malloc(sizeof(*id_listp->list[id_listp->n_list]));

	if (!id_listp->list[id_listp->n_list])
		return;

	dabba__interface_id__init(id_listp->list[id_listp->n_list]);
	id_listp->list[id_listp->n_list]->name = rtnl_link_get_name(link);
	id_listp->n_list++;
}

void dabbad_interface_id_get(Dabba__DabbaService_Service * service,
			     const Dabba__Dummy * dummy,
			     Dabba__InterfaceIdList_Closure closure,
			     void *closure_data)
{
	Dabba__InterfaceIdList id_list = DABBA__INTERFACE_ID_LIST__INIT;
	Dabba__InterfaceIdList *id_listp = NULL;
	struct nl_sock *sock = NULL;
	struct nl_cache *cache = NULL;
	size_t a;

	assert(service);
	assert(dummy);

	cache = link_cache_alloc(&sock);

	if (!cache)
		goto out;

	nl_cache_foreach(cache, __interface_id_get, &id_list);
	id_listp = &id_list;

 out:
	closure(id_listp, closure_data);

	for (a = 0; a < id_list.n_list; a++)
		free(id_list.list[a]);

	free(id_list.list);
	link_cache_destroy(sock, cache);
}
