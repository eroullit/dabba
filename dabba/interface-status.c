/**
 * \file interface-status.c
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
#include <dabba/cli.h>
#include <dabba/rpc.h>
#include <dabba/help.h>

static void display_interface_status_header(void)
{
	printf("---\n");
	printf("  interfaces:\n");
}

static void interface_status_list_print(const Dabba__InterfaceStatusList *
					result, void *closure_data)
{
	Dabba__InterfaceStatus *statusp;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;
	size_t a;

	assert(closure_data);

	display_interface_status_header();

	for (a = 0; result && a < result->n_list; a++) {
		statusp = result->list[a];
		printf("    - name: %s\n", statusp->id->name);
		printf("      status: {");
		printf("connectivity: %s, ", print_tf(statusp->connectivity));
		printf("up: %s, ", print_tf(statusp->up));
		printf("running: %s, ", print_tf(statusp->running));
		printf("promiscuous: %s, ", print_tf(statusp->promiscous));
		printf("loopback: %s", print_tf(statusp->loopback));
		printf("}\n");
	}

	*status = 1;
}

int rpc_interface_status_get(ProtobufCService * service,
			     const Dabba__InterfaceIdList * id_list)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(id_list);

	dabba__dabba_service__interface_status_get(service, id_list,
						   interface_status_list_print,
						   &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

int rpc_interface_status_modify(ProtobufCService * service,
				const Dabba__InterfaceStatus * statusp)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(statusp);

	dabba__dabba_service__interface_status_modify(service, statusp,
						      rpc_dummy_print,
						      &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

int cmd_interface_status_modify(int argc, const char **argv)
{
	enum interface_option {
		/* option */
		OPT_INTERFACE_PROMISCUOUS,
		OPT_INTERFACE_UP,
		OPT_INTERFACE_ID,
		OPT_TCP,
		OPT_LOCAL,
		OPT_HELP
	};

	const struct option interface_option[] = {
		{"id", required_argument, NULL, OPT_INTERFACE_ID},
		{"promiscuous", required_argument, NULL,
		 OPT_INTERFACE_PROMISCUOUS},
		{"up", required_argument, NULL,
		 OPT_INTERFACE_UP},
		{"tcp", optional_argument, NULL, OPT_TCP},
		{"local", optional_argument, NULL, OPT_LOCAL},
		{"help", required_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0},
	};

	int ret, rc = 0;
	const char *server_id = DABBA_RPC_DEFAULT_LOCAL_SERVER_NAME;
	ProtobufC_RPC_AddressType server_type = PROTOBUF_C_RPC_ADDRESS_LOCAL;
	ProtobufCService *service;
	Dabba__InterfaceStatus status = DABBA__INTERFACE_STATUS__INIT;

	while ((ret =
		getopt_long_only(argc, (char **)argv, "", interface_option,
				 NULL)) != EOF) {
		switch (ret) {
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
			status.id = malloc(sizeof(*status.id));

			if (!status.id)
				return ENOMEM;

			dabba__interface_id__init(status.id);
			status.id->name = optarg;
			break;

		case OPT_INTERFACE_PROMISCUOUS:
			rc = str2bool(optarg, &status.promiscous);

			if (rc)
				goto out;

			status.has_promiscous = 1;
			break;
		case OPT_INTERFACE_UP:
			rc = str2bool(optarg, &status.up);

			if (rc)
				goto out;

			status.has_up = 1;
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
		rc = rpc_interface_status_modify(service, &status);
	else
		rc = EINVAL;
 out:
	free(status.id);
	return rc;
}
