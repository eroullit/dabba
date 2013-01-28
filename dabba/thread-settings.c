/**
 * \file thread-settings.c
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
const char *thread_type2str(const int type);

static void thread_settings_print(const Dabba__ThreadList * result,
				  void *closure_data)
{
	size_t a;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;

	rpc_header_print("threads");

	for (a = 0; result && a < result->n_list; a++) {
		printf("    - id: %" PRIu64 "\n", result->list[a]->id->id);
		printf("      type: %s\n",
		       thread_type2str(result->list[a]->type));
		printf("      scheduling policy: %s\n",
		       sched_policy2str(result->list[a]->sched_policy));
		printf("      scheduling priority: %i\n",
		       result->list[a]->sched_priority);
		printf("      cpu affinity: %s\n", result->list[a]->cpu_set);
		/* TODO map priority/policy protobuf enums to string */
	}

	*status = 1;
}

static void thread_dummy_print(const Dabba__Dummy * result, void *closure_data)
{
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;

	assert(result);
	assert(closure_data);

	*status = 1;
}

int rpc_thread_settings_get(ProtobufCService * service,
			    const Dabba__ThreadIdList * id_list)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(id_list);

	dabba__dabba_service__thread_get(service, id_list,
					 thread_settings_print, &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

int rpc_thread_settings_modify(ProtobufCService * service,
			       const Dabba__Thread * thread)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(thread);

	dabba__dabba_service__thread_modify(service, thread, thread_dummy_print,
					    &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}
