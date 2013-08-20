/**
 * \file interface-offload.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */


/* HACK prevent libnl3 include clash between <net/if.h> and <linux/if.h> */
#ifndef _LINUX_IF_H
#define _LINUX_IF_H
#endif				/* _LINUX_IF_H */

#include <assert.h>
#include <errno.h>
#include <netlink/cache.h>
#include <netlink/route/link.h>
#include <libdabba/interface.h>
#include <dabbad/interface.h>
#include <dabbad/interface-offload.h>

/**
 * \internal
 * \brief Get the offload settings of a network interface
 * \param[in]           obj	        Pointer to interface netlink structure
 * \param[in]           arg             Pointer to interface offload protobuf message
 * \note Might silently skip an interface if memory could not be allocated.
 */

static void __interface_offload_get(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = (struct rtnl_link *)obj;
	Dabba__InterfaceOffloadList *offload_list = arg;
	Dabba__InterfaceOffload *offloadp, **listpp;
	size_t lsize = sizeof(*offload_list->list) * (offload_list->n_list + 1);
	int rc;

	listpp = realloc(offload_list->list, lsize);

	if (!listpp)
		return;

	offload_list->list = listpp;
	offload_list->list[offload_list->n_list] =
	    malloc(sizeof(*offload_list->list[offload_list->n_list]));
	offloadp = offload_list->list[offload_list->n_list];

	if (!offloadp)
		return;

	dabba__interface_offload__init(offloadp);

	offloadp->id = malloc(sizeof(*offloadp->id));
	offloadp->status = malloc(sizeof(*offloadp->status));

	if (!offloadp->id || !offloadp->status) {
		free(offloadp->status);
		free(offloadp->id);
		free(offloadp);
		return;
	}

	dabba__interface_id__init(offloadp->id);
	dabba__error_code__init(offloadp->status);

	offloadp->id->name = rtnl_link_get_name(link);

	offloadp->has_rx_csum = offloadp->has_tx_csum = offloadp->has_sg = 1;
	offloadp->has_tso = offloadp->has_ufo = offloadp->has_gso = 1;
	offloadp->has_gro = offloadp->has_lro = offloadp->rxhash = 1;

	/* FIXME find a way to report error separately */
	rc = ldab_dev_rx_csum_offload_get(offloadp->id->name,
					 &offloadp->rx_csum);
	rc = ldab_dev_tx_csum_offload_get(offloadp->id->name,
					 &offloadp->tx_csum);
	rc = ldab_dev_scatter_gather_get(offloadp->id->name, &offloadp->sg);
	rc = ldab_dev_tcp_seg_offload_get(offloadp->id->name, &offloadp->tso);
	rc = ldab_dev_udp_frag_offload_get(offloadp->id->name, &offloadp->ufo);
	rc = ldab_dev_generic_seg_offload_get(offloadp->id->name,
					     &offloadp->gso);
	rc = ldab_dev_generic_rcv_offload_get(offloadp->id->name,
					     &offloadp->gro);

	offloadp->status->code = rc;

	offload_list->n_list++;
}

/**
 * \brief Get the offload settings of a list of requested network interfaces
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           id_list         Pointer to the requested interface id list
 * \param[in]           closure         Pointer to protobuf closure function pointer
 * \param[in,out]       closure_data	Pointer to protobuf closure data
 * \note Might silently skip an interface if memory could not be allocated.
 */

void dabbad_interface_offload_get(Dabba__DabbaService_Service * service,
				  const Dabba__InterfaceIdList * id_list,
				  Dabba__InterfaceOffloadList_Closure
				  closure, void *closure_data)
{
	Dabba__InterfaceOffloadList pause_list =
	    DABBA__INTERFACE_PAUSE_LIST__INIT;
	Dabba__InterfaceOffloadList *pause_listp = NULL;
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
						__interface_offload_get,
						&pause_list);
		}
	} else
		nl_cache_foreach(cache, __interface_offload_get, &pause_list);

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
 * \brief Modify the offload settings of a requested network interface
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           offloadp        Pointer to the new interface offload settings
 * \param[in]           closure         Pointer to protobuf closure function pointer
 * \param[in,out]       closure_data	Pointer to protobuf closure data
 * \note This RPC only modifies the requested offload settings
 * \note If the requested interface coalesce settings cannot be fetched, no modification will occur.
 * \note The offload settings are applied by best-effort,
 *       if a modification fails no further modifications are applied
 */

void dabbad_interface_offload_modify(Dabba__DabbaService_Service * service,
				     const Dabba__InterfaceOffload * offloadp,
				     Dabba__ErrorCode_Closure closure,
				     void *closure_data)
{
	struct nl_sock *sock = NULL;
	struct nl_cache *cache;
	struct rtnl_link *link = NULL;
	int rc = 0;

	assert(service);
	assert(closure_data);

	cache = link_cache_alloc(&sock);

	if (!cache) {
		rc = ENOMEM;
		goto out;
	}

	link = rtnl_link_get_by_name(cache, offloadp->id->name);

	if (!link) {
		rc = ENODEV;
		goto out;
	}

	if (!rc && offloadp->has_rx_csum)
		rc = ldab_dev_rx_csum_offload_set(offloadp->id->name,
						 offloadp->rx_csum);

	if (!rc && offloadp->has_tx_csum)
		rc = ldab_dev_tx_csum_offload_set(offloadp->id->name,
						 offloadp->tx_csum);

	if (!rc && offloadp->has_sg)
		rc = ldab_dev_scatter_gather_set(offloadp->id->name,
						offloadp->sg);

	if (!rc && offloadp->has_tso)
		rc = ldab_dev_tcp_seg_offload_set(offloadp->id->name,
						 offloadp->tso);

	if (!rc && offloadp->has_ufo)
		rc = ldab_dev_udp_frag_offload_set(offloadp->id->name,
						  offloadp->ufo);

	if (!rc && offloadp->has_gso)
		rc = ldab_dev_generic_seg_offload_set(offloadp->id->name,
						     offloadp->gso);

	if (!rc && offloadp->has_gro)
		rc = ldab_dev_generic_rcv_offload_set(offloadp->id->name,
						     offloadp->gro);

 out:
	offloadp->status->code = rc;
	closure(offloadp->status, closure_data);
	rtnl_link_put(link);
	link_cache_destroy(sock, cache);
}
