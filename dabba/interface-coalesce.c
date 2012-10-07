/**
 * \file interface-coalesce.c
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
#include <assert.h>

#include <dabbad/dabbad.h>
#include <dabba/macros.h>
#include <dabba/ipc.h>

static void display_interface_coalease_header(void)
{
	printf("---\n");
	printf("  interfaces:\n");
}

static void display_interface_coalesce(const struct dabba_ipc_msg *const msg)
{
	size_t a;
	const struct dabba_interface_coalesce *iface;

	assert(msg);
	assert(msg->msg_body.elem_nr <= DABBA_INTERFACE_COALESCE_MAX_SIZE);
	assert(msg->msg_body.type == DABBA_INTERFACE_COALESCE);

	for (a = 0; a < msg->msg_body.elem_nr; a++) {
		iface = &msg->msg_body.msg.interface_coalesce[a];
		printf("    - name: %s\n", iface->name);
		printf("      coalesce:\n");
		printf("        packet rate high: %u\n",
		       iface->coalesce.pkt_rate_high);
		printf("        packet rate low: %u\n",
		       iface->coalesce.pkt_rate_low);
		printf("        rate sample interval: %u\n",
		       iface->coalesce.rate_sample_interval);
		printf("        stats block: %u\n",
		       iface->coalesce.stats_block_coalesce_usecs);
		printf("        rx:\n");
		printf("            adaptive: %s\n",
		       print_tf(iface->coalesce.use_adaptive_rx_coalesce));
		printf("            usec: {");
		printf("normal: %u, ", iface->coalesce.rx_coalesce_usecs);
		printf("irq: %u, ", iface->coalesce.rx_coalesce_usecs_irq);
		printf("high: %u, ", iface->coalesce.rx_coalesce_usecs_high);
		printf("low: %u", iface->coalesce.rx_coalesce_usecs_low);
		printf("}\n");
		printf("            max frame: {");
		printf("normal: %u, ", iface->coalesce.rx_max_coalesced_frames);
		printf("irq: %u, ",
		       iface->coalesce.rx_max_coalesced_frames_irq);
		printf("high: %u, ",
		       iface->coalesce.rx_max_coalesced_frames_high);
		printf("low: %u", iface->coalesce.rx_max_coalesced_frames_low);
		printf("}\n");
		printf("        tx:\n");
		printf("            adaptive: %s\n",
		       print_tf(iface->coalesce.use_adaptive_tx_coalesce));
		printf("            usec: {");
		printf("normal: %u, ", iface->coalesce.tx_coalesce_usecs);
		printf("irq: %u, ", iface->coalesce.tx_coalesce_usecs_irq);
		printf("high: %u, ", iface->coalesce.tx_coalesce_usecs_high);
		printf("low: %u", iface->coalesce.tx_coalesce_usecs_low);
		printf("}\n");
		printf("            max frame: {");
		printf("normal: %u, ", iface->coalesce.tx_max_coalesced_frames);
		printf("irq: %u, ",
		       iface->coalesce.tx_max_coalesced_frames_irq);
		printf("high: %u, ",
		       iface->coalesce.tx_max_coalesced_frames_high);
		printf("low: %u", iface->coalesce.tx_max_coalesced_frames_low);
		printf("}\n");
	}
}

/**
 * \brief Get interface coalesce parameters and output them on \c stdout
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 */

int cmd_interface_coalesce(int argc, const char **argv)
{
	struct dabba_ipc_msg msg;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));

	msg.msg_body.type = DABBA_INTERFACE_COALESCE;
	msg.msg_body.op_type = OP_GET;
	msg.msg_body.method_type = MT_BULK;

	display_interface_coalease_header();

	return dabba_ipc_fetch_all(&msg, display_interface_coalesce);
}
