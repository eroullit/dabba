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
#include <errno.h>
#include <dabba/rpc.h>
#include <string.h>

/**
 * \brief Connect the client to the selected server to make a RPC
 * \param[in]           name	        Dabbad server name
 * \param[in]           type		unix domain or TCP socket connection
 * \return Returns a protobuf service handle pointer
 * \note Performs 3 retries each 100ms if the requested server does not answer.
 */

ProtobufCService *dabba_rpc_client_connect(const char *const name,
					   const ProtobufC_RPC_AddressType type)
{
	/* The first failure triggers a instantaneous retry, we want 3 retry */
	size_t a, nretry = 4;
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

	protobuf_c_rpc_client_set_autoreconnect_period(client, 100);

	for (a = 0; a < nretry && !protobuf_c_rpc_client_is_connected(client);
	     a++)
		protobuf_c_dispatch_run(protobuf_c_dispatch_default());

	return a < nretry ? service : NULL;
}

/**
 * \brief Poll until an RPC call is dispatched and completed
 * \param[in]           is_done	        Pointer to protobuf completion variable
 */

void dabba_rpc_call_is_done(protobuf_c_boolean * is_done)
{
	assert(is_done);
	while (!(*is_done))
		protobuf_c_dispatch_run(protobuf_c_dispatch_default());
}

/**
 * \brief Print a YAML document header on \c stdout
 * \param[in]           title	        Title string
 */

void rpc_header_print(const char *const title)
{
	assert(title);

	printf("---\n  %s:\n", title);
}

/**
 * \internal
 * \brief Print RPC error code to \c stdout
 * \param[in]           error_code	 Error code value
 * \note also prints a string describing error number as a YAML comment
 */

void __rpc_error_code_print(const int error_code)
{
	printf("  rc: %i # %s\n", error_code, strerror(error_code));
}

/**
 * \brief Print RPC error code to \c stdout
 * \param[in]           result  	 RPC error code message
 * \param[in]           closure_data	 Pointer to protobuf closure data
 */

void rpc_error_code_print(const Dabba__ErrorCode const *result,
			  void *closure_data)
{
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;

	assert(closure_data);

	if (result)
		__rpc_error_code_print(result->code);
	else
		__rpc_error_code_print(EINVAL);

	*status = 1;
}
