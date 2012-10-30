/**
 * \file interface-status.c
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
#include <inttypes.h>
#include <assert.h>

#include <dabbad/dabbad.h>
#include <dabba/macros.h>
#include <dabba/ipc.h>

static void display_interface_status_header(void)
{
	printf("---\n");
	printf("  interfaces:\n");
}

/**
 * \brief Print fetch interface information in a YAML format
 * \param[in]           interface_msg	interface information IPC message
 * \param[in]           elem_nr		number of interfaces to report
 */

static void display_interface_status(const struct dabba_ipc_msg *const msg)
{
	size_t a;
	const struct dabba_interface_list *iface;

	assert(msg);
	assert(msg->msg_body.elem_nr <= DABBA_INTERFACE_LIST_MAX_SIZE);
	assert(msg->msg_body.type == DABBA_INTERFACE_LIST);

	for (a = 0; a < msg->msg_body.elem_nr; a++) {
		iface = &msg->msg_body.msg.interface_list[a];
		printf("    - name: %s\n", iface->name);
		printf("      status: {");
		printf("link: %s, ", print_tf(iface->link));
		printf("up: %s, ", print_tf(iface->up == TRUE));
		printf("running: %s, ", print_tf(iface->running == TRUE));
		printf("promiscuous: %s, ", print_tf(iface->promisc == TRUE));
		printf("loopback: %s", print_tf(iface->loopback == TRUE));
		printf("}\n");
	}
#if 0
	printf("      statistics:\n");
	printf("          rx: {");
	printf("byte: %" PRIu64 ", ", iface->rx.byte);
	printf("packet: %" PRIu64 ", ", iface->rx.packet);
	printf("error: %" PRIu64 ", ", iface->rx.error);
	printf("dropped: %" PRIu64 ", ", iface->rx.dropped);
	printf("compressed: %" PRIu64 "", iface->rx.compressed);
	printf("}\n");
	printf("          tx: {");
	printf("byte: %" PRIu64 ", ", iface->tx.byte);
	printf("packet: %" PRIu64 ", ", iface->tx.packet);
	printf("error: %" PRIu64 ", ", iface->tx.error);
	printf("dropped: %" PRIu64 ", ", iface->tx.dropped);
	printf("compressed: %" PRIu64 "", iface->tx.compressed);
	printf("}\n");
	printf("          rx error: {");
	printf("fifo: %" PRIu64 ", ", iface->rx_error.fifo);
	printf("frame: %" PRIu64 ", ", iface->rx_error.frame);
	printf("crc: %" PRIu64 ", ", iface->rx_error.crc);
	printf("length: %" PRIu64 ", ", iface->rx_error.length);
	printf("missed: %" PRIu64 ", ", iface->rx_error.missed);
	printf("overflow: %" PRIu64 "", iface->rx_error.over);
	printf("}\n");
	printf("          tx error: {");
	printf("fifo: %" PRIu64 ", ", iface->tx_error.fifo);
	printf("carrier: %" PRIu64 ", ", iface->tx_error.carrier);
	printf("heartbeat: %" PRIu64 ", ", iface->tx_error.heartbeat);
	printf("window: %" PRIu64 ", ", iface->tx_error.window);
	printf("aborted: %" PRIu64 "", iface->tx_error.aborted);
	printf("}\n");
}
#endif
}

/**
 * \brief Get current interface list information and output them on \c stdout
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 *
 * This function prepares an IPC message to query the supported network
 * interfaces present on the system. Once the message is sent, it waits for the
 * dabba daemon to reply.
 */

int cmd_interface_status(int argc, const char **argv)
{
	struct dabba_ipc_msg msg;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));

	if (!dabba_operation_is_present(argc, optind))
		return -1;

	msg.msg_body.type = DABBA_INTERFACE_LIST;
	msg.msg_body.op_type = dabba_operation_get(argv[optind++]);
	msg.msg_body.method_type = MT_BULK;

	display_interface_status_header();

	return dabba_ipc_fetch_all(&msg, display_interface_status);
}
