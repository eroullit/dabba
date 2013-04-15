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

/**
 * \internal
 * \brief Protobuf closure to print thread settings list in YAML
 * \param[in]           result	        Pointer to thread settings list
 * \param[in]           closure_data	Pointer to protobuf closure data
 */

static void thread_settings_print(const Dabba__ThreadList * result,
				  void *closure_data)
{
	const Dabba__Thread *thread;
	size_t a;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;

	assert(closure_data);

	rpc_header_print("threads");

	for (a = 0; result && a < result->n_list; a++) {
		thread = result->list[a];
		printf("    - id: %" PRIu64 "\n", thread->id->id);
		printf("    ");
		__rpc_error_code_print(thread->status->code);
		printf("      type: %s\n", thread_type2str(thread->type));
		printf("      scheduling policy: %s\n",
		       sched_policy2str(thread->sched_policy));
		printf("      scheduling priority: %i\n",
		       thread->sched_priority);
		printf("      cpu affinity: %s\n", thread->cpu_set);
		/* TODO map priority/policy protobuf enums to string */
	}

	*status = 1;
}

/**
 * \brief Invoke thread settings get RPC
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           id_list         Pointer to thread id to fetch
 * \return Always returns 0.
 * \note An empty id list will query the status of all available thread.
 */

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

/**
 * \brief Invoke thread settings modify RPC
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           status          Pointer to thread new status settings
 * \return Always returns 0.
 */

int rpc_thread_settings_modify(ProtobufCService * service,
			       const Dabba__Thread * thread)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(thread);

	dabba__dabba_service__thread_modify(service, thread,
					    rpc_error_code_print, &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}
