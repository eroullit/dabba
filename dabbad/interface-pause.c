/**
 * \file interface-pause.c
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
#include <dabbad/interface-pause.h>

/**
 * \internal
 * \brief Get the pause settings of a network interface
 * \param[in]           obj	        Pointer to interface netlink structure
 * \param[in]           arg             Pointer to interface pause protobuf message
 * \note Might silently skip an interface if memory could not be allocated.
 */

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
	pausep->status->code = ldab_dev_pause_get(pausep->id->name, &pause);

	pausep->has_autoneg = pausep->has_rx_pause = pausep->has_tx_pause = 1;
	pausep->autoneg = pause.autoneg;
	pausep->rx_pause = pause.rx_pause;
	pausep->tx_pause = pause.tx_pause;

	pause_list->n_list++;
}

/**
 * \brief Get the pause settings of a list of requested network interfaces
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           id_list         Pointer to the requested interface id list
 * \param[in]           closure         Pointer to protobuf closure function pointer
 * \param[in,out]       closure_data	Pointer to protobuf closure data
 * \note Might silently skip an interface if memory could not be allocated.
 */

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

/**
 * \brief Modify the pause settings of a requested network interface
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           pause           Pointer to the new interface pause settings
 * \param[in]           closure         Pointer to protobuf closure function pointer
 * \param[in,out]       closure_data	Pointer to protobuf closure data
 * \note This RPC only modifies the requested pause settings
 * \note If the requested interface pause settings cannot be fetched, no modification will occur.
 */

void dabbad_interface_pause_modify(Dabba__DabbaService_Service * service,
				   const Dabba__InterfacePause * pause,
				   Dabba__ErrorCode_Closure
				   closure, void *closure_data)
{
	struct ethtool_pauseparam p;
	int rc;

	assert(service);
	assert(closure_data);

	rc = ldab_dev_pause_get(pause->id->name, &p);

	if (rc)
		goto out;

	if (pause->has_rx_pause)
		p.rx_pause = pause->rx_pause;

	if (pause->has_tx_pause)
		p.tx_pause = pause->tx_pause;

	if (pause->has_autoneg)
		p.autoneg = pause->autoneg;

	rc = ldab_dev_pause_set(pause->id->name, &p);

 out:
	pause->status->code = rc;
	closure(pause->status, closure_data);
}
