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

int rpc_interface_coalesce_get(ProtobufCService * service,
			       const Dabba__InterfaceIdList * id_list)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(id_list);

	dabba__dabba_service__interface_coalesce_get(service, id_list,
						     interface_coalesce_list_print,
						     &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

int rpc_interface_coalesce_modify(ProtobufCService * service,
				  const Dabba__InterfaceCoalesce * coalescep)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(coalescep);

	dabba__dabba_service__interface_coalesce_modify(service, coalescep,
							rpc_dummy_print,
							&is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

int cmd_interface_coalesce_modify(int argc, const char **argv)
{
	enum interface_option {
		OPT_PACKET_RATE_HIGH,
		OPT_PACKET_RATE_LOW,
		OPT_RATE_SAMPLE_INTERVAL,
		OPT_STATS_BLOCK,
		OPT_INTERFACE_ID,
		OPT_TCP,
		OPT_LOCAL,
		OPT_HELP
	};

	const struct option interface_option[] = {
		{"packet-rate-high", required_argument, NULL,
		 OPT_PACKET_RATE_HIGH},
		{"packet-rate-low", required_argument, NULL,
		 OPT_PACKET_RATE_LOW},
		{"packet-rate-interval", required_argument, NULL,
		 OPT_RATE_SAMPLE_INTERVAL},
		{"stats-block", required_argument, NULL, OPT_STATS_BLOCK},
		{"id", required_argument, NULL, OPT_INTERFACE_ID},
		{"tcp", optional_argument, NULL, OPT_TCP},
		{"local", optional_argument, NULL, OPT_LOCAL},
		{"help", required_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0},
	};

	int ret, rc = 0;
	const char *server_id = DABBA_RPC_DEFAULT_LOCAL_SERVER_NAME;
	ProtobufC_RPC_AddressType server_type = PROTOBUF_C_RPC_ADDRESS_LOCAL;
	ProtobufCService *service;
	Dabba__InterfaceCoalesce coalesce = DABBA__INTERFACE_COALESCE__INIT;

	/* HACK: getopt*() start to parse options at argv[1] */
	argc++;
	argv--;

	while ((ret =
		getopt_long_only(argc, (char **)argv, "", interface_option,
				 NULL)) != EOF) {
		switch (ret) {
		case OPT_PACKET_RATE_HIGH:
			coalesce.pkt_rate_high = strtoul(optarg, NULL, 10);
			coalesce.has_pkt_rate_high = 1;
			break;
		case OPT_PACKET_RATE_LOW:
			coalesce.pkt_rate_low = strtoul(optarg, NULL, 10);
			coalesce.has_pkt_rate_low = 1;
			break;
		case OPT_RATE_SAMPLE_INTERVAL:
			coalesce.rate_sample_interval =
			    strtoul(optarg, NULL, 10);
			coalesce.has_rate_sample_interval = 1;
			break;
		case OPT_TCP:
			server_type = PROTOBUF_C_RPC_ADDRESS_TCP;
			server_id = DABBA_RPC_DEFAULT_TCP_SERVER_NAME;

			if (optarg)
				server_id = optarg;
			break;
		case OPT_LOCAL:
			server_type = PROTOBUF_C_RPC_ADDRESS_LOCAL;
			server_id = DABBA_RPC_DEFAULT_LOCAL_SERVER_NAME;

			if (optarg)
				server_id = optarg;
			break;
		case OPT_INTERFACE_ID:
			coalesce.id = malloc(sizeof(*coalesce.id));

			if (!coalesce.id)
				return ENOMEM;

			dabba__interface_id__init(coalesce.id);
			coalesce.id->name = optarg;
			break;
		case OPT_HELP:
		default:
			show_usage(interface_option);
			rc = -1;
			goto out;
		}
	}

	service = dabba_rpc_client_connect(server_id, server_type);

	if (service)
		rc = rpc_interface_coalesce_modify(service, &coalesce);
	else
		rc = EINVAL;
 out:
	free(coalesce.id);
	return rc;
}
