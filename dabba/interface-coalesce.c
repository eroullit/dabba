/**
 * \file interface-coalesce.c
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
#include <assert.h>
#include <errno.h>

#include <dabbad/dabbad.h>
#include <dabba/rpc.h>
#include <dabba/help.h>
#include <dabba/macros.h>

static void display_interface_coalesce_header(void)
{
	printf("---\n");
	printf("  interfaces:\n");
}

static void interface_coalesce_list_print(const Dabba__InterfaceCoalesceList *
					  result, void *closure_data)
{
	Dabba__InterfaceCoalesce *coalescep;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;
	size_t a;

	assert(closure_data);

	display_interface_coalesce_header();

	for (a = 0; result && a < result->n_list; a++) {
		coalescep = result->list[a];
		printf("    - name: %s\n", coalescep->id->name);
		printf("      coalesce:\n");
		printf("        packet rate high: %u\n",
		       coalescep->pkt_rate_high);
		printf("        packet rate low: %u\n",
		       coalescep->pkt_rate_low);
		printf("        rate sample interval: %u\n",
		       coalescep->rate_sample_interval);
		printf("        stats block: %u\n",
		       coalescep->stats_block_coalesce_usecs);
		printf("        rx:\n");
		printf("            adaptive: %s\n",
		       print_tf(coalescep->use_adaptive_rx_coalesce));
		printf("            usec: {");
		printf("normal: %u, ", coalescep->rx_coalesce_usecs);
		printf("irq: %u, ", coalescep->rx_coalesce_usecs_irq);
		printf("high: %u, ", coalescep->rx_coalesce_usecs_high);
		printf("low: %u", coalescep->rx_coalesce_usecs_low);
		printf("}\n");
		printf("            max frame: {");
		printf("normal: %u, ", coalescep->rx_max_coalesced_frames);
		printf("irq: %u, ", coalescep->rx_max_coalesced_frames_irq);
		printf("high: %u, ", coalescep->rx_max_coalesced_frames_high);
		printf("low: %u", coalescep->rx_max_coalesced_frames_low);
		printf("}\n");
		printf("        tx:\n");
		printf("            adaptive: %s\n",
		       print_tf(coalescep->use_adaptive_tx_coalesce));
		printf("            usec: {");
		printf("normal: %u, ", coalescep->tx_coalesce_usecs);
		printf("irq: %u, ", coalescep->tx_coalesce_usecs_irq);
		printf("high: %u, ", coalescep->tx_coalesce_usecs_high);
		printf("low: %u", coalescep->tx_coalesce_usecs_low);
		printf("}\n");
		printf("            max frame: {");
		printf("normal: %u, ", coalescep->tx_max_coalesced_frames);
		printf("irq: %u, ", coalescep->tx_max_coalesced_frames_irq);
		printf("high: %u, ", coalescep->tx_max_coalesced_frames_high);
		printf("low: %u", coalescep->tx_max_coalesced_frames_low);
		printf("}\n");
	}

	*status = 1;
}

int rpc_interface_coalesce_get(const char *const server_id,
			       const Dabba__InterfaceIdList * id_list)
{
	ProtobufCService *service;
	protobuf_c_boolean is_done = 0;

	service = dabba_rpc_client_connect(server_id);

	dabba__dabba_service__interface_coalesce_get(service, id_list,
						     interface_coalesce_list_print,
						     &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}
