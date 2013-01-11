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

static int prepare_interface_coalesce_query(int argc, char **argv,
					    char **server_name,
					    Dabba__InterfaceIdList * list)
{
	enum interface_coalesce_option {
		OPT_SERVER_ID,
		OPT_INTERFACE_ID,
		OPT_HELP,
	};
	const struct option interface_coalesce_option[] = {
		{"id", required_argument, NULL, OPT_INTERFACE_ID},
		{"server", required_argument, NULL, OPT_SERVER_ID},
		{NULL, 0, NULL, 0},
	};
	int ret, rc = 0;
	Dabba__InterfaceId **idpp;

	assert(list);

	while ((ret =
		getopt_long_only(argc, argv, "", interface_coalesce_option,
				 NULL)) != EOF) {
		switch (ret) {
		case OPT_SERVER_ID:
			*server_name = optarg;
			break;
		case OPT_INTERFACE_ID:
			idpp =
			    realloc(list->list,
				    sizeof(*list->list) * (list->n_list + 1));

			if (!idpp)
				return ENOMEM;

			list->list = idpp;
			list->list[list->n_list] =
			    malloc(sizeof(*list->list[list->n_list]));

			if (!list->list[list->n_list])
				return ENOMEM;

			dabba__interface_id__init(list->list[list->n_list]);

			list->list[list->n_list]->name = optarg;
			list->n_list++;

			break;
		case OPT_HELP:
		default:
			show_usage(interface_coalesce_option);
			rc = -1;
			break;
		}
	}

	return rc;
}

/**
 * \brief Get interface coalesce parameters and output them on \c stdout
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 */

int cmd_interface_coalesce(int argc, const char **argv)
{
	ProtobufCService *service;
	protobuf_c_boolean is_done = 0;
	Dabba__InterfaceIdList id_list = DABBA__INTERFACE_ID_LIST__INIT;
	char *server_name = NULL;
	size_t a;
	int rc;

	assert(argc >= 0);
	assert(argv);

	rc = prepare_interface_coalesce_query(argc, (char **)argv, &server_name,
					      &id_list);

	if (rc)
		goto out;

	service = dabba_rpc_client_connect(server_name);

	dabba__dabba_service__interface_coalesce_get(service, &id_list,
						     interface_coalesce_list_print,
						     &is_done);

	dabba_rpc_call_is_done(&is_done);

 out:
	for (a = 0; a < id_list.n_list; a++)
		free(id_list.list[a]);

	free(id_list.list);

	return rc;
}
