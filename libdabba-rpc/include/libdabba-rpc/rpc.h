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

#ifndef LIBDABBARPC_RPC_H
#define	LIBDABBARPC_RPC_H

#include <google/protobuf-c/protobuf-c-rpc.h>
#include <libdabba-rpc/dabba.pb-c.h>

#ifndef DABBA_RPC_DEFAULT_HOST_NAME
#define DABBA_RPC_DEFAULT_HOST_NAME "localhost"
#endif				/* DABBA_RPC_DEFAULT_HOST_NAME */

/* 0xDABA = 55994 */
#ifndef DABBA_RPC_DEFAULT_PORT
#define DABBA_RPC_DEFAULT_PORT "55994"
#endif				/* DABBA_RPC_DEFAULT_PORT */

#ifndef DABBA_RPC_DEFAULT_TCP_SERVER_NAME
#define DABBA_RPC_DEFAULT_TCP_SERVER_NAME DABBA_RPC_DEFAULT_HOST_NAME":"DABBA_RPC_DEFAULT_PORT
#endif				/* DABBA_RPC_DEFAULT_TCP_SERVER_NAME */

#ifndef DABBA_RPC_DEFAULT_LOCAL_SERVER_NAME
#define DABBA_RPC_DEFAULT_LOCAL_SERVER_NAME "/tmp/dabba"
#endif				/* DABBA_RPC_DEFAULT_LOCAL_SERVER_NAME */

#endif				/* LIBDABBARPC_RPC_H */
