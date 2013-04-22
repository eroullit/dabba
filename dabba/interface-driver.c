/**
 * \file interface-driver.c
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
#include <assert.h>
#include <errno.h>

#include <libdabba/macros.h>
#include <dabba/rpc.h>
#include <dabba/help.h>
#include <dabba/interface.h>
#include <dabba/dabba.h>

/**
 * \internal
 * \brief Protobuf closure to print interface driver list in YAML
 * \param[in]           result	        Pointer to interface driver list
 * \param[in]           closure_data	Pointer to protobuf closure data
 */

static void interface_driver_list_print(const Dabba__InterfaceDriverList *
					result, void *closure_data)
{
	Dabba__InterfaceDriver *driverp;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;
	size_t a;

	assert(closure_data);

	rpc_header_print("interfaces");

	for (a = 0; result && a < result->n_list; a++) {
		driverp = result->list[a];
		printf("    - name: %s\n", driverp->id->name);
		printf("    ");
		__rpc_error_code_print(driverp->status->code);
		printf("      driver name: %s\n", driverp->name);
		printf("      driver version: %s\n", driverp->version);
		printf("      firmware version: %s\n", driverp->fw_version);
		printf("      bus info: %s\n", driverp->bus_info);
	}

	*status = 1;
}

/**
 * \internal
 * \brief Invoke interface driver get RPC
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           id_list         Pointer to interface id to fetch
 * \return Always returns 0.
 * \note An empty id list will query the driver information of all available interfaces.
 */

static int rpc_interface_driver_get(ProtobufCService * service,
				    const Dabba__InterfaceIdList * id_list)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(id_list);

	dabba__dabba_service__interface_driver_get(service, id_list,
						   interface_driver_list_print,
						   &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

static int cmd_interface_driver_get(int argc, const char **argv)
{
	return rpc_interface_get(argc, argv, rpc_interface_driver_get);
}

int cmd_interface_driver(int argc, const char **argv)
{
	static const struct cmd_struct cmd[] = {
		{"get", cmd_interface_driver_get}
	};

	return cmd_run_builtin(cmd, ARRAY_SIZE(cmd), argc, argv);
}
