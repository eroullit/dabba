/**
 * \file interface-offload.c
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
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <assert.h>
#include <dabba/macros.h>
#include <dabba/rpc.h>
#include <dabba/help.h>

static void display_interface_offload_header(void)
{
	printf("---\n");
	printf("  interfaces:\n");
}

static void interface_offload_list_print(const Dabba__InterfaceOffloadList *
					 result, void *closure_data)
{
	Dabba__InterfaceOffload *offloadp;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;
	size_t a;

	assert(closure_data);

	display_interface_offload_header();

	for (a = 0; result && a < result->n_list; a++) {
		offloadp = result->list[a];
		printf("    - name: %s\n", offloadp->id->name);
		printf("      offload:\n");
		printf("        rx checksum: %s\n",
		       print_tf(offloadp->rx_csum));
		printf("        tx checksum: %s\n",
		       print_tf(offloadp->tx_csum));
		printf("        scatter gather: %s\n", print_tf(offloadp->sg));
		printf("        tcp segment: %s\n", print_tf(offloadp->tso));
		printf("        udp fragment: %s\n", print_tf(offloadp->ufo));
		printf("        generic segmentation: %s\n",
		       print_tf(offloadp->gso));
		printf("        generic receive: %s\n",
		       print_tf(offloadp->gro));
		printf("        rx hashing: %s\n", print_tf(offloadp->rxhash));
	}

	*status = 1;
}

int rpc_interface_offload_get(ProtobufCService * service,
			      const Dabba__InterfaceIdList * id_list)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(id_list);

	dabba__dabba_service__interface_offload_get(service, id_list,
						    interface_offload_list_print,
						    &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}
