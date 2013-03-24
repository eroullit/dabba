/**
 * \file capture-list.c
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

static void capture_settings_print(const Dabba__CaptureList *
				   result, void *closure_data)
{
	const Dabba__Capture *capture;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;
	size_t a;

	rpc_header_print("captures");

	for (a = 0; result && a < result->n_list; a++) {
		capture = result->list[a];
		__rpc_error_code_print(capture->status->code);
		printf("    - id: %" PRIu64 "\n", (uint64_t) capture->id->id);
		printf("      packet mmap size: %" PRIu64 "\n",
		       capture->frame_nr * capture->frame_size);
		printf("      frame number: %" PRIu64 "\n", capture->frame_nr);
		printf("      pcap: %s\n", capture->pcap);
		printf("      interface: %s\n", capture->interface);
	}

	*status = 1;
}

int rpc_capture_settings_get(ProtobufCService * service,
			     const Dabba__ThreadIdList * id_list)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(id_list);

	dabba__dabba_service__capture_get(service, id_list,
					  capture_settings_print, &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}
