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
#include <dabbad/dabbad.h>
#include <dabba/macros.h>
#include <dabba/rpc.h>
#include <dabba/help.h>

static void display_interface_statistics_header(void)
{
	printf("---\n");
	printf("  interfaces:\n");
}

static void interface_statistics_list_print(const Dabba__InterfaceStatisticsList
					    * result, void *closure_data)
{
	Dabba__InterfaceStatistics *statisticsp;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;
	size_t a;

	assert(closure_data);

	display_interface_statistics_header();

	for (a = 0; result && a < result->n_list; a++) {
		statisticsp = result->list[a];
		printf("    - name: %s\n", statisticsp->id->name);
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

static int prepare_interface_statistics_query(int argc, char **argv,
					      char **server_name,
					      Dabba__InterfaceIdList * list)
{
	enum interface_statistics_option {
		OPT_SERVER_ID,
		OPT_INTERFACE_ID,
		OPT_HELP,
	};
	const struct option interface_statistics_option[] = {
		{"id", required_argument, NULL, OPT_INTERFACE_ID},
		{"server", required_argument, NULL, OPT_SERVER_ID},
		{NULL, 0, NULL, 0},
	};
	int ret, rc = 0;
	Dabba__InterfaceId **idpp;

	assert(list);

	while ((ret =
		getopt_long_only(argc, argv, "", interface_statistics_option,
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
			show_usage(interface_statistics_option);
			rc = -1;
			break;
		}
	}

	return rc;
}

/**
 * \brief Get current interface list information and output them on \c stdout
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 *
 * This function prepares an IPC message to query the supported network
 * interfaces present on the system. Once the message is sent, it waits for the
 * dabba daemon to reply.
 */

int cmd_interface_statistics(int argc, const char **argv)
{
	ProtobufCService *service;
	protobuf_c_boolean is_done = 0;
	Dabba__InterfaceIdList id_list = DABBA__INTERFACE_ID_LIST__INIT;
	char *server_name = NULL;
	size_t a;
	int rc;

	assert(argc >= 0);
	assert(argv);

	rc = prepare_interface_statistics_query(argc, (char **)argv,
						&server_name, &id_list);

	if (rc)
		goto out;

	service = dabba_rpc_client_connect(server_name);

	dabba__dabba_service__interface_statistics_get(service, &id_list,
						       interface_statistics_list_print,
						       &is_done);

	dabba_rpc_call_is_done(&is_done);

 out:
	for (a = 0; a < id_list.n_list; a++)
		free(id_list.list[a]);

	free(id_list.list);

	return rc;
}
