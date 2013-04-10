/**
 * \file interface.h
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2011
 * \date 2011
 */

/* __LICENSE_HEADER_BEGIN__ */

/*
 * Copyright (C) 2011	Emmanuel Roullit <emmanuel.roullit@gmail.com>
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

#ifndef NIC_H
#define	NIC_H

#include <stdint.h>

/**
 * \brief Pseudo-interface name to indicate that all interfaces must be used
 */

#ifndef ANY_INTERFACE
#define ANY_INTERFACE "any"
#endif				/* ANY_INTERFACE */

struct ethtool_drvinfo;
struct ethtool_cmd;
struct ethtool_pauseparam;
struct ethtool_coalesce;

int devname_to_ifindex(const char *const dev, int *index);
int ifindex_to_devname(const int index, char *dev, size_t dev_len);
int dev_tx_queue_len_get(const char *const dev, uint32_t * txqlen);
int dev_flags_get(const char *const dev, short *flags);
int dev_flags_set(const char *const dev, const short flags);
int dev_driver_get(const char *const dev, struct ethtool_drvinfo *driver);
int dev_settings_get(const char *const dev, struct ethtool_cmd *settings);
int dev_settings_set(const char *const dev, struct ethtool_cmd *settings);
int dev_pause_get(const char *const dev, struct ethtool_pauseparam *pause);
int dev_pause_set(const char *const dev, struct ethtool_pauseparam *pause);
int dev_coalesce_get(const char *const dev, struct ethtool_coalesce *coalesce);
int dev_coalesce_set(const char *const dev, struct ethtool_coalesce *coalesce);
int dev_rx_csum_offload_get(const char *const dev, int *rx_csum);
int dev_rx_csum_offload_set(const char *const dev, int rx_csum);
int dev_tx_csum_offload_get(const char *const dev, int *tx_csum);
int dev_tx_csum_offload_set(const char *const dev, int tx_csum);
int dev_scatter_gather_get(const char *const dev, int *sg);
int dev_scatter_gather_set(const char *const dev, int sg);
int dev_tcp_seg_offload_get(const char *const dev, int *tso);
int dev_tcp_seg_offload_set(const char *const dev, int tso);
int dev_udp_frag_offload_get(const char *const dev, int *ufo);
int dev_udp_frag_offload_set(const char *const dev, int ufo);
int dev_generic_seg_offload_get(const char *const dev, int *gso);
int dev_generic_seg_offload_set(const char *const dev, int gso);
int dev_generic_rcv_offload_get(const char *const dev, int *gro);
int dev_generic_rcv_offload_set(const char *const dev, int gro);
int dev_large_rcv_offload_get(const char *const dev, int *lro);
int dev_large_rcv_offload_set(const char *const dev, int lro);
int dev_rx_hash_offload_get(const char *const dev, int *rxhash);
int dev_rx_hash_offload_set(const char *const dev, int rxhash);

int dev_link_get(const char *const dev, int *link);

#endif				/* NIC_H */
