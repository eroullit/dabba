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

=item status

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
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>
#include <errno.h>
#include <getopt.h>
#include <linux/ethtool.h>
#include <libdabba/macros.h>
#include <dabba/dabba.h>
#include <dabba/cli.h>
#include <dabba/rpc.h>
#include <dabba/interface-status.h>
#include <dabba/interface-settings.h>
#include <dabba/interface-driver.h>
#include <dabba/interface-capabilities.h>
#include <dabba/interface-coalesce.h>
#include <dabba/interface-pause.h>
#include <dabba/interface-offload.h>
#include <dabba/interface-statistics.h>
#include <dabba/help.h>
#include <dabba/macros.h>

enum interface_modify_option {
	OPT_INTERFACE_UP,
	OPT_INTERFACE_RUNNING,
	OPT_INTERFACE_PROMISC,
	OPT_INTERFACE_ID,
};

const char *port2str(const uint8_t port)
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

static int cmd_interface_get(int argc, const char **argv)
{
	enum interface_option {
		/* option */
		OPT_INTERFACE_ID,
		OPT_TCP,
		OPT_LOCAL,
		OPT_HELP
	};

	static const struct rpc_struct {
		const char *const cmd;
		int (*const rpc) (ProtobufCService * service,
				  const Dabba__InterfaceIdList * id_list);
	} interface_commands[] = {
		{
		"statistics", rpc_interface_statistics_get}, {
		"status", rpc_interface_status_get}, {
		"settings", rpc_interface_settings_get}, {
		"pause", rpc_interface_pause_get}, {
		"offload", rpc_interface_offload_get}, {
		"driver", rpc_interface_driver_get}, {
		"coalesce", rpc_interface_coalesce_get}, {
		"capabilities", rpc_interface_capabilities_get}
	};

	const struct option interface_option[] = {
		{"id", required_argument, NULL, OPT_INTERFACE_ID},
		{"tcp", optional_argument, NULL, OPT_TCP},
		{"local", optional_argument, NULL, OPT_LOCAL},
		{"help", required_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0},
	};

	int ret, rc = 0;
	size_t a;
	const char *cmd = argv[0];
	const char *server_id = DABBA_RPC_DEFAULT_LOCAL_SERVER_NAME;
	ProtobufC_RPC_AddressType server_type = PROTOBUF_C_RPC_ADDRESS_LOCAL;
	ProtobufCService *service;
	Dabba__InterfaceIdList id_list = DABBA__INTERFACE_ID_LIST__INIT;
	Dabba__InterfaceId **idpp;
	int (*rpc_get) (ProtobufCService * service,
			const Dabba__InterfaceIdList * id_list) = NULL;

	if (argc || argv[0]) {
		/* Parse get action to run */
		for (a = 0; a < ARRAY_SIZE(interface_commands); a++)
			if (!strcmp(interface_commands[a].cmd, cmd)) {
				rpc_get = interface_commands[a].rpc;
				break;
			}
	}

	if (!rpc_get)
		return ENOSYS;

	/* parse action options */
	while ((ret =
		getopt_long_only(argc, (char **)argv, "", interface_option,
				 NULL)) != EOF) {
		switch (ret) {
		case OPT_TCP:
			server_type = PROTOBUF_C_RPC_ADDRESS_TCP;
			server_id = DABBA_RPC_DEFAULT_TCP_SERVER_NAME;

			if (optarg)
				server_id = optarg;
			break;
		case OPT_LOCAL:
			server_type = PROTOBUF_C_RPC_ADDRESS_LOCAL;
			server_id = DABBA_RPC_DEFAULT_LOCAL_SERVER_NAME;

			if (optarg)
				server_id = optarg;
			break;
		case OPT_INTERFACE_ID:
			idpp =
			    realloc(id_list.list,
				    sizeof(*id_list.list) * (id_list.n_list +
							     1));

			if (!idpp)
				return ENOMEM;

			id_list.list = idpp;
			id_list.list[id_list.n_list] =
			    malloc(sizeof(*id_list.list[id_list.n_list]));

			if (!id_list.list[id_list.n_list])
				return ENOMEM;

			dabba__interface_id__init(id_list.list[id_list.n_list]);

			id_list.list[id_list.n_list]->name = optarg;
			id_list.n_list++;

			break;
		case OPT_HELP:
		default:
			show_usage(interface_option);
			rc = -1;
			goto out;
		}
	}

	service = dabba_rpc_client_connect(server_id, server_type);

	if (service)
		rc = rpc_get(service, &id_list);
	else
		rc = EINVAL;

 out:
	/* cleanup */
	for (a = 0; a < id_list.n_list; a++)
		free(id_list.list[a]);

	free(id_list.list);

	/* Check error reporting */

	return rc;
}

static int cmd_interface_modify(int argc, const char **argv)
{
	static struct cmd_struct cmd[] = {
		{"status", cmd_interface_status_modify},
		{"pause", cmd_interface_pause_modify},
		{"settings", cmd_interface_settings_modify},
		{"coalesce", cmd_interface_coalesce_modify},
		{"capabilities", cmd_interface_capabilities_modify}
	};

	return cmd_run_builtin(cmd, ARRAY_SIZE(cmd), argc, argv);
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
	static const struct cmd_struct cmd[] = {
		{"get", cmd_interface_get},
		{"modify", cmd_interface_modify}
	};

	return cmd_run_builtin(cmd, ARRAY_SIZE(cmd), argc, argv);
}
