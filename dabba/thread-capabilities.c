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
#include <dabba/rpc.h>

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
 * \brief Invoke thread capabilities get RPC
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           id_list         Pointer to thread id to fetch
 * \return Always returns 0.
 * \note An empty id list will query the status of all available thread.
 */

int rpc_thread_capabilities_get(ProtobufCService * service,
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
