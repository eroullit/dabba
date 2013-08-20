/**
 * \file interface-capabilities.c
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
#include <dabbad/interface-capabilities.h>

/**
 * \internal
 * \brief Get the capabilities of a network interface
 * \param[in]           obj	        Pointer to interface netlink structure
 * \param[in]           arg             Pointer to interface capabilities protobuf message
 * \note Might silently skip an interface if memory could not be allocated.
 */

static void __interface_capabilities_get(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = (struct rtnl_link *)obj;
	Dabba__InterfaceCapabilitiesList *capabilities_list = arg;
	Dabba__InterfaceCapabilities *capabilitiesp, **listpp;
	size_t lsize =
	    sizeof(*capabilities_list->list) * (capabilities_list->n_list + 1);
	struct ethtool_cmd capabilities;

	listpp = realloc(capabilities_list->list, lsize);

	if (!listpp)
		return;

	capabilities_list->list = listpp;
	capabilities_list->list[capabilities_list->n_list] =
	    malloc(sizeof(*capabilities_list->list[capabilities_list->n_list]));
	capabilitiesp = capabilities_list->list[capabilities_list->n_list];

	if (!capabilitiesp)
		return;

	dabba__interface_capabilities__init(capabilitiesp);

	capabilitiesp->id = malloc(sizeof(*capabilitiesp->id));
	capabilitiesp->status = malloc(sizeof(*capabilitiesp->status));
	capabilitiesp->supported_opt =
	    malloc(sizeof(*capabilitiesp->supported_opt));
	capabilitiesp->supported_speed =
	    malloc(sizeof(*capabilitiesp->supported_speed));
	capabilitiesp->advertising_opt =
	    malloc(sizeof(*capabilitiesp->advertising_opt));
	capabilitiesp->advertising_speed =
	    malloc(sizeof(*capabilitiesp->advertising_speed));
	capabilitiesp->lp_advertising_opt =
	    malloc(sizeof(*capabilitiesp->lp_advertising_opt));
	capabilitiesp->lp_advertising_speed =
	    malloc(sizeof(*capabilitiesp->lp_advertising_speed));

	if (!capabilitiesp->id || !capabilitiesp->status
	    || !capabilitiesp->supported_opt || !capabilitiesp->supported_speed
	    || !capabilitiesp->advertising_opt
	    || !capabilitiesp->advertising_speed
	    || !capabilitiesp->lp_advertising_opt
	    || !capabilitiesp->lp_advertising_speed) {
		free(capabilitiesp->id);
		free(capabilitiesp->status);
		free(capabilitiesp->supported_opt);
		free(capabilitiesp->supported_speed);
		free(capabilitiesp->advertising_opt);
		free(capabilitiesp->advertising_speed);
		free(capabilitiesp->lp_advertising_opt);
		free(capabilitiesp->lp_advertising_speed);
		free(capabilitiesp);
		return;
	}

	dabba__interface_id__init(capabilitiesp->id);
	dabba__error_code__init(capabilitiesp->status);
	dabba__interface_speed_capabilites__init
	    (capabilitiesp->supported_speed);
	dabba__interface_option_capabilites__init(capabilitiesp->supported_opt);
	dabba__interface_speed_capabilites__init
	    (capabilitiesp->advertising_speed);
	dabba__interface_option_capabilites__init
	    (capabilitiesp->advertising_opt);
	dabba__interface_speed_capabilites__init
	    (capabilitiesp->lp_advertising_speed);
	dabba__interface_option_capabilites__init
	    (capabilitiesp->lp_advertising_opt);

	capabilitiesp->supported_speed->ethernet =
	    malloc(sizeof(*capabilitiesp->supported_speed->ethernet));
	capabilitiesp->supported_speed->fast_ethernet =
	    malloc(sizeof(*capabilitiesp->supported_speed->fast_ethernet));
	capabilitiesp->supported_speed->gbps_ethernet =
	    malloc(sizeof(*capabilitiesp->supported_speed->gbps_ethernet));
	capabilitiesp->supported_speed->_10gbps_ethernet =
	    malloc(sizeof(*capabilitiesp->supported_speed->_10gbps_ethernet));
	capabilitiesp->advertising_speed->ethernet =
	    malloc(sizeof(*capabilitiesp->advertising_speed->ethernet));
	capabilitiesp->advertising_speed->fast_ethernet =
	    malloc(sizeof(*capabilitiesp->advertising_speed->fast_ethernet));
	capabilitiesp->advertising_speed->gbps_ethernet =
	    malloc(sizeof(*capabilitiesp->advertising_speed->gbps_ethernet));
	capabilitiesp->advertising_speed->_10gbps_ethernet =
	    malloc(sizeof(*capabilitiesp->advertising_speed->_10gbps_ethernet));
	capabilitiesp->lp_advertising_speed->ethernet =
	    malloc(sizeof(*capabilitiesp->lp_advertising_speed->ethernet));
	capabilitiesp->lp_advertising_speed->fast_ethernet =
	    malloc(sizeof(*capabilitiesp->lp_advertising_speed->fast_ethernet));
	capabilitiesp->lp_advertising_speed->gbps_ethernet =
	    malloc(sizeof(*capabilitiesp->lp_advertising_speed->gbps_ethernet));
	capabilitiesp->lp_advertising_speed->_10gbps_ethernet =
	    malloc(sizeof
		   (*capabilitiesp->lp_advertising_speed->_10gbps_ethernet));

	if (!capabilitiesp->supported_speed->ethernet
	    || !capabilitiesp->supported_speed->fast_ethernet
	    || !capabilitiesp->supported_speed->gbps_ethernet
	    || !capabilitiesp->supported_speed->_10gbps_ethernet
	    || !capabilitiesp->advertising_speed->ethernet
	    || !capabilitiesp->advertising_speed->fast_ethernet
	    || !capabilitiesp->advertising_speed->gbps_ethernet
	    || !capabilitiesp->advertising_speed->_10gbps_ethernet
	    || !capabilitiesp->lp_advertising_speed->ethernet
	    || !capabilitiesp->lp_advertising_speed->fast_ethernet
	    || !capabilitiesp->lp_advertising_speed->gbps_ethernet
	    || !capabilitiesp->lp_advertising_speed->_10gbps_ethernet) {
		free(capabilitiesp->supported_speed->ethernet);
		free(capabilitiesp->supported_speed->fast_ethernet);
		free(capabilitiesp->supported_speed->gbps_ethernet);
		free(capabilitiesp->supported_speed->_10gbps_ethernet);
		free(capabilitiesp->advertising_speed->ethernet);
		free(capabilitiesp->advertising_speed->fast_ethernet);
		free(capabilitiesp->advertising_speed->gbps_ethernet);
		free(capabilitiesp->advertising_speed->_10gbps_ethernet);
		free(capabilitiesp->lp_advertising_speed->ethernet);
		free(capabilitiesp->lp_advertising_speed->fast_ethernet);
		free(capabilitiesp->lp_advertising_speed->gbps_ethernet);
		free(capabilitiesp->lp_advertising_speed->_10gbps_ethernet);
		free(capabilitiesp->id);
		free(capabilitiesp->status);
		free(capabilitiesp->supported_opt);
		free(capabilitiesp->supported_speed);
		free(capabilitiesp->advertising_opt);
		free(capabilitiesp->advertising_speed);
		free(capabilitiesp->lp_advertising_opt);
		free(capabilitiesp->lp_advertising_speed);
		free(capabilitiesp);
	}

	dabba__interface_duplex_capabilites__init
	    (capabilitiesp->supported_speed->ethernet);
	dabba__interface_duplex_capabilites__init
	    (capabilitiesp->supported_speed->fast_ethernet);
	dabba__interface_duplex_capabilites__init
	    (capabilitiesp->supported_speed->gbps_ethernet);
	dabba__interface_duplex_capabilites__init
	    (capabilitiesp->supported_speed->_10gbps_ethernet);
	dabba__interface_duplex_capabilites__init
	    (capabilitiesp->advertising_speed->ethernet);
	dabba__interface_duplex_capabilites__init
	    (capabilitiesp->advertising_speed->fast_ethernet);
	dabba__interface_duplex_capabilites__init
	    (capabilitiesp->advertising_speed->gbps_ethernet);
	dabba__interface_duplex_capabilites__init
	    (capabilitiesp->advertising_speed->_10gbps_ethernet);
	dabba__interface_duplex_capabilites__init
	    (capabilitiesp->lp_advertising_speed->ethernet);
	dabba__interface_duplex_capabilites__init
	    (capabilitiesp->lp_advertising_speed->fast_ethernet);
	dabba__interface_duplex_capabilites__init
	    (capabilitiesp->lp_advertising_speed->gbps_ethernet);
	dabba__interface_duplex_capabilites__init
	    (capabilitiesp->lp_advertising_speed->_10gbps_ethernet);

	capabilitiesp->id->name = rtnl_link_get_name(link);
	capabilitiesp->status->code =
	    ldab_dev_settings_get(capabilitiesp->id->name, &capabilities);

	capabilitiesp->has_aui = capabilitiesp->has_bnc =
	    capabilitiesp->has_fibre = capabilitiesp->has_mii =
	    capabilitiesp->has_tp = 1;

	capabilitiesp->supported_opt->has_autoneg =
	    capabilitiesp->supported_opt->has_pause =
	    capabilitiesp->advertising_opt->has_autoneg =
	    capabilitiesp->advertising_opt->has_pause =
	    capabilitiesp->lp_advertising_opt->has_autoneg =
	    capabilitiesp->lp_advertising_opt->has_pause = 1;
	capabilitiesp->supported_speed->ethernet->has_half =
	    capabilitiesp->supported_speed->ethernet->has_full =
	    capabilitiesp->supported_speed->fast_ethernet->has_half =
	    capabilitiesp->supported_speed->fast_ethernet->has_full =
	    capabilitiesp->supported_speed->gbps_ethernet->has_half =
	    capabilitiesp->supported_speed->gbps_ethernet->has_full =
	    capabilitiesp->supported_speed->_10gbps_ethernet->has_half =
	    capabilitiesp->supported_speed->_10gbps_ethernet->has_full =
	    capabilitiesp->advertising_speed->ethernet->has_half =
	    capabilitiesp->advertising_speed->ethernet->has_full =
	    capabilitiesp->advertising_speed->fast_ethernet->has_half =
	    capabilitiesp->advertising_speed->fast_ethernet->has_full =
	    capabilitiesp->advertising_speed->gbps_ethernet->has_half =
	    capabilitiesp->advertising_speed->gbps_ethernet->has_full =
	    capabilitiesp->advertising_speed->_10gbps_ethernet->has_half =
	    capabilitiesp->advertising_speed->_10gbps_ethernet->has_full =
	    capabilitiesp->lp_advertising_speed->ethernet->has_half =
	    capabilitiesp->lp_advertising_speed->ethernet->has_full =
	    capabilitiesp->lp_advertising_speed->fast_ethernet->has_half =
	    capabilitiesp->lp_advertising_speed->fast_ethernet->has_full =
	    capabilitiesp->lp_advertising_speed->gbps_ethernet->has_half =
	    capabilitiesp->lp_advertising_speed->gbps_ethernet->has_full =
	    capabilitiesp->lp_advertising_speed->_10gbps_ethernet->has_half =
	    capabilitiesp->lp_advertising_speed->_10gbps_ethernet->has_full = 1;

	capabilitiesp->aui = (capabilities.supported & SUPPORTED_AUI);
	capabilitiesp->bnc = (capabilities.supported & SUPPORTED_BNC);
	capabilitiesp->fibre = (capabilities.supported & SUPPORTED_FIBRE);
	capabilitiesp->mii = (capabilities.supported & SUPPORTED_MII);
	capabilitiesp->tp = (capabilities.supported & SUPPORTED_TP);

	/* There is no 10Gbps half duplex support */
	capabilitiesp->supported_speed->ethernet->half =
	    (capabilities.supported & SUPPORTED_10baseT_Half);
	capabilitiesp->supported_speed->ethernet->full =
	    (capabilities.supported & SUPPORTED_10baseT_Full);
	capabilitiesp->supported_speed->fast_ethernet->half =
	    (capabilities.supported & SUPPORTED_100baseT_Half);
	capabilitiesp->supported_speed->fast_ethernet->full =
	    (capabilities.supported & SUPPORTED_100baseT_Full);
	capabilitiesp->supported_speed->gbps_ethernet->half =
	    (capabilities.supported & SUPPORTED_1000baseT_Half);
	capabilitiesp->supported_speed->gbps_ethernet->full =
	    (capabilities.supported & SUPPORTED_1000baseT_Full);
	capabilitiesp->supported_speed->_10gbps_ethernet->half = 0;
	capabilitiesp->supported_speed->_10gbps_ethernet->full =
	    (capabilities.supported & SUPPORTED_10000baseT_Full);

	capabilitiesp->supported_opt->autoneg =
	    (capabilities.supported & SUPPORTED_Autoneg);
	capabilitiesp->supported_opt->pause =
	    (capabilities.supported & SUPPORTED_Pause);

	capabilitiesp->advertising_speed->ethernet->half =
	    (capabilities.advertising & ADVERTISED_10baseT_Half);
	capabilitiesp->advertising_speed->ethernet->full =
	    (capabilities.advertising & ADVERTISED_10baseT_Full);
	capabilitiesp->advertising_speed->fast_ethernet->half =
	    (capabilities.advertising & ADVERTISED_100baseT_Half);
	capabilitiesp->advertising_speed->fast_ethernet->full =
	    (capabilities.advertising & ADVERTISED_100baseT_Full);
	capabilitiesp->advertising_speed->gbps_ethernet->half =
	    (capabilities.advertising & ADVERTISED_1000baseT_Half);
	capabilitiesp->advertising_speed->gbps_ethernet->full =
	    (capabilities.advertising & ADVERTISED_1000baseT_Full);
	capabilitiesp->advertising_speed->_10gbps_ethernet->half = 0;
	capabilitiesp->advertising_speed->_10gbps_ethernet->full =
	    (capabilities.advertising & ADVERTISED_10000baseT_Full);

	capabilitiesp->advertising_opt->autoneg =
	    (capabilities.advertising & ADVERTISED_Autoneg);
	capabilitiesp->advertising_opt->pause =
	    (capabilities.advertising & ADVERTISED_Pause);

	capabilitiesp->lp_advertising_speed->ethernet->half =
	    (capabilities.lp_advertising & ADVERTISED_10baseT_Half);
	capabilitiesp->lp_advertising_speed->ethernet->full =
	    (capabilities.lp_advertising & ADVERTISED_10baseT_Full);
	capabilitiesp->lp_advertising_speed->fast_ethernet->half =
	    (capabilities.lp_advertising & ADVERTISED_100baseT_Half);
	capabilitiesp->lp_advertising_speed->fast_ethernet->full =
	    (capabilities.lp_advertising & ADVERTISED_100baseT_Full);
	capabilitiesp->lp_advertising_speed->gbps_ethernet->half =
	    (capabilities.lp_advertising & ADVERTISED_1000baseT_Half);
	capabilitiesp->lp_advertising_speed->gbps_ethernet->full =
	    (capabilities.lp_advertising & ADVERTISED_1000baseT_Full);
	capabilitiesp->lp_advertising_speed->_10gbps_ethernet->half = 0;
	capabilitiesp->lp_advertising_speed->_10gbps_ethernet->full =
	    (capabilities.lp_advertising & ADVERTISED_10000baseT_Full);

	capabilitiesp->lp_advertising_opt->autoneg =
	    (capabilities.lp_advertising & ADVERTISED_Autoneg);
	capabilitiesp->lp_advertising_opt->pause =
	    (capabilities.lp_advertising & ADVERTISED_Pause);

	capabilities_list->n_list++;
}

/**
 * \brief Get the capabilities of a list of requested network interfaces
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           id_list         Pointer to the requested interface id list
 * \param[in]           closure         Pointer to protobuf closure function pointer
 * \param[in,out]       closure_data	Pointer to protobuf closure data
 * \note Might silently skip an interface if memory could not be allocated.
 */

void dabbad_interface_capabilities_get(Dabba__DabbaService_Service * service,
				       const Dabba__InterfaceIdList * id_list,
				       Dabba__InterfaceCapabilitiesList_Closure
				       closure, void *closure_data)
{
	Dabba__InterfaceCapabilitiesList capabilities_list =
	    DABBA__INTERFACE_PAUSE_LIST__INIT;
	Dabba__InterfaceCapabilitiesList *capabilities_listp = NULL;
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
						__interface_capabilities_get,
						&capabilities_list);
		}
	} else
		nl_cache_foreach(cache, __interface_capabilities_get,
				 &capabilities_list);

	capabilities_listp = &capabilities_list;

 out:
	closure(capabilities_listp, closure_data);
	for (a = 0; a < capabilities_list.n_list; a++) {
		free(capabilities_list.list[a]->supported_speed->ethernet);
		free(capabilities_list.list[a]->supported_speed->fast_ethernet);
		free(capabilities_list.list[a]->supported_speed->gbps_ethernet);
		free(capabilities_list.list[a]->
		     supported_speed->_10gbps_ethernet);
		free(capabilities_list.list[a]->advertising_speed->ethernet);
		free(capabilities_list.list[a]->
		     advertising_speed->fast_ethernet);
		free(capabilities_list.list[a]->
		     advertising_speed->gbps_ethernet);
		free(capabilities_list.list[a]->
		     advertising_speed->_10gbps_ethernet);
		free(capabilities_list.list[a]->lp_advertising_speed->ethernet);
		free(capabilities_list.list[a]->
		     lp_advertising_speed->fast_ethernet);
		free(capabilities_list.list[a]->
		     lp_advertising_speed->gbps_ethernet);
		free(capabilities_list.list[a]->
		     lp_advertising_speed->_10gbps_ethernet);
		free(capabilities_list.list[a]->supported_opt);
		free(capabilities_list.list[a]->supported_speed);
		free(capabilities_list.list[a]->advertising_opt);
		free(capabilities_list.list[a]->advertising_speed);
		free(capabilities_list.list[a]->lp_advertising_opt);
		free(capabilities_list.list[a]->lp_advertising_speed);
		free(capabilities_list.list[a]->status);
		free(capabilities_list.list[a]->id);
		free(capabilities_list.list[a]);
	}
	free(capabilities_list.list);
	link_destroy(link);
	link_cache_destroy(sock, cache);
}

/**
 * \internal
 * \brief Modify the advertised option capabilities from a protobuf interface option capabilities message
 * \param[in,out]       adv_opt 	Pointer to the interface advertised option capabilities flags
 * \param[in]           caps            Pointer to the new advertised interface option settings
 * \param[in,out]       apply           Pointer to flags to signal settings changes
 */

static void interface_advertising_option_set(uint32_t * const adv_opt,
					     const
					     Dabba__InterfaceOptionCapabilites *
					     caps, int *const apply)
{
	assert(adv_opt);
	assert(caps);
	assert(apply);

	if (caps->has_autoneg) {
		if (caps->autoneg)
			*adv_opt |= ADVERTISED_Autoneg;
		else
			*adv_opt &= ~ADVERTISED_Autoneg;

		*apply = 1;
	}

	if (caps->has_pause) {
		if (caps->pause)
			*adv_opt |= ADVERTISED_Pause;
		else
			*adv_opt &= ~ADVERTISED_Pause;

		*apply = 1;
	}
}

/**
 * \internal
 * \brief Modify the advertised speed capabilities from a protobuf interface speed capabilities message
 * \param[in,out]       adv_speed	Pointer to the interface advertised speed capabilities flags
 * \param[in]           speed           Pointer to the new advertised interface speed settings
 * \param[in,out]       apply           Pointer to flags to signal settings changes
 */

static void interface_speed_option_set(uint32_t * const adv_speed,
				       const Dabba__InterfaceSpeedCapabilites *
				       speed, int *const apply)
{
	assert(adv_speed);
	assert(speed);
	assert(apply);

	if (speed->ethernet) {
		if (speed->ethernet->has_half) {
			if (speed->ethernet->half)
				*adv_speed |= ADVERTISED_10baseT_Half;
			else
				*adv_speed &= ~ADVERTISED_10baseT_Half;

			*apply = 1;
		}

		if (speed->ethernet->has_full) {
			if (speed->ethernet->full)
				*adv_speed |= ADVERTISED_10baseT_Full;
			else
				*adv_speed &= ~ADVERTISED_10baseT_Full;

			*apply = 1;
		}
	}

	if (speed->fast_ethernet) {
		if (speed->fast_ethernet->has_half) {
			if (speed->fast_ethernet->half)
				*adv_speed |= ADVERTISED_100baseT_Half;
			else
				*adv_speed &= ~ADVERTISED_100baseT_Half;

			*apply = 1;
		}

		if (speed->fast_ethernet->has_full) {
			if (speed->fast_ethernet->full)
				*adv_speed |= ADVERTISED_100baseT_Full;
			else
				*adv_speed &= ~ADVERTISED_100baseT_Full;

			*apply = 1;
		}
	}

	if (speed->gbps_ethernet) {
		if (speed->gbps_ethernet->has_half) {
			if (speed->gbps_ethernet->half)
				*adv_speed |= ADVERTISED_1000baseT_Half;
			else
				*adv_speed &= ~ADVERTISED_1000baseT_Half;

			*apply = 1;
		}

		if (speed->gbps_ethernet->has_full) {
			if (speed->gbps_ethernet->full)
				*adv_speed |= ADVERTISED_1000baseT_Full;
			else
				*adv_speed &= ~ADVERTISED_1000baseT_Full;

			*apply = 1;
		}
	}

	if (speed->_10gbps_ethernet) {
		/* 10Gbps speed cannot advertise half-duplex */
		if (speed->_10gbps_ethernet->has_full) {
			if (speed->_10gbps_ethernet->full)
				*adv_speed |= ADVERTISED_10000baseT_Full;
			else
				*adv_speed &= ~ADVERTISED_10000baseT_Full;

			*apply = 1;
		}
	}
}

/**
 * \brief Modify the capabilities of a requested network interface
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           capabilitiesp   Pointer to the new interface capabilities settings
 * \param[in]           closure         Pointer to protobuf closure function pointer
 * \param[in,out]       closure_data	Pointer to protobuf closure data
 * \note This RPC only modifies the requested capabilities settings
 * \note If the requested interface capabilities cannot be fetched, no modification will occur.
 */

void dabbad_interface_capabilities_modify(Dabba__DabbaService_Service * service,
					  const Dabba__InterfaceCapabilities *
					  capabilitiesp,
					  Dabba__ErrorCode_Closure closure,
					  void *closure_data)
{
	struct nl_sock *sock = NULL;
	struct nl_cache *cache;
	struct rtnl_link *link, *change;
	struct ethtool_cmd eth_set;
	int apply = 0, rc;

	assert(service);
	assert(closure_data);

	cache = link_cache_alloc(&sock);
	change = rtnl_link_alloc();

	if (!cache || !change) {
		rc = ENOMEM;
		goto out;
	}

	link = rtnl_link_get_by_name(cache, capabilitiesp->id->name);

	if (!link) {
		rc = ENODEV;
		goto out;
	}

	rc = ldab_dev_settings_get(capabilitiesp->id->name, &eth_set);

	if (rc)
		goto out;

	if (capabilitiesp->has_aui) {
		if (capabilitiesp->aui)
			eth_set.port |= PORT_AUI;
		else
			eth_set.port &= ~PORT_AUI;

		apply = 1;
	}

	if (capabilitiesp->has_bnc) {
		if (capabilitiesp->bnc)
			eth_set.port |= PORT_BNC;
		else
			eth_set.port &= ~PORT_BNC;

		apply = 1;
	}

	if (capabilitiesp->has_fibre) {
		if (capabilitiesp->fibre)
			eth_set.port |= PORT_FIBRE;
		else
			eth_set.port &= ~PORT_FIBRE;

		apply = 1;
	}

	if (capabilitiesp->has_mii) {
		if (capabilitiesp->mii)
			eth_set.port |= PORT_MII;
		else
			eth_set.port &= ~PORT_MII;

		apply = 1;
	}

	if (capabilitiesp->has_tp) {
		if (capabilitiesp->tp)
			eth_set.port |= PORT_TP;
		else
			eth_set.port &= ~PORT_TP;

		apply = 1;
	}

	if (capabilitiesp->advertising_speed)
		interface_speed_option_set(&eth_set.advertising,
					   capabilitiesp->advertising_speed,
					   &apply);

	if (capabilitiesp->advertising_opt)
		interface_advertising_option_set(&eth_set.advertising,
						 capabilitiesp->advertising_opt,
						 &apply);

	if (apply)
		rc = ldab_dev_settings_set(capabilitiesp->id->name, &eth_set);

	rtnl_link_change(sock, link, change, 0);
	rtnl_link_put(link);
 out:
	capabilitiesp->status->code = rc;
	closure(capabilitiesp->status, closure_data);
	link_destroy(change);
	link_cache_destroy(sock, cache);
}
