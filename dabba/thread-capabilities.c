/**
 * \file thread-capabilities.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */

/* __LICENSE_HEADER_BEGIN__ */

/*
 * Copyright (C) 2013	Emmanuel Roullit <emmanuel.roullit@gmail.com>
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
#include <errno.h>
#include <libdabba/macros.h>
#include <dabba/rpc.h>
#include <dabba/help.h>
#include <dabba/dabba.h>

const char *sched_policy2str(const int policy_value);

/**
 * \internal
 * \brief Protobuf closure to print thread capabilities list in YAML
 * \param[in]           result	        Pointer to thread capabilities list
 * \param[in]           closure_data	Pointer to protobuf closure data
 */

static void thread_capabilities_print(const Dabba__ThreadCapabilitiesList *
				      result, void *closure_data)
{
	const Dabba__ThreadCapabilities *cap;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;
	size_t a;

	assert(closure_data);

	rpc_header_print("thread capabilities");

	for (a = 0; result && a < result->n_list; a++) {
		cap = result->list[a];
		printf("    %s:\n", sched_policy2str(cap->policy));
		printf("    ");
		__rpc_error_code_print(cap->status->code);
		printf("        scheduling priority:\n");
		printf("            minimum: %i\n", cap->prio_min);
		printf("            maximum: %i\n", cap->prio_max);
	}

	*status = 1;
}

/**
 * \internal
 * \brief Invoke thread capabilities get RPC
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           id_list         Pointer to thread id to fetch
 * \return Always returns 0.
 * \note An empty id list will query the status of all available thread.
 */

static int rpc_thread_capabilities_get(ProtobufCService * service,
				       const Dabba__ThreadIdList * id_list)
{
	protobuf_c_boolean is_done = 0;
	Dabba__Dummy dummy = DABBA__DUMMY__INIT;

	assert(service);
	assert(id_list);

	dabba__dabba_service__thread_capabilities_get(service, &dummy,
						      thread_capabilities_print,
						      &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

static int cmd_thread_capabilities_get(int argc, const char **argv)
{
	enum thread_option {
		/* option */
		OPT_TCP,
		OPT_LOCAL,
		OPT_HELP
	};

	const struct option thread_option[] = {
		{"tcp", optional_argument, NULL, OPT_TCP},
		{"local", optional_argument, NULL, OPT_LOCAL},
		{"help", no_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0},
	};

	int ret, rc = 0;
	Dabba__ThreadIdList id_list = DABBA__THREAD_ID_LIST__INIT;
	const char *server_id = DABBA_RPC_DEFAULT_LOCAL_SERVER_NAME;
	ProtobufC_RPC_AddressType server_type = PROTOBUF_C_RPC_ADDRESS_LOCAL;
	ProtobufCService *service;

	/* HACK: getopt*() start to parse options at argv[1] */
	argc++;
	argv--;

	/* parse options and actions to run */
	while ((ret =
		getopt_long_only(argc, (char **)argv, "", thread_option,
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
		case OPT_HELP:
		default:
			show_usage(thread_option);
			rc = -1;
			goto out;
		}
	}

	service = dabba_rpc_client_connect(server_id, server_type);

	if (!service)
		return EINVAL;

	rpc_thread_capabilities_get(service, &id_list);
 out:
	return rc;

}

int cmd_thread_capabilities(int argc, const char **argv)
{
	static const struct cmd_struct cmd[] = {
		{"get", cmd_thread_capabilities_get}
	};

	return cmd_run_builtin(cmd, ARRAY_SIZE(cmd), argc, argv);
}
