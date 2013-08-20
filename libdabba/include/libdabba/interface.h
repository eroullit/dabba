/**
 * \file interface.h
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */


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

int ldab_devname_to_ifindex(const char *const dev, int *index);
int ldab_ifindex_to_devname(const int index, char *dev, size_t dev_len);
int dev_tx_queue_len_get(const char *const dev, uint32_t * txqlen);
int ldab_dev_flags_get(const char *const dev, short *flags);
int ldab_dev_flags_set(const char *const dev, const short flags);
int ldab_dev_driver_get(const char *const dev, struct ethtool_drvinfo *driver);
int ldab_dev_settings_get(const char *const dev, struct ethtool_cmd *settings);
int ldab_dev_settings_set(const char *const dev, struct ethtool_cmd *settings);
int ldab_dev_pause_get(const char *const dev, struct ethtool_pauseparam *pause);
int ldab_dev_pause_set(const char *const dev, struct ethtool_pauseparam *pause);
int ldab_dev_coalesce_get(const char *const dev,
			 struct ethtool_coalesce *coalesce);
int ldab_dev_coalesce_set(const char *const dev,
			 struct ethtool_coalesce *coalesce);
int ldab_dev_rx_csum_offload_get(const char *const dev, int *rx_csum);
int ldab_dev_rx_csum_offload_set(const char *const dev, int rx_csum);
int ldab_dev_tx_csum_offload_get(const char *const dev, int *tx_csum);
int ldab_dev_tx_csum_offload_set(const char *const dev, int tx_csum);
int ldab_dev_scatter_gather_get(const char *const dev, int *sg);
int ldab_dev_scatter_gather_set(const char *const dev, int sg);
int ldab_dev_tcp_seg_offload_get(const char *const dev, int *tso);
int ldab_dev_tcp_seg_offload_set(const char *const dev, int tso);
int ldab_dev_udp_frag_offload_get(const char *const dev, int *ufo);
int ldab_dev_udp_frag_offload_set(const char *const dev, int ufo);
int ldab_dev_generic_seg_offload_get(const char *const dev, int *gso);
int ldab_dev_generic_seg_offload_set(const char *const dev, int gso);
int ldab_dev_generic_rcv_offload_get(const char *const dev, int *gro);
int ldab_dev_generic_rcv_offload_set(const char *const dev, int gro);
int ldab_dev_large_rcv_offload_get(const char *const dev, int *lro);
int ldab_dev_large_rcv_offload_set(const char *const dev, int lro);
int ldab_dev_rx_hash_offload_get(const char *const dev, int *rxhash);
int ldab_dev_rx_hash_offload_set(const char *const dev, int rxhash);

int ldab_dev_link_get(const char *const dev, int *link);

#endif				/* NIC_H */
