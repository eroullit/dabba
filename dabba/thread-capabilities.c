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

static void thread_capabilities_print(const Dabba__ThreadCapabilitiesList *
				      result, void *closure_data)
{
	const Dabba__ThreadCapabilities *cap;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;
	size_t a;

	rpc_header_print("threads capabilities");

	for (a = 0; a < result->n_list; a++) {
		cap = result->list[a];
		printf("    %s:\n", sched_policy2str(cap->policy));
		printf("        scheduling priority:\n");
		printf("            minimum: %i\n", cap->prio_min);
		printf("            maximum: %i\n", cap->prio_max);
	}

	*status = 1;
}

int rpc_thread_capabilities_get(const char *const server_id,
				const Dabba__ThreadIdList * id_list)
{
	ProtobufCService *service;
	protobuf_c_boolean is_done = 0;
	Dabba__Dummy dummy = DABBA__DUMMY__INIT;

	assert(id_list);

	service = dabba_rpc_client_connect(server_id);

	dabba__dabba_service__thread_capabilities_get(service, &dummy,
						      thread_capabilities_print,
						      &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}
