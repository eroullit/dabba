/**
 * \file rpc.c
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
#include <unistd.h>
#include <dabba/rpc.h>

ProtobufCService *dabba_rpc_client_connect(const char *const name,
					   const ProtobufC_RPC_AddressType type)
{
	/* The first failure triggers a instantaneous retry, we want 10 retry */
	size_t a, nretry = 11;
	ProtobufCService *service;
	ProtobufC_RPC_Client *client;

	assert(name);
	assert(type == PROTOBUF_C_RPC_ADDRESS_LOCAL
	       || type == PROTOBUF_C_RPC_ADDRESS_TCP);

	service =
	    protobuf_c_rpc_client_new(type, name,
				      &dabba__dabba_service__descriptor, NULL);

	if (!service)
		return NULL;

	client = (ProtobufC_RPC_Client *) service;

	protobuf_c_rpc_client_set_autoreconnect_period(client, 1000);

	for (a = 0; a < nretry && !protobuf_c_rpc_client_is_connected(client);
	     a++)
		protobuf_c_dispatch_run(protobuf_c_dispatch_default());

	return a < nretry ? service : NULL;
}

void dabba_rpc_call_is_done(protobuf_c_boolean * is_done)
{
	assert(is_done);
	while (!(*is_done))
		protobuf_c_dispatch_run(protobuf_c_dispatch_default());
}

void rpc_header_print(const char *const title)
{
	assert(title);

	printf("---\n  %s:\n", title);
}

void __rpc_error_code_print(const int error_code)
{
	printf("# error code: %i\n", error_code);
}

void rpc_dummy_print(const Dabba__Dummy * result, void *closure_data)
{
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;

	assert(result);
	assert(closure_data);

	*status = 1;
}

void rpc_error_code_print(const Dabba__ErrorCode const *result,
			  void *closure_data)
{
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;

	assert(result);
	assert(closure_data);

	__rpc_error_code_print(result->code);

	*status = 1;
}
