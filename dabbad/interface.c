/**
 * \file interface.c
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
#include <netlink/cache.h>
#include <netlink/route/link.h>
#include <libdabba/macros.h>
#include <libdabba/strlcpy.h>
#include <libdabba/interface.h>
#include <dabbad/interface.h>
#include <libdabba-rpc/dabba.pb-c.h>

static void interface_stats_copy(struct dabba_interface_list *iflist,
				 struct rtnl_link *link)
{
	assert(iflist);
	assert(link);

	iflist->rx.byte = rtnl_link_get_stat(link, RTNL_LINK_RX_BYTES);
	iflist->rx.packet = rtnl_link_get_stat(link, RTNL_LINK_RX_PACKETS);
	iflist->rx.error = rtnl_link_get_stat(link, RTNL_LINK_RX_ERRORS);
	iflist->rx.dropped = rtnl_link_get_stat(link, RTNL_LINK_RX_DROPPED);
	iflist->rx.compressed =
	    rtnl_link_get_stat(link, RTNL_LINK_RX_COMPRESSED);
	iflist->rx_error.fifo = rtnl_link_get_stat(link, RTNL_LINK_RX_FIFO_ERR);
	iflist->rx_error.frame =
	    rtnl_link_get_stat(link, RTNL_LINK_RX_FRAME_ERR);
	iflist->rx_error.crc = rtnl_link_get_stat(link, RTNL_LINK_RX_CRC_ERR);
	iflist->rx_error.length =
	    rtnl_link_get_stat(link, RTNL_LINK_RX_LEN_ERR);
	iflist->rx_error.missed =
	    rtnl_link_get_stat(link, RTNL_LINK_RX_MISSED_ERR);
	iflist->rx_error.over = rtnl_link_get_stat(link, RTNL_LINK_RX_OVER_ERR);

	iflist->tx.byte = rtnl_link_get_stat(link, RTNL_LINK_TX_BYTES);
	iflist->tx.packet = rtnl_link_get_stat(link, RTNL_LINK_TX_PACKETS);
	iflist->tx.error = rtnl_link_get_stat(link, RTNL_LINK_TX_ERRORS);
	iflist->tx.dropped = rtnl_link_get_stat(link, RTNL_LINK_TX_DROPPED);
	iflist->tx.compressed =
	    rtnl_link_get_stat(link, RTNL_LINK_TX_COMPRESSED);
	iflist->tx_error.fifo = rtnl_link_get_stat(link, RTNL_LINK_TX_FIFO_ERR);
	iflist->tx_error.carrier =
	    rtnl_link_get_stat(link, RTNL_LINK_TX_CARRIER_ERR);
	iflist->tx_error.heartbeat =
	    rtnl_link_get_stat(link, RTNL_LINK_TX_HBEAT_ERR);
	iflist->tx_error.window =
	    rtnl_link_get_stat(link, RTNL_LINK_TX_WIN_ERR);
	iflist->tx_error.aborted =
	    rtnl_link_get_stat(link, RTNL_LINK_TX_ABORT_ERR);
}

/**
 * \brief Get the list of usable interfaces by dabbad and give it to the user
 * \param[in,out]       msg	        IPC message
 * \return 0 on success, -1 if the interface list could not be fetched.
 *
 * This function only retrieves interfaces which belong to the packet family.
 */

void interface_list(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = (struct rtnl_link *)obj;
	struct dabba_ipc_msg *msg = arg;
	struct dabba_interface_list *ifconf;
	size_t iflist_size = ARRAY_SIZE(msg->msg_body.msg.interface_list);
	uint16_t flags;

	if (msg->msg_body.elem_nr < iflist_size) {
		ifconf =
		    &msg->msg_body.msg.interface_list[msg->msg_body.elem_nr];
		strlcpy(ifconf->name, rtnl_link_get_name(link), IFNAMSIZ);
		flags = rtnl_link_get_flags(link);
		ifconf->up = (flags & IFF_UP) == IFF_UP ? TRUE : FALSE;
		ifconf->promisc =
		    (flags & IFF_PROMISC) == IFF_PROMISC ? TRUE : FALSE;
		ifconf->loopback =
		    (flags & IFF_LOOPBACK) == IFF_LOOPBACK ? TRUE : FALSE;
		interface_stats_copy(ifconf, link);
		msg->msg_body.elem_nr++;
	}
}

void interface_driver(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = (struct rtnl_link *)obj;
	struct dabba_ipc_msg *msg = arg;
	struct dabba_interface_driver *ifdriver;
	size_t ifdriver_size = ARRAY_SIZE(msg->msg_body.msg.interface_driver);

	if (msg->msg_body.elem_nr < ifdriver_size) {
		ifdriver =
		    &msg->msg_body.msg.interface_driver[msg->msg_body.elem_nr];
		strlcpy(ifdriver->name, rtnl_link_get_name(link), IFNAMSIZ);
		dev_driver_get(ifdriver->name, &ifdriver->driver_info);
		msg->msg_body.elem_nr++;
	}
}

void interface_settings(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = (struct rtnl_link *)obj;
	struct dabba_ipc_msg *msg = arg;
	struct dabba_interface_settings *ifsettings;
	size_t ifsettings_size =
	    ARRAY_SIZE(msg->msg_body.msg.interface_settings);

	if (msg->msg_body.elem_nr < ifsettings_size) {
		ifsettings =
		    &msg->msg_body.msg.interface_settings[msg->msg_body.
							  elem_nr];
		strlcpy(ifsettings->name, rtnl_link_get_name(link), IFNAMSIZ);
		dev_settings_get(ifsettings->name, &ifsettings->settings);
		ifsettings->mtu = rtnl_link_get_mtu(link);
		ifsettings->tx_qlen = rtnl_link_get_txqlen(link);
		msg->msg_body.elem_nr++;
	}
}

void interface_pause(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = (struct rtnl_link *)obj;
	struct dabba_ipc_msg *msg = arg;
	struct dabba_interface_pause *ifpause;
	size_t ifpause_size = ARRAY_SIZE(msg->msg_body.msg.interface_pause);

	if (msg->msg_body.elem_nr < ifpause_size) {
		ifpause =
		    &msg->msg_body.msg.interface_pause[msg->msg_body.elem_nr];
		strlcpy(ifpause->name, rtnl_link_get_name(link), IFNAMSIZ);
		dev_pause_get(ifpause->name, &ifpause->pause);
		msg->msg_body.elem_nr++;
	}
}

void interface_coalesce(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = (struct rtnl_link *)obj;
	struct dabba_ipc_msg *msg = arg;
	struct dabba_interface_coalesce *ifcoalesce;
	size_t ifcoalesce_size =
	    ARRAY_SIZE(msg->msg_body.msg.interface_coalesce);

	if (msg->msg_body.elem_nr < ifcoalesce_size) {
		ifcoalesce =
		    &msg->msg_body.msg.interface_coalesce[msg->msg_body.
							  elem_nr];
		strlcpy(ifcoalesce->name, rtnl_link_get_name(link), IFNAMSIZ);
		dev_coalesce_get(ifcoalesce->name, &ifcoalesce->coalesce);
		msg->msg_body.elem_nr++;
	}
}

void interface_offload(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = (struct rtnl_link *)obj;
	struct dabba_ipc_msg *msg = arg;
	struct dabba_interface_offload *ifoffload;
	size_t ifoffload_size = ARRAY_SIZE(msg->msg_body.msg.interface_offload);

	if (msg->msg_body.elem_nr < ifoffload_size) {
		ifoffload =
		    &msg->msg_body.msg.interface_offload[msg->msg_body.elem_nr];
		strlcpy(ifoffload->name, rtnl_link_get_name(link), IFNAMSIZ);

		dev_rx_csum_offload_get(ifoffload->name, &ifoffload->rx_csum);
		dev_tx_csum_offload_get(ifoffload->name, &ifoffload->tx_csum);
		dev_scatter_gather_get(ifoffload->name, &ifoffload->sg);
		dev_tcp_seg_offload_get(ifoffload->name, &ifoffload->tso);
		dev_udp_frag_offload_get(ifoffload->name, &ifoffload->ufo);
		dev_generic_seg_offload_get(ifoffload->name, &ifoffload->gso);
		dev_generic_rcv_offload_get(ifoffload->name, &ifoffload->gro);
		dev_large_rcv_offload_get(ifoffload->name, &ifoffload->lro);
		dev_rx_hash_offload_get(ifoffload->name, &ifoffload->rxhash);
		msg->msg_body.elem_nr++;
	}
}

int dabbad_interface_bulk_get(struct dabba_ipc_msg *msg,
			      void (*msg_cb) (struct nl_object * obj,
					      void *arg))
{
	struct nl_sock *sock = NULL;
	struct nl_cache *cache = NULL;
	int rc = 0;
	size_t off;

	assert(msg);
	assert(msg_cb);

	sock = nl_socket_alloc();

	if (!sock) {
		rc = ENOMEM;
		goto out;
	}

	rc = nl_connect(sock, NETLINK_ROUTE);

	if (rc)
		goto out;

	rc = rtnl_link_alloc_cache(sock, AF_UNSPEC, &cache);

	if (rc)
		goto out;

	for (off = 0; off < msg->msg_body.offset; off++)
		if (!nl_cache_is_empty(cache))
			nl_cache_remove(nl_cache_get_first(cache));

	nl_cache_foreach(cache, msg_cb, msg);

 out:
	nl_cache_free(cache);
	nl_socket_free(sock);

	return rc;
}

char *interface_list_name_get(struct dabba_ipc_msg *msg, const uint16_t index)
{
	return msg->msg_body.msg.interface_list[index].name;
}

int dabbad_interface_filter_get(struct dabba_ipc_msg *msg,
				char *(*key_cb) (struct dabba_ipc_msg * msg,
						 const uint16_t index),
				void (*msg_cb) (struct nl_object * obj,
						void *arg))
{
	struct nl_sock *sock = NULL;
	struct nl_cache *cache = NULL;
	struct rtnl_link *link;
	char *interface_name;
	int rc = 0;
	size_t a, ielem_nr;

	assert(msg);
	assert(key_cb);
	assert(msg_cb);

	sock = nl_socket_alloc();

	if (!sock) {
		rc = ENOMEM;
		goto out;
	}

	rc = nl_connect(sock, NETLINK_ROUTE);

	if (rc)
		goto out;

	rc = rtnl_link_alloc_cache(sock, AF_UNSPEC, &cache);

	if (rc)
		goto out;

	ielem_nr = msg->msg_body.elem_nr;
	msg->msg_body.elem_nr = 0;

	for (a = 0; a < ielem_nr; a++) {
		interface_name = key_cb(msg, a);

		/* Skip this object silently for now */
		if (!interface_name
		    || !(link = rtnl_link_get_by_name(cache, interface_name))) {
			msg->msg_body.elem_nr++;
			break;
		}

		msg_cb(OBJ_CAST(link), msg);
	}

 out:
	nl_cache_free(cache);
	nl_socket_free(sock);

	return rc;
}

/**
 * \brief Modify a supported interface status
 * \param[in,out]       msg	        IPC message
 * \return 0 on success, else if the interface status could not be modified.
 */

int dabbad_interface_modify(struct dabba_ipc_msg *msg)
{
	int rc;
	short flags;

	rc = dev_flags_get(msg->msg_body.msg.interface_list[0].name, &flags);

	if (rc)
		return rc;

	if (msg->msg_body.msg.interface_list[0].up == TRUE)
		flags |= IFF_UP;
	else if (msg->msg_body.msg.interface_list[0].up == FALSE)
		flags &= ~IFF_UP;

	if (msg->msg_body.msg.interface_list[0].running == TRUE)
		flags |= IFF_RUNNING;
	else if (msg->msg_body.msg.interface_list[0].running == FALSE)
		flags &= ~IFF_RUNNING;

	if (msg->msg_body.msg.interface_list[0].promisc == TRUE)
		flags |= IFF_PROMISC;
	else if (msg->msg_body.msg.interface_list[0].promisc == FALSE)
		flags &= ~IFF_PROMISC;

	return dev_flags_set(msg->msg_body.msg.interface_list[0].name, flags);
}

void __interface_id_get_all(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = (struct rtnl_link *)obj;
	Dabba__InterfaceIdList *id_listp = (Dabba__InterfaceIdList *) arg;
	size_t a;

	for (a = 0; a < id_listp->n_list; a++)
		if (!id_listp->list[a]->name) {
			id_listp->list[a]->name = rtnl_link_get_name(link);
			break;
		}
}

void dabbad_interface_id_get_all(Dabba__DabbaService_Service * service,
				 const Dabba__Dummy * dummy,
				 Dabba__InterfaceIdList_Closure closure,
				 void *closure_data)
{
	Dabba__InterfaceIdList id_list = DABBA__INTERFACE_ID_LIST__INIT;
	Dabba__InterfaceId id_init = DABBA__INTERFACE_ID__INIT;
	Dabba__InterfaceIdList *id_listp = NULL;
	struct nl_sock *sock = NULL;
	struct nl_cache *cache = NULL;
	size_t a;

	assert(service);
	assert(dummy);

	sock = nl_socket_alloc();

	if (!sock)
		goto out;

	if (nl_connect(sock, NETLINK_ROUTE))
		goto out;

	if (rtnl_link_alloc_cache(sock, AF_UNSPEC, &cache))
		goto out;

	id_list.n_list = nl_cache_nitems(cache);
	id_list.list = calloc(id_list.n_list, sizeof(*id_list.list));

	if (!id_list.list)
		goto out;

	for (a = 0; a < id_list.n_list; a++) {
		if (!(id_list.list[a] = calloc(1, sizeof(*id_list.list[a]))))
			goto out;

		*id_list.list[a] = id_init;
	}

	nl_cache_foreach(cache, __interface_id_get_all, &id_list);
	id_listp = &id_list;

 out:
	closure(id_listp, closure_data);

	for (a = 0; a < id_list.n_list; a++)
		free(id_list.list[a]);

	free(id_list.list);
	nl_cache_free(cache);
	nl_socket_free(sock);
}

void dabbad_interface_status_get_by_id(Dabba__DabbaService_Service *
				       service,
				       const Dabba__InterfaceId * idp,
				       Dabba__InterfaceStatus_Closure
				       closure, void *closure_data)
{
	Dabba__InterfaceStatus status = DABBA__INTERFACE_STATUS__INIT;
	Dabba__InterfaceId id = DABBA__INTERFACE_ID__INIT;
	Dabba__InterfaceStatus *statusp = NULL;
	struct nl_sock *sock = NULL;
	struct nl_cache *cache = NULL;
	struct rtnl_link *link;
	unsigned int flags;

	assert(service);
	assert(closure_data);

	if (!idp || !idp->name)
		goto out;

	status.id = &id;

	sock = nl_socket_alloc();

	if (!sock)
		goto out;

	if (nl_connect(sock, NETLINK_ROUTE))
		goto out;

	if (rtnl_link_alloc_cache(sock, AF_UNSPEC, &cache))
		goto out;

	link = rtnl_link_get_by_name(cache, idp->name);

	if (!link)
		goto out;

	flags = rtnl_link_get_flags(link);

	status.has_connectivity = 1;
	status.has_loopback = 1;
	status.has_promiscous = 1;
	status.has_running = 1;
	status.has_up = 1;

	dev_link_get(idp->name, (uint32_t *) & status.connectivity);
	status.loopback = (flags & IFF_LOOPBACK) == IFF_LOOPBACK;
	status.up = (flags & IFF_UP) == IFF_UP;
	status.running = (flags & IFF_RUNNING) == IFF_RUNNING;
	status.promiscous = (flags & IFF_PROMISC) == IFF_PROMISC;

	status.id->name = rtnl_link_get_name(link);
	statusp = &status;

 out:
	closure(statusp, closure_data);
	nl_cache_free(cache);
	nl_socket_free(sock);
}

void interface_status_list(struct nl_object *obj, void *arg)
{
	struct rtnl_link *link = (struct rtnl_link *)obj;
	Dabba__InterfaceStatusList *status_list = arg;
	Dabba__InterfaceStatus *status;
	uint16_t flags;
	size_t a;

	for (a = 0; a < status_list->n_list; a++) {
		status = status_list->list[a];

		if (!status->id->name) {
			status->has_connectivity = 1;
			status->has_loopback = 1;
			status->has_promiscous = 1;
			status->has_running = 1;
			status->has_up = 1;

			status->id->name = rtnl_link_get_name(link);
			flags = rtnl_link_get_flags(link);

			dev_link_get(status->id->name,
				     (uint32_t *) & status->connectivity);
			status->loopback =
			    (flags & IFF_LOOPBACK) == IFF_LOOPBACK;
			status->up = (flags & IFF_UP) == IFF_UP;
			status->running = (flags & IFF_RUNNING) == IFF_RUNNING;
			status->promiscous =
			    (flags & IFF_PROMISC) == IFF_PROMISC;

			break;
		}
	}
}

void dabbad_interface_status_get_all(Dabba__DabbaService_Service * service,
				     const Dabba__Dummy * dummy,
				     Dabba__InterfaceStatusList_Closure
				     closure, void *closure_data)
{
	Dabba__InterfaceStatusList status_list =
	    DABBA__INTERFACE_STATUS_LIST__INIT;
	Dabba__InterfaceStatusList *status_listp = NULL;
	Dabba__InterfaceStatus status_init = DABBA__INTERFACE_STATUS__INIT;
	Dabba__InterfaceId id_init = DABBA__INTERFACE_ID__INIT;
	struct nl_sock *sock = NULL;
	struct nl_cache *cache = NULL;
	size_t a;

	assert(service);
	assert(dummy);
	assert(closure_data);

	sock = nl_socket_alloc();

	if (!sock)
		goto out;

	if (nl_connect(sock, NETLINK_ROUTE))
		goto out;

	if (rtnl_link_alloc_cache(sock, AF_UNSPEC, &cache))
		goto out;

	status_list.n_list = nl_cache_nitems(cache);
	status_list.list =
	    calloc(status_list.n_list, sizeof(*status_list.list));

	if (!status_list.list)
		goto out;

	for (a = 0; a < status_list.n_list; a++) {
		status_list.list[a] = calloc(1, sizeof(*status_list.list[a]));

		if (!status_list.list[a])
			goto out;

		*status_list.list[a] = status_init;

		status_list.list[a]->id =
		    calloc(1, sizeof(*status_list.list[a]->id));

		if (!status_list.list[a]->id)
			goto out;

		*status_list.list[a]->id = id_init;
	}

	nl_cache_foreach(cache, interface_status_list, &status_list);
	status_listp = &status_list;

 out:
	closure(status_listp, closure_data);
	for (a = 0; a < status_list.n_list; a++) {
		free(status_list.list[a]->id);
		free(status_list.list[a]);
	}

	free(status_list.list);
	nl_cache_free(cache);
	nl_socket_free(sock);
}
