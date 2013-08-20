/**
 * \file interface-settings.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */


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

/**
 * \internal
 * \brief Get the network interface settings
 * \param[in]           obj	        Pointer to interface netlink structure
 * \param[in]           arg             Pointer to interface settings protobuf message
 * \note Might silently skip an interface if memory could not be allocated.
 */

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

	settingsp->id = malloc(sizeof(*settingsp->id));
	settingsp->status = malloc(sizeof(*settingsp->status));

	if (!settingsp->id || !settingsp->status) {
		free(settingsp->id);
		free(settingsp->status);
		free(settingsp);
		return;
	}

	dabba__interface_id__init(settingsp->id);
	dabba__error_code__init(settingsp->status);

	settingsp->id->name = rtnl_link_get_name(link);
	settingsp->has_speed = settingsp->has_duplex = 1;
	settingsp->has_autoneg = settingsp->has_mtu = 1;
	settingsp->has_tx_qlen = settingsp->has_port = 1;
	settingsp->has_maxrxpkt = settingsp->has_maxtxpkt = 1;

	settingsp->status->code =
	    ldab_dev_settings_get(settingsp->id->name, &settings);
	settingsp->mtu = rtnl_link_get_mtu(link);
	settingsp->tx_qlen = rtnl_link_get_txqlen(link);

	settingsp->speed = ethtool_cmd_speed(&settings);
	settingsp->duplex = settings.duplex;
	settingsp->autoneg = settings.autoneg == AUTONEG_ENABLE;
	settingsp->maxrxpkt = settings.maxrxpkt;
	settingsp->maxtxpkt = settings.maxtxpkt;

	settings_list->n_list++;
}

/**
 * \brief Get the settings of a list of requested network interfaces
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           id_list         Pointer to the requested interface id list
 * \param[in]           closure         Pointer to protobuf closure function pointer
 * \param[in,out]       closure_data	Pointer to protobuf closure data
 * \note Might silently skip an interface if memory could not be allocated.
 */

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
		free(pause_list.list[a]->status);
		free(pause_list.list[a]->id);
		free(pause_list.list[a]);
	}
	free(pause_list.list);
	link_destroy(link);
	link_cache_destroy(sock, cache);
}

/**
 * \brief Modify the settings of a requested network interface
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           settingsp       Pointer to the new interface settings
 * \param[in]           closure         Pointer to protobuf closure function pointer
 * \param[in,out]       closure_data	Pointer to protobuf closure data
 * \note This RPC only modifies the requested interface settings
 * \note If the requested interface hardware settings cannot be fetched, no hardware modification will occur.
 */

void dabbad_interface_settings_modify(Dabba__DabbaService_Service * service,
				      const Dabba__InterfaceSettings *
				      settingsp,
				      Dabba__ErrorCode_Closure closure,
				      void *closure_data)
{
	struct nl_sock *sock = NULL;
	struct nl_cache *cache;
	struct rtnl_link *link, *change;
	struct ethtool_cmd eth_set;
	int eth_apply = 0, nl_apply = 0, rc;

	assert(service);
	assert(closure_data);

	cache = link_cache_alloc(&sock);
	change = rtnl_link_alloc();

	if (!cache || !change) {
		rc = ENOMEM;
		goto out;
	}

	link = rtnl_link_get_by_name(cache, settingsp->id->name);

	if (!link) {
		rc = ENODEV;
		goto out;
	}

	rc = ldab_dev_settings_get(settingsp->id->name, &eth_set);

	if (!rc) {
		if (settingsp->has_speed) {
			ethtool_cmd_speed_set(&eth_set, settingsp->speed);
			eth_apply = 1;
		}

		if (settingsp->has_duplex) {
			eth_set.duplex = settingsp->duplex;
			eth_apply = 1;
		}

		if (settingsp->has_autoneg) {
			eth_set.autoneg = settingsp->autoneg;
			eth_apply = 1;
		}

		if (settingsp->has_port) {
			eth_set.port = settingsp->port;
			eth_apply = 1;
		}

		if (settingsp->has_maxrxpkt) {
			eth_set.maxrxpkt = settingsp->maxrxpkt;
			eth_apply = 1;
		}

		if (settingsp->has_maxtxpkt) {
			eth_set.maxtxpkt = settingsp->maxtxpkt;
			eth_apply = 1;
		}
	}

	if (settingsp->has_mtu) {
		rtnl_link_set_mtu(change, settingsp->mtu);
		nl_apply = 1;
	}

	if (settingsp->has_tx_qlen) {
		rtnl_link_set_txqlen(change, settingsp->tx_qlen);
		nl_apply = 1;
	}

	if (!rc && eth_apply)
		rc = ldab_dev_settings_set(settingsp->id->name, &eth_set);

	if (nl_apply)
		rc = rtnl_link_change(sock, link, change, 0);

	rtnl_link_put(link);
 out:
	settingsp->status->code = rc;
	closure(settingsp->status, closure_data);
	link_destroy(change);
	link_cache_destroy(sock, cache);
}
