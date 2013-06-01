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

#include <string.h>
#include <errno.h>

#include <google/protobuf-c/protobuf-c-rpc.h>
#include <libdabba-rpc/rpc.h>
#include <dabbad/interface.h>
#include <dabbad/interface-status.h>
#include <dabbad/interface-driver.h>
#include <dabbad/interface-pause.h>
#include <dabbad/interface-offload.h>
#include <dabbad/interface-settings.h>
#include <dabbad/interface-coalesce.h>
#include <dabbad/interface-capabilities.h>
#include <dabbad/interface-statistics.h>
#include <dabbad/thread.h>
#include <dabbad/capture.h>

/**
 * \brief Protobuf service structure used by dabbad
 */

static Dabba__DabbaService_Service dabba_service =
DABBA__DABBA_SERVICE__INIT(dabbad_);

/**
 * \brief Poll server for new RPC queries to process
 * \param[in]       name	        String to RPC server address
 * \param[in]       type	        Tell if the RPC server listens to Unix or TCP sockets
 * \return 0 if the server successfully exited, \c EINVAL is the server could not be started
 * \note This function polls for new RPC queries endlessly
 */

int dabbad_rpc_msg_poll(const char *const name,
			const ProtobufC_RPC_AddressType type)
{
	ProtobufC_RPC_Server *server;

	assert(name);
	assert(type == PROTOBUF_C_RPC_ADDRESS_LOCAL
	       || type == PROTOBUF_C_RPC_ADDRESS_TCP);

	if (strlen(name) == 0)
		return EINVAL;

	server = protobuf_c_rpc_server_new(type, name,
					   (ProtobufCService *) & dabba_service,
					   NULL);

	if (!server)
		return EINVAL;

	for (;;)
		protobuf_c_dispatch_run(protobuf_c_dispatch_default());

	protobuf_c_rpc_server_destroy(server, 0);

	return 0;
}
