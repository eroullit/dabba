/**
 * \file interface-coalesce.c
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
#include <dabbad/interface-coalesce.h>

/**
 * \internal
 * \brief Get the coalesce settings of a network interface
 * \param[in]           obj	        Pointer to interface netlink structure
 * \param[in]           arg             Pointer to interface coalesce protobuf message
 * \note Might silently skip an interface if memory could not be allocated.
 */

static void __interface_coalesce_get(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = (struct rtnl_link *)obj;
	Dabba__InterfaceCoalesceList *coalesce_list = arg;
	Dabba__InterfaceCoalesce *coalescep, **listpp;
	size_t lsize =
	    sizeof(*coalesce_list->list) * (coalesce_list->n_list + 1);
	struct ethtool_coalesce coalesce;

	listpp = realloc(coalesce_list->list, lsize);

	if (!listpp)
		return;

	coalesce_list->list = listpp;
	coalesce_list->list[coalesce_list->n_list] =
	    malloc(sizeof(*coalesce_list->list[coalesce_list->n_list]));
	coalescep = coalesce_list->list[coalesce_list->n_list];

	if (!coalescep)
		return;

	dabba__interface_coalesce__init(coalescep);

	coalescep->id = malloc(sizeof(*coalescep->id));
	coalescep->status = malloc(sizeof(*coalescep->status));

	if (!coalescep->id || !coalescep->status) {
		free(coalescep->status);
		free(coalescep->id);
		free(coalescep);
		return;
	}

	dabba__interface_id__init(coalescep->id);
	dabba__error_code__init(coalescep->status);

	coalescep->id->name = rtnl_link_get_name(link);
	coalescep->status->code =
	    ldab_dev_coalesce_get(coalescep->id->name, &coalesce);

	coalescep->has_pkt_rate_high = coalescep->has_pkt_rate_low =
	    coalescep->has_rate_sample_interval =
	    coalescep->has_stats_block_coalesce_usecs =
	    coalescep->has_use_adaptive_rx_coalesce =
	    coalescep->has_rx_coalesce_usecs =
	    coalescep->has_rx_coalesce_usecs_irq =
	    coalescep->has_rx_coalesce_usecs_high =
	    coalescep->has_rx_coalesce_usecs_low =
	    coalescep->has_rx_max_coalesced_frames =
	    coalescep->has_rx_max_coalesced_frames_irq =
	    coalescep->has_rx_max_coalesced_frames_high =
	    coalescep->has_rx_max_coalesced_frames_low =
	    coalescep->has_use_adaptive_tx_coalesce =
	    coalescep->has_tx_coalesce_usecs =
	    coalescep->has_tx_coalesce_usecs_irq =
	    coalescep->has_tx_coalesce_usecs_high =
	    coalescep->has_tx_coalesce_usecs_low =
	    coalescep->has_tx_max_coalesced_frames =
	    coalescep->has_tx_max_coalesced_frames_irq =
	    coalescep->has_tx_max_coalesced_frames_high =
	    coalescep->has_tx_max_coalesced_frames_low = 1;

	coalescep->pkt_rate_high = coalesce.pkt_rate_high;
	coalescep->pkt_rate_low = coalesce.pkt_rate_low;
	coalescep->rate_sample_interval = coalesce.rate_sample_interval;
	coalescep->stats_block_coalesce_usecs =
	    coalesce.stats_block_coalesce_usecs;
	coalescep->use_adaptive_rx_coalesce = coalesce.use_adaptive_rx_coalesce;
	coalescep->rx_coalesce_usecs = coalesce.rx_coalesce_usecs;
	coalescep->rx_coalesce_usecs_irq = coalesce.rx_coalesce_usecs_irq;
	coalescep->rx_coalesce_usecs_high = coalesce.rx_coalesce_usecs_high;
	coalescep->rx_coalesce_usecs_low = coalesce.rx_coalesce_usecs_low;
	coalescep->rx_max_coalesced_frames = coalesce.rx_max_coalesced_frames;
	coalescep->rx_max_coalesced_frames_irq =
	    coalesce.rx_max_coalesced_frames_irq;
	coalescep->rx_max_coalesced_frames_high =
	    coalesce.rx_max_coalesced_frames_high;
	coalescep->rx_max_coalesced_frames_low =
	    coalesce.rx_max_coalesced_frames_low;
	coalescep->use_adaptive_tx_coalesce = coalesce.use_adaptive_tx_coalesce;
	coalescep->tx_coalesce_usecs = coalesce.tx_coalesce_usecs;
	coalescep->tx_coalesce_usecs_irq = coalesce.tx_coalesce_usecs_irq;
	coalescep->tx_coalesce_usecs_high = coalesce.tx_coalesce_usecs_high;
	coalescep->tx_coalesce_usecs_low = coalesce.tx_coalesce_usecs_low;
	coalescep->tx_max_coalesced_frames = coalesce.tx_max_coalesced_frames;
	coalescep->tx_max_coalesced_frames_irq =
	    coalesce.tx_max_coalesced_frames_irq;
	coalescep->tx_max_coalesced_frames_high =
	    coalesce.tx_max_coalesced_frames_high;
	coalescep->tx_max_coalesced_frames_low =
	    coalesce.tx_max_coalesced_frames_low;

	coalesce_list->n_list++;
}

/**
 * \brief Get the coalesce settings of a list of requested network interfaces
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           id_list         Pointer to the requested interface id list
 * \param[in]           closure         Pointer to protobuf closure function pointer
 * \param[in,out]       closure_data	Pointer to protobuf closure data
 * \note Might silently skip an interface if memory could not be allocated.
 */

void dabbad_interface_coalesce_get(Dabba__DabbaService_Service * service,
				   const Dabba__InterfaceIdList * id_list,
				   Dabba__InterfaceCoalesceList_Closure
				   closure, void *closure_data)
{
	Dabba__InterfaceCoalesceList coalesce_list =
	    DABBA__INTERFACE_PAUSE_LIST__INIT;
	Dabba__InterfaceCoalesceList *coalesce_listp = NULL;
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
						__interface_coalesce_get,
						&coalesce_list);
		}
	} else
		nl_cache_foreach(cache, __interface_coalesce_get,
				 &coalesce_list);

	coalesce_listp = &coalesce_list;

 out:
	closure(coalesce_listp, closure_data);
	for (a = 0; a < coalesce_list.n_list; a++) {
		free(coalesce_list.list[a]->status);
		free(coalesce_list.list[a]->id);
		free(coalesce_list.list[a]);
	}
	free(coalesce_list.list);
	link_destroy(link);
	link_cache_destroy(sock, cache);
}

/**
 * \brief Modify the coalesce settings of a requested network interface
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           coalescep       Pointer to the new interface coalesce settings
 * \param[in]           closure         Pointer to protobuf closure function pointer
 * \param[in,out]       closure_data	Pointer to protobuf closure data
 * \note This RPC only modifies the requested coalesce settings
 * \note If the requested interface coalesce settings cannot be fetched, no modification will occur.
 */

void dabbad_interface_coalesce_modify(Dabba__DabbaService_Service * service,
				      const Dabba__InterfaceCoalesce *
				      coalescep,
				      Dabba__ErrorCode_Closure closure,
				      void *closure_data)
{
	struct ethtool_coalesce eth_coalesce;
	int apply = 0, rc;

	assert(service);
	assert(closure_data);

	rc = ldab_dev_coalesce_get(coalescep->id->name, &eth_coalesce);

	if (rc)
		goto out;

	if (coalescep->has_pkt_rate_high) {
		eth_coalesce.pkt_rate_high = coalescep->pkt_rate_high;
		apply = 1;
	}

	if (coalescep->has_pkt_rate_low) {
		eth_coalesce.pkt_rate_low = coalescep->pkt_rate_low;
		apply = 1;
	}

	if (coalescep->has_pkt_rate_low) {
		eth_coalesce.pkt_rate_low = coalescep->pkt_rate_low;
		apply = 1;
	}

	if (coalescep->has_rate_sample_interval) {
		eth_coalesce.rate_sample_interval =
		    coalescep->rate_sample_interval;
		apply = 1;
	}

	if (coalescep->has_stats_block_coalesce_usecs) {
		eth_coalesce.stats_block_coalesce_usecs =
		    coalescep->stats_block_coalesce_usecs;
		apply = 1;
	}

	if (coalescep->has_use_adaptive_rx_coalesce) {
		eth_coalesce.use_adaptive_rx_coalesce =
		    coalescep->use_adaptive_rx_coalesce;
		apply = 1;
	}

	if (coalescep->has_rx_coalesce_usecs) {
		eth_coalesce.rx_coalesce_usecs = coalescep->rx_coalesce_usecs;
		apply = 1;
	}

	if (coalescep->has_rx_coalesce_usecs_irq) {
		eth_coalesce.rx_coalesce_usecs_irq =
		    coalescep->rx_coalesce_usecs_irq;
		apply = 1;
	}

	if (coalescep->has_rx_coalesce_usecs_high) {
		eth_coalesce.rx_coalesce_usecs_high =
		    coalescep->rx_coalesce_usecs_high;
		apply = 1;
	}

	if (coalescep->has_rx_coalesce_usecs_low) {
		eth_coalesce.rx_coalesce_usecs_low =
		    coalescep->rx_coalesce_usecs_low;
		apply = 1;
	}

	if (coalescep->has_rx_max_coalesced_frames) {
		eth_coalesce.rx_max_coalesced_frames =
		    coalescep->rx_max_coalesced_frames;
		apply = 1;
	}

	if (coalescep->has_rx_max_coalesced_frames_irq) {
		eth_coalesce.rx_max_coalesced_frames_irq =
		    coalescep->rx_max_coalesced_frames_irq;
		apply = 1;
	}

	if (coalescep->has_rx_max_coalesced_frames_high) {
		eth_coalesce.rx_max_coalesced_frames_high =
		    coalescep->rx_max_coalesced_frames_high;
		apply = 1;
	}

	if (coalescep->has_rx_max_coalesced_frames_low) {
		eth_coalesce.rx_max_coalesced_frames_low =
		    coalescep->rx_max_coalesced_frames_low;
		apply = 1;
	}

	if (coalescep->has_use_adaptive_tx_coalesce) {
		eth_coalesce.use_adaptive_tx_coalesce =
		    coalescep->use_adaptive_tx_coalesce;
		apply = 1;
	}

	if (coalescep->has_tx_coalesce_usecs) {
		eth_coalesce.tx_coalesce_usecs = coalescep->tx_coalesce_usecs;
		apply = 1;
	}

	if (coalescep->has_tx_coalesce_usecs_irq) {
		eth_coalesce.tx_coalesce_usecs_irq =
		    coalescep->tx_coalesce_usecs_irq;
		apply = 1;
	}

	if (coalescep->has_tx_coalesce_usecs_high) {
		eth_coalesce.tx_coalesce_usecs_high =
		    coalescep->tx_coalesce_usecs_high;
		apply = 1;
	}

	if (coalescep->has_tx_coalesce_usecs_low) {
		eth_coalesce.tx_coalesce_usecs_low =
		    coalescep->tx_coalesce_usecs_low;
		apply = 1;
	}

	if (coalescep->has_tx_max_coalesced_frames) {
		eth_coalesce.tx_max_coalesced_frames =
		    coalescep->tx_max_coalesced_frames;
		apply = 1;
	}

	if (coalescep->has_tx_max_coalesced_frames_irq) {
		eth_coalesce.tx_max_coalesced_frames_irq =
		    coalescep->tx_max_coalesced_frames_irq;
		apply = 1;
	}

	if (coalescep->has_tx_max_coalesced_frames_high) {
		eth_coalesce.tx_max_coalesced_frames_high =
		    coalescep->tx_max_coalesced_frames_high;
		apply = 1;
	}

	if (coalescep->has_tx_max_coalesced_frames_low) {
		eth_coalesce.tx_max_coalesced_frames_low =
		    coalescep->tx_max_coalesced_frames_low;
		apply = 1;
	}

	if (apply)
		rc = ldab_dev_coalesce_set(coalescep->id->name, &eth_coalesce);

 out:
	coalescep->status->code = rc;
	closure(coalescep->status, closure_data);
}
