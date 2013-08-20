/**
 * \file interface-status.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */


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

/**
 * \brief Get the status of a list of requested network interfaces
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           id_list         Pointer to the requested interface id list
 * \param[in]           closure         Pointer to protobuf closure function pointer
 * \param[in,out]       closure_data	Pointer to protobuf closure data
 * \note Might silently skip an interface if memory could not be allocated.
 */

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
	statusp->status = malloc(sizeof(*statusp->status));

	if (!statusp->id || !statusp->status) {
		free(statusp->status);
		free(statusp->id);
		free(statusp);
		return;
	}

	dabba__interface_id__init(statusp->id);
	dabba__error_code__init(statusp->status);

	statusp->has_connectivity = statusp->has_loopback = statusp->has_up = 1;
	statusp->has_promiscuous = statusp->has_running = 1;

	statusp->id->name = rtnl_link_get_name(link);
	flags = rtnl_link_get_flags(link);

	statusp->status->code =
	    ldab_dev_link_get(statusp->id->name, &statusp->connectivity);

	statusp->loopback = (flags & IFF_LOOPBACK) == IFF_LOOPBACK;
	statusp->up = (flags & IFF_UP) == IFF_UP;
	statusp->running = (flags & IFF_RUNNING) == IFF_RUNNING;
	statusp->promiscuous = (flags & IFF_PROMISC) == IFF_PROMISC;

	status_list->n_list++;
}

/**
 * \brief Get the status of a list of requested network interfaces
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           id_list         Pointer to the requested interface id list
 * \param[in]           closure         Pointer to protobuf closure function pointer
 * \param[in,out]       closure_data	Pointer to protobuf closure data
 * \note Might silently skip a thread if memory could not be allocated.
 */

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
		free(status_list.list[a]->status);
		free(status_list.list[a]->id);
		free(status_list.list[a]);
	}
	free(status_list.list);
	link_destroy(link);
	link_cache_destroy(sock, cache);
}

/**
 * \brief Modify the status of a requested network interface
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           statusp         Pointer to the new interface status
 * \param[in]           closure         Pointer to protobuf closure function pointer
 * \param[in,out]       closure_data	Pointer to protobuf closure data
 * \note This RPC only modifies the requested interface status
 */

void dabbad_interface_status_modify(Dabba__DabbaService_Service * service,
				    const Dabba__InterfaceStatus * statusp,
				    Dabba__ErrorCode_Closure closure,
				    void *closure_data)
{
	struct nl_sock *sock = NULL;
	struct nl_cache *cache;
	struct rtnl_link *link, *change;
	int rc;

	assert(service);
	assert(closure_data);

	cache = link_cache_alloc(&sock);
	change = rtnl_link_alloc();

	if (!cache || !change) {
		rc = ENOMEM;
		goto out;
	}

	link = rtnl_link_get_by_name(cache, statusp->id->name);

	if (!link) {
		rc = ENODEV;
		goto out;
	}

	if (statusp->has_promiscuous) {
		if (statusp->promiscuous)
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
	statusp->status->code = rc;
	closure(statusp->status, closure_data);
	link_destroy(change);
	link_cache_destroy(sock, cache);
}
