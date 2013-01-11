/**
 * \file interface-settings.c
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
#include <dabbad/interface-settings.h>

static void __interface_settings_get(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = (struct rtnl_link *)obj;
	Dabba__InterfaceSettingsList *settings_list = arg;
	Dabba__InterfaceSettings *settingsp, **listpp;
	size_t lsize =
	    sizeof(*settings_list->list) * (settings_list->n_list + 1);
	struct ethtool_cmd settings;

	listpp = realloc(settings_list->list, lsize);

	if (!listpp)
		return;

	settings_list->list = listpp;
	settings_list->list[settings_list->n_list] =
	    malloc(sizeof(*settings_list->list[settings_list->n_list]));
	settingsp = settings_list->list[settings_list->n_list];

	if (!settingsp)
		return;

	dabba__interface_settings__init(settingsp);

	settingsp->id = malloc(sizeof(*settingsp));

	if (!settingsp->id) {
		free(settingsp);
		return;
	}

	dabba__interface_id__init(settingsp->id);

	settingsp->id->name = rtnl_link_get_name(link);
	settingsp->has_speed = settingsp->has_duplex = 1;
	settingsp->has_autoneg = settingsp->has_mtu = 1;
	settingsp->has_tx_qlen = settingsp->has_port = 1;
	settingsp->has_maxrxpkt = settingsp->has_maxtxpkt = 1;

	dev_settings_get(settingsp->id->name, &settings);
	settingsp->mtu = rtnl_link_get_mtu(link);
	settingsp->tx_qlen = rtnl_link_get_txqlen(link);

	settingsp->speed = ethtool_cmd_speed(&settings);
	settingsp->duplex = settings.duplex == DUPLEX_FULL;
	settingsp->autoneg = settings.autoneg == AUTONEG_ENABLE;
	settingsp->maxrxpkt = settings.maxrxpkt;
	settingsp->maxtxpkt = settings.maxtxpkt;

	settings_list->n_list++;
}

void dabbad_interface_settings_get(Dabba__DabbaService_Service * service,
				   const Dabba__InterfaceIdList * id_list,
				   Dabba__InterfaceSettingsList_Closure
				   closure, void *closure_data)
{
	Dabba__InterfaceSettingsList pause_list =
	    DABBA__INTERFACE_PAUSE_LIST__INIT;
	Dabba__InterfaceSettingsList *pause_listp = NULL;
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
						__interface_settings_get,
						&pause_list);
		}
	} else
		nl_cache_foreach(cache, __interface_settings_get, &pause_list);

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
