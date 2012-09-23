/**
 * \file nic.h
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

struct libdabba_interface_offload {
	uint8_t rx_csum, tx_csum, sg, tso, ufo, gso, gro, lro, ntuple, rxhash;
};

struct ethtool_drvinfo;
struct ethtool_cmd;
struct ethtool_pauseparam;
struct ethtool_coalesce;

int devname_to_ifindex(const char *const dev, int *index);
int ifindex_to_devname(const int index, char *dev, size_t dev_len);
int dev_flags_get(const char *const dev, short *flags);
int dev_flags_set(const char *const dev, const short flags);
int dev_driver_get(const char *const dev, struct ethtool_drvinfo *driver_info);
int dev_settings_get(const char *const dev, struct ethtool_cmd *settings);
int dev_pause_get(const char *const dev, struct ethtool_pauseparam *pause);
int dev_coalesce_get(const char *const dev, struct ethtool_coalesce *coalesce);
int dev_offload_get(const char *const dev,
		    struct libdabba_interface_offload *offload);

#endif				/* NIC_H */
