/**
 * \file interface.c
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

/*

=head1 NAME

dabba-interface - Manage network interface

=head1 SYNOPSIS

dabba interface <command> [<arguments>...] [--help]

=head1 DESCRIPTION

Give the user the possibility to manage the available network interfaces.

=head1 COMMANDS

=over

=item list

Fetch and print status information and statistics about currently
supported interfaces.
The output is formatted in YAML.

=item driver

Output driver information which are used by the network interfaces.

=item settings

Retrieve current interface settings.

=item capabilities

Report which features are supported by the interfaces.

=item pause

Output current interface pause settings.

=item coalesce

Query interface coalescing information.

=item offload

Report which interface offloading features are enabled.

=item modify

Modify status of available network interfaces.

=back

=head1 OPTIONS

=over

=item --id <name>

interface name to work on.

=item --up (true|false)

Activate or shutdown a specific interface.

=item --running (true|false)

Mark the interface as operational.

=item --promiscuous (true|false)

Enable or disable the 'promiscuous' mode of an interface.
If selected, all packets on the network will be received by the interface.

=back

=head1 EXAMPLES

=over

=item dabba interface list

Output the list of network interface which can be used by dabba.

=item dabba interface modify --id eth0 --up true --running true --promiscuous true

Modify the status of the interface 'eth0' to be up and running and alse
listen to all incoming packets.

=back

=head1 AUTHOR

Written by Emmanuel Roullit <emmanuel.roullit@gmail.com>

=head1 BUGS

=over

=item Please report bugs to <https://github.com/eroullit/dabba/issues>

=item dabba project project page: <https://github.com/eroullit/dabba>

=back

=head1 COPYRIGHT

=over

=item Copyright © 2012 Emmanuel Roullit.

=item License GPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html>.

=item This is free software: you are free to change and redistribute it.

=item There is NO WARRANTY, to the extent permitted by law.

=back

=cut

*/

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <libdabba/macros.h>
#include <dabba/dabba.h>
#include <dabba/ipc.h>
#include <dabba/interface-settings.h>
#include <dabba/interface-driver.h>
#include <dabba/interface-capabilities.h>
#include <dabba/interface-coalesce.h>
#include <dabba/interface-pause.h>
#include <dabba/interface-offload.h>
#include <dabba/help.h>
#include <dabba/macros.h>

enum interface_modify_option {
	OPT_INTERFACE_UP,
	OPT_INTERFACE_RUNNING,
	OPT_INTERFACE_PROMISC,
	OPT_INTERFACE_ID,
};

const char *ethtool_port_str_get(const uint8_t port)
{
	static const char *const port_str[] = {
		[PORT_TP] = "tp",
		[PORT_AUI] = "aui",
		[PORT_MII] = "mii",
		[PORT_FIBRE] = "fibre",
		[PORT_BNC] = "bnc",
		[PORT_DA] = "da"
	};

	return port < sizeof(port_str) ? port_str[port] : "unknown";
}

static struct option *interface_modify_options_get(void)
{
	static struct option interface_modify_option[] = {
		{"up", required_argument, NULL, OPT_INTERFACE_UP},
		{"running", required_argument, NULL, OPT_INTERFACE_RUNNING},
		{"promiscuous", required_argument, NULL, OPT_INTERFACE_PROMISC},
		{"id", required_argument, NULL, OPT_INTERFACE_ID},
		{NULL, 0, NULL, 0},
	};

	return interface_modify_option;
}

static void display_interface_list_header(void)
{
	printf("---\n");
	printf("  interfaces:\n");
}

/**
 * \brief Print fetch interface information in a YAML format
 * \param[in]           interface_msg	interface information IPC message
 * \param[in]           elem_nr		number of interfaces to report
 */

static void display_interface_list(const struct dabba_ipc_msg *const msg)
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

int cmd_interface_list(int argc, const char **argv)
{
	struct dabba_ipc_msg msg;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));

	msg.msg_body.type = DABBA_INTERFACE_LIST;
	msg.msg_body.op_type = OP_GET;
	msg.msg_body.method_type = MT_BULK;

	display_interface_list_header();

	return dabba_ipc_fetch_all(&msg, display_interface_list);
}

static int prepare_interface_modify_query(int argc, char **argv, struct dabba_interface_list
					  *ifconf_msg)
{
	int ret, rc = 0;

	assert(ifconf_msg);

	ifconf_msg->up = UNSET;
	ifconf_msg->running = UNSET;
	ifconf_msg->promisc = UNSET;
	ifconf_msg->loopback = UNSET;

	while ((ret =
		getopt_long_only(argc, argv, "", interface_modify_options_get(),
				 NULL)) != EOF) {
		switch (ret) {
		case OPT_INTERFACE_UP:
			ifconf_msg->up = dabba_tristate_parse(optarg);
			break;
		case OPT_INTERFACE_RUNNING:
			ifconf_msg->running = dabba_tristate_parse(optarg);
			break;
		case OPT_INTERFACE_PROMISC:
			ifconf_msg->promisc = dabba_tristate_parse(optarg);
			break;
		case OPT_INTERFACE_ID:
			strncpy(ifconf_msg->name, optarg,
				sizeof(ifconf_msg->name));
			break;
		default:
			show_usage(interface_modify_options_get());
			rc = -1;
			break;
		}
	}

	return rc;
}

/**
 * \brief Modify parameters of a supported interface
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 */

int cmd_interface_modify(int argc, const char **argv)
{
	int rc;
	struct dabba_ipc_msg msg;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));

	msg.msg_body.type = DABBA_INTERFACE_MODIFY;
	msg.msg_body.op_type = OP_MODIFY;
	msg.msg_body.method_type = MT_FILTERED;
	msg.msg_body.elem_nr = 1;

	rc = prepare_interface_modify_query(argc, (char **)argv,
					    msg.msg_body.msg.interface_list);

	if (rc)
		return rc;

	return dabba_ipc_msg(&msg);
}

/**
 * \brief Parse which interface sub-command.
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, ENOSYS if the sub-command does not exist,
 * else on failure.
 *
 * This function parses the interface sub-command string and the rest of the
 * argument vector to the proper sub-command handler.
 */

int cmd_interface(int argc, const char **argv)
{
	const char *cmd = argv[0];
	size_t i;
	static struct cmd_struct interface_commands[] = {
		{"list", cmd_interface_list},
		{"driver", cmd_interface_driver},
		{"settings", cmd_interface_settings},
		{"capabilities", cmd_interface_capabilities},
		{"pause", cmd_interface_pause},
		{"coalesce", cmd_interface_coalesce},
		{"offload", cmd_interface_offload},
		{"modify", cmd_interface_modify}
	};

	if (argc == 0 || cmd == NULL || !strcmp(cmd, "--help"))
		cmd = "help";

	for (i = 0; i < ARRAY_SIZE(interface_commands); i++) {
		if (!strcmp(interface_commands[i].cmd, cmd))
			return run_builtin(&interface_commands[i], argc, argv);
	}

	return ENOSYS;
}
