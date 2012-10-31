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
#include <getopt.h>
#include <inttypes.h>
#include <assert.h>
#include <errno.h>
#include <dabbad/dabbad.h>
#include <dabba/macros.h>
#include <dabba/ipc.h>
#include <dabba/rpc.h>
#include <dabba/help.h>

static void display_interface_status_header(void)
{
	printf("---\n");
	printf("  interfaces:\n");
}

static void interface_status_print(const Dabba__InterfaceStatus * result,
				   void *closure_data)
{
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;

	assert(closure_data);

	display_interface_status_header();

	if (result && result->id) {
		printf("    - name: %s\n", result->id->name);
		printf("      status: {");
		printf("connectivity: %s, ", print_tf(result->connectivity));
		printf("up: %s, ", print_tf(result->up));
		printf("running: %s, ", print_tf(result->running));
		printf("promiscuous: %s, ", print_tf(result->promiscous));
		printf("loopback: %s", print_tf(result->loopback));
		printf("}\n");
	}

	*status = 1;
}

static int prepare_interface_status_query(int argc, char **argv,
					  char **server_name,
					  Dabba__InterfaceId * id)
{
	enum interface_status_option {
		OPT_SERVER_ID,
		OPT_INTERFACE_ID,
		OPT_HELP,
	};

	const struct option interface_status_option[] = {
		{"id", required_argument, NULL, OPT_INTERFACE_ID},
		{"server", required_argument, NULL, OPT_SERVER_ID},
		{NULL, 0, NULL, 0},
	};
	int ret, rc = 0;

	while ((ret =
		getopt_long_only(argc, argv, "", interface_status_option,
				 NULL)) != EOF) {
		switch (ret) {
		case OPT_SERVER_ID:
			*server_name = optarg;
			break;
		case OPT_INTERFACE_ID:
			id->name = optarg;
			break;
		case OPT_HELP:
		default:
			show_usage(interface_status_option);
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

int cmd_interface_status(int argc, const char **argv)
{
	ProtobufCService *service;
	protobuf_c_boolean is_done = 0;
	Dabba__InterfaceId id = DABBA__INTERFACE_ID__INIT;
	char *server_name = NULL;
	int rc;

	assert(argc >= 0);
	assert(argv);

	rc = prepare_interface_status_query(argc, (char **)argv, &server_name,
					    &id);

	if (rc)
		return rc;

	if (!id.name)
		return EINVAL;

	service = dabba_rpc_client_connect(server_name);

	dabba__dabba_service__interface_status_get_by_id(service, &id,
							 interface_status_print,
							 &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}
