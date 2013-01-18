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

#ifndef DABBA_RPC_H
#define	DABBA_RPC_H

#include <libdabba-rpc/rpc.h>

ProtobufCService *dabba_rpc_client_connect(const char *const name,
					   const ProtobufC_RPC_AddressType
					   type);
void dabba_rpc_call_is_done(protobuf_c_boolean * is_done);
void rpc_header_print(const char *const title);

#endif				/* DABBA_RPC_H */
