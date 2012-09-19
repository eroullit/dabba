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

Fetch and print information about currenty supported interfaces.
The output is formatted in YAML.

=item modify

Modify status of available network interfaces

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
#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <libdabba/macros.h>
#include <libdabba/strlcpy.h>
#include <dabba/dabba.h>
#include <dabba/ipc.h>
#include <dabba/help.h>

#ifndef print_tf
#define print_tf(expr) (expr) ? "true" : "false"
#endif

enum interface_modify_option {
	OPT_INTERFACE_UP,
	OPT_INTERFACE_RUNNING,
	OPT_INTERFACE_PROMISC,
	OPT_INTERFACE_ID,
};

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

static void display_interface_list(const struct dabba_interface_list *const
				   interface_msg, const size_t elem_nr)
{
	size_t a;
	const struct dabba_interface_list *iface;

	assert(interface_msg);
	assert(elem_nr <= DABBA_INTERFACE_LIST_MAX_SIZE);

	for (a = 0; a < elem_nr; a++) {
		iface = &interface_msg[a];
		printf("    - name: %s\n", iface->name);
		printf("      status: {");
		printf("up: %s, ", print_tf(iface->up == TRUE));
		printf("running: %s, ", print_tf(iface->running == TRUE));
		printf("promiscuous: %s, ", print_tf(iface->promisc == TRUE));
		printf("loopback: %s", print_tf(iface->loopback == TRUE));
		printf("}\n");
		printf("      statistics:\n");
		printf("          rx: {");
		printf("byte: %u, ", iface->rx.byte);
		printf("packet: %u, ", iface->rx.packet);
		printf("error: %u, ", iface->rx.error);
		printf("dropped: %u, ", iface->rx.dropped);
		printf("compressed: %u", iface->rx.compressed);
		printf("}\n");
		printf("          tx: {");
		printf("byte: %u, ", iface->tx.byte);
		printf("packet: %u, ", iface->tx.packet);
		printf("error: %u, ", iface->tx.error);
		printf("dropped: %u, ", iface->tx.dropped);
		printf("compressed: %u", iface->tx.compressed);
		printf("}\n");
		printf("          rx error: {");
		printf("fifo: %u, ", iface->rx_error.fifo);
		printf("frame: %u, ", iface->rx_error.frame);
		printf("crc: %u, ", iface->rx_error.crc);
		printf("length: %u, ", iface->rx_error.length);
		printf("missed: %u, ", iface->rx_error.missed);
		printf("overflow: %u", iface->rx_error.over);
		printf("}\n");
		printf("          tx error: {");
		printf("fifo: %u, ", iface->tx_error.fifo);
		printf("carrier: %u, ", iface->tx_error.carrier);
		printf("heartbeat: %u, ", iface->tx_error.heartbeat);
		printf("window: %u, ", iface->tx_error.window);
		printf("aborted: %u", iface->tx_error.aborted);
		printf("}\n");
	}
}

static void display_interface_driver(const struct dabba_interface_driver *const
				     interface_driver, const size_t elem_nr)
{
	size_t a;
	const struct dabba_interface_driver *iface;

	assert(interface_driver);
	assert(elem_nr <= DABBA_INTERFACE_DRIVER_MAX_SIZE);

	for (a = 0; a < elem_nr; a++) {
		iface = &interface_driver[a];
		printf("    - name: %s\n", iface->name);
		printf("      driver: {");
		printf("name: %s, ", iface->driver_info.driver);
		printf("version: %s, ", iface->driver_info.version);
		printf("firmware version: %s", iface->driver_info.fw_version);
		printf("}\n");
	}
}

static void display_interface_settings(const struct dabba_interface_settings
				       *const interface_settings,
				       const size_t elem_nr)
{
	size_t a;
	const struct dabba_interface_settings *iface;

	assert(interface_settings);
	assert(elem_nr <= DABBA_INTERFACE_SETTINGS_MAX_SIZE);

	for (a = 0; a < elem_nr; a++) {
		iface = &interface_settings[a];
		printf("    - name: %s\n", iface->name);
		printf("      settings: {");
		printf("speed: %u, ", ethtool_cmd_speed(&iface->settings));
		printf("duplex: %s, ", print_tf(iface->settings.duplex));
		printf("autoneg: %s, ", print_tf(iface->settings.autoneg));
		printf("port: %u, ", iface->settings.port);
		printf("address: %u, ", iface->settings.phy_address);
		printf("max rx packet: %u, ", iface->settings.maxrxpkt);
		printf("max tx packet: %u", iface->settings.maxtxpkt);
		printf("}\n");
	}
}

/**
 * \brief Request the current supported interface list
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
	int rc;
	struct dabba_ipc_msg msg;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));

	msg.mtype = 1;
	msg.msg_body.type = DABBA_INTERFACE_LIST;

	display_interface_list_header();

	do {
		msg.msg_body.offset += msg.msg_body.elem_nr;
		msg.msg_body.elem_nr = 0;

		rc = dabba_ipc_msg(&msg);

		if (rc)
			break;

		display_interface_list(msg.msg_body.msg.interface_list,
				       msg.msg_body.elem_nr);
	} while (msg.msg_body.elem_nr);

	return rc;
}

int cmd_interface_driver(int argc, const char **argv)
{
	int rc;
	struct dabba_ipc_msg msg;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));

	msg.mtype = 1;
	msg.msg_body.type = DABBA_INTERFACE_DRIVER;

	display_interface_list_header();

	do {
		msg.msg_body.offset += msg.msg_body.elem_nr;
		msg.msg_body.elem_nr = 0;

		rc = dabba_ipc_msg(&msg);

		if (rc)
			break;

		display_interface_driver(msg.msg_body.msg.interface_driver,
					 msg.msg_body.elem_nr);
	} while (msg.msg_body.elem_nr);

	return rc;
}

int cmd_interface_settings(int argc, const char **argv)
{
	int rc;
	struct dabba_ipc_msg msg;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));

	msg.mtype = 1;
	msg.msg_body.type = DABBA_INTERFACE_SETTINGS;

	display_interface_list_header();

	do {
		msg.msg_body.offset += msg.msg_body.elem_nr;
		msg.msg_body.elem_nr = 0;

		rc = dabba_ipc_msg(&msg);

		if (rc)
			break;

		display_interface_settings(msg.msg_body.msg.interface_settings,
					   msg.msg_body.elem_nr);
	} while (msg.msg_body.elem_nr);

	return rc;
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
			strlcpy(ifconf_msg->name, optarg,
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
 * \brief Modify parametets of a supported interface
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

	msg.mtype = 1;
	msg.msg_body.type = DABBA_INTERFACE_MODIFY;
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
