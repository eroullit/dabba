/**
 * \file interface-statistics.c
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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <inttypes.h>
#include <assert.h>
#include <errno.h>
#include <dabba/macros.h>
#include <dabba/rpc.h>
#include <dabba/help.h>

/**
 * \internal
 * \brief Protobuf closure to print interface statistics list in YAML
 * \param[in]           result	        Pointer to interface statistics list
 * \param[in]           closure_data	Pointer to protobuf closure data
 */

static void interface_statistics_list_print(const Dabba__InterfaceStatisticsList
					    * result, void *closure_data)
{
	Dabba__InterfaceStatistics *statisticsp;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;
	size_t a;

	assert(closure_data);

	rpc_header_print("interfaces");

	for (a = 0; result && a < result->n_list; a++) {
		statisticsp = result->list[a];
		printf("    - name: %s\n", statisticsp->id->name);
		printf("    ");
		__rpc_error_code_print(statisticsp->status->code);
		printf("      statistics:\n");
		printf("          rx: {");
		printf("byte: %" PRIu64 ", ", statisticsp->rx_byte);
		printf("packet: %" PRIu64 ", ", statisticsp->rx_packet);
		printf("error: %" PRIu64 ", ", statisticsp->rx_error);
		printf("dropped: %" PRIu64 ", ", statisticsp->rx_dropped);
		printf("compressed: %" PRIu64 "", statisticsp->rx_compressed);
		printf("}\n");
		printf("          tx: {");
		printf("byte: %" PRIu64 ", ", statisticsp->tx_byte);
		printf("packet: %" PRIu64 ", ", statisticsp->tx_packet);
		printf("error: %" PRIu64 ", ", statisticsp->tx_error);
		printf("dropped: %" PRIu64 ", ", statisticsp->tx_dropped);
		printf("compressed: %" PRIu64 "", statisticsp->tx_compressed);
		printf("}\n");
		printf("          rx error: {");
		printf("fifo: %" PRIu64 ", ", statisticsp->rx_error_fifo);
		printf("frame: %" PRIu64 ", ", statisticsp->rx_error_frame);
		printf("crc: %" PRIu64 ", ", statisticsp->rx_error_crc);
		printf("length: %" PRIu64 ", ", statisticsp->rx_error_length);
		printf("missed: %" PRIu64 ", ", statisticsp->rx_error_missed);
		printf("overflow: %" PRIu64 "", statisticsp->rx_error_over);
		printf("}\n");
		printf("          tx error: {");
		printf("fifo: %" PRIu64 ", ", statisticsp->tx_error_fifo);
		printf("carrier: %" PRIu64 ", ", statisticsp->tx_error_carrier);
		printf("heartbeat: %" PRIu64 ", ",
		       statisticsp->tx_error_heartbeat);
		printf("window: %" PRIu64 ", ", statisticsp->tx_error_window);
		printf("aborted: %" PRIu64 "", statisticsp->tx_error_aborted);
		printf("}\n");
	}

	*status = 1;
}

/**
 * \brief Invoke interface statistics get RPC
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           id_li
 * st         Pointer to interface id to fetch
 * \return Always returns 0.
 * \note An empty id list will query the statistics counters of all available interfaces.
 */

int rpc_interface_statistics_get(ProtobufCService * service,
				 const Dabba__InterfaceIdList * id_list)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(id_list);

	dabba__dabba_service__interface_statistics_get(service, id_list,
						       interface_statistics_list_print,
						       &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}
