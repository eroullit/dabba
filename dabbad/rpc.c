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

#include <google/protobuf-c/protobuf-c-rpc.h>
#include <libdabba-rpc/dabba.pb-c.h>

int dabbad_rpc_msg_poll(void)
{
	ProtobufC_RPC_Server *server;

	/* 0xDABA = 55994 */
	server =
	    protobuf_c_rpc_server_new(PROTOBUF_C_RPC_ADDRESS_TCP, "55994", NULL,
				      NULL);

	for (;;)
		protobuf_c_dispatch_run(protobuf_c_dispatch_default());

	protobuf_c_rpc_server_destroy(server, 0);

	return 0;
}
