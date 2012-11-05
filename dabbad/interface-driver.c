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
#include <dabbad/interface-driver.h>

void __interface_driver_get(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = (struct rtnl_link *)obj;
	Dabba__InterfaceDriverList *driver_list = arg;
	Dabba__InterfaceDriver *driverp, **listpp;
	struct ethtool_drvinfo drvinfo;
	size_t lsize = sizeof(*driver_list->list) * (driver_list->n_list + 1);

	listpp = realloc(driver_list->list, lsize);

	if (!listpp)
		return;

	driver_list->list = listpp;
	driver_list->list[driver_list->n_list] =
	    malloc(sizeof(*driver_list->list[driver_list->n_list]));
	driverp = driver_list->list[driver_list->n_list];

	if (!driverp)
		return;

	dabba__interface_driver__init(driverp);

	driverp->id = malloc(sizeof(*driverp));

	if (!driverp->id) {
		free(driverp);
		return;
	}

	dabba__interface_id__init(driverp->id);

	driverp->id->name = rtnl_link_get_name(link);

	dev_driver_get(driverp->id->name, &drvinfo);

	driverp->name = strndup(drvinfo.driver, sizeof(drvinfo.driver));
	driverp->version = strndup(drvinfo.version, sizeof(drvinfo.version));
	driverp->fw_version =
	    strndup(drvinfo.fw_version, sizeof(drvinfo.fw_version));
	driverp->bus_info = strndup(drvinfo.bus_info, sizeof(drvinfo.bus_info));

	driver_list->n_list++;
}

void dabbad_interface_driver_get(Dabba__DabbaService_Service * service,
				 const Dabba__InterfaceIdList * id_list,
				 Dabba__InterfaceDriverList_Closure
				 closure, void *closure_data)
{
	Dabba__InterfaceDriverList driver_list =
	    DABBA__INTERFACE_DRIVER_LIST__INIT;
	Dabba__InterfaceDriverList *driver_listp = NULL;
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
						__interface_driver_get,
						&driver_list);
		}
	} else
		nl_cache_foreach(cache, __interface_driver_get, &driver_list);

	driver_listp = &driver_list;

 out:
	closure(driver_listp, closure_data);
	for (a = 0; a < driver_list.n_list; a++) {
		free(driver_list.list[a]->id);
		free(driver_list.list[a]->name);
		free(driver_list.list[a]->version);
		free(driver_list.list[a]->fw_version);
		free(driver_list.list[a]->bus_info);
		free(driver_list.list[a]);
	}
	free(driver_list.list);
	nl_object_free(OBJ_CAST(link));
	link_cache_destroy(sock, cache);
}
