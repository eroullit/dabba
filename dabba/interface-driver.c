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
#include <getopt.h>
#include <assert.h>

#include <dabbad/dabbad.h>
#include <dabba/ipc.h>

static void display_interface_driver_header(void)
{
	printf("---\n");
	printf("  interfaces:\n");
}

static void display_interface_driver(const struct dabba_ipc_msg *const msg)
{
	size_t a;
	const struct dabba_interface_driver *iface;

	assert(msg);
	assert(msg->msg_body.elem_nr <= DABBA_INTERFACE_DRIVER_MAX_SIZE);
	assert(msg->msg_body.type == DABBA_INTERFACE_DRIVER);

	for (a = 0; a < msg->msg_body.elem_nr; a++) {
		iface = &msg->msg_body.msg.interface_driver[a];
		printf("    - name: %s\n", iface->name);
		printf("      driver name: %s\n", iface->driver_info.driver);
		printf("      driver version: %s\n",
		       iface->driver_info.version);
		printf("      firmware version: %s\n",
		       iface->driver_info.fw_version);
		printf("      bus info: %s\n", iface->driver_info.bus_info);
	}
}

/**
 * \brief Get interface driver information and output them on \c stdout
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 */

int cmd_interface_driver(int argc, const char **argv)
{
	struct dabba_ipc_msg msg;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));

	msg.msg_body.type = DABBA_INTERFACE_DRIVER;
	msg.msg_body.op_type = dabba_operation_get(argv[optind++]);
	msg.msg_body.method_type = MT_BULK;

	display_interface_driver_header();

	return dabba_ipc_fetch_all(&msg, display_interface_driver);
}
