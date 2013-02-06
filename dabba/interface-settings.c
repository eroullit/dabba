/**
 * \file interface-settings.c
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

extern const char *port2str(const uint8_t port);

static void display_interface_settings_header(void)
{
	printf("---\n");
	printf("  interfaces:\n");
}

static void interface_settings_list_print(const Dabba__InterfaceSettingsList *
					  result, void *closure_data)
{
	Dabba__InterfaceSettings *settingsp;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;
	size_t a;

	assert(closure_data);

	display_interface_settings_header();

	for (a = 0; result && a < result->n_list; a++) {
		settingsp = result->list[a];
		printf("    - name: %s\n", settingsp->id->name);
		printf("      settings:\n");
		printf("        speed: %u\n", settingsp->speed);
		printf("        duplex: %s\n",
		       settingsp->duplex ? "full" : "half");
		printf("        autoneg: %s\n", print_tf(settingsp->autoneg));
		printf("        mtu: %u\n", settingsp->mtu);
		printf("        tx qlen: %u\n", settingsp->tx_qlen);
		printf("        port: %s\n", port2str(settingsp->port));
		printf("        max rx packet: %u\n", settingsp->maxrxpkt);
		printf("        max tx packet: %u\n", settingsp->maxtxpkt);
	}

	*status = 1;
}

int rpc_interface_settings_get(ProtobufCService * service,
			       const Dabba__InterfaceIdList * id_list)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(id_list);

	dabba__dabba_service__interface_settings_get(service, id_list,
						     interface_settings_list_print,
						     &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}
