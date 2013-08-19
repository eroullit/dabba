/**
 * \file interface-settings.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (C) 2012
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

dabba-interface-settings - Manage network interface hardware settings

=head1 SYNOPSIS

dabba interface settings <action> [<arguments>...] [--help]

=head1 DESCRIPTION

Give the user the possibility to manage the hardware settings of supported
network interfaces.

=head1 ACTIONS

=over

=item get

Fetch and print hardware settings information about currently supported interfaces.
The output is formatted in YAML.

=item modify

Apply new hardware settings to a specific network interface.

=back

=head1 OPTIONS

=over

=item --id <name>

interface name to work on.

=item --speed (10|100|1000|10000)

Set speed on network interface

=item --duplex (half|full)

Set network interface duplex mode

=item --autoneg (true|false)

Activate or shutdown speed auto-negotiation.

=item --mtu <mtu>

Set the maximum transfer unit of a network interface

=item --txqlen <qlen>

Set the length of the transmit queue of the network interface

=item --tcp[=<hostname>:<port>]

Query a running instance of dabbad using a TCP socket (default: localhost:55994)

=item --local[=<path>]

Query a running instance of dabbad using a Unix domain socket (default: /tmp/dabba)

=item --help

Prints the help message on the terminal

=back

=head1 EXAMPLES

=over

=item dabba interface settings get

Output the hardware settings of all available network interfaces.

=item dabba interface settings get --id eth0

Output the hardware settings of 'eth0'.

=item dabba interface settings modify --id eth0 --speed 100

Set 'eth0' to use a 100Mbps connectivity (if supported).

=item dabba interface settings modify --id eth0 --autoneg false

Disable speed auto negotiation on 'eth0'.

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

=item Copyright (C) 2012 Emmanuel Roullit.

=item License GPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html>.

=item This is free software: you are free to change and redistribute it.

=item There is NO WARRANTY, to the extent permitted by law.

=back

=cut

*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <assert.h>
#include <libdabba/macros.h>
#include <dabba/macros.h>
#include <dabba/rpc.h>
#include <dabba/cli.h>
#include <dabba/help.h>
#include <dabba/interface.h>
#include <dabba/dabba.h>

/**
 * \internal
 * \brief Protobuf closure to print interface settings list in YAML
 * \param[in]           result	        Pointer to interface settings list
 * \param[in]           closure_data	Pointer to protobuf closure data
 */

static void interface_settings_list_print(const Dabba__InterfaceSettingsList *
					  result, void *closure_data)
{
	Dabba__InterfaceSettings *settingsp;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;
	size_t a;

	assert(closure_data);

	rpc_header_print("interfaces");

	for (a = 0; result && a < result->n_list; a++) {
		settingsp = result->list[a];
		printf("    - name: %s\n", settingsp->id->name);
		printf("    ");
		__rpc_error_code_print(settingsp->status->code);
		printf("      settings:\n");
		printf("        speed: %u\n", settingsp->speed);
		printf("        duplex: %s\n", duplex2str(settingsp->duplex));
		printf("        autoneg: %s\n", print_tf(settingsp->autoneg));
		printf("        mtu: %u\n", settingsp->mtu);
		printf("        txqlen: %u\n", settingsp->tx_qlen);
		printf("        port: %s\n", port2str(settingsp->port));
		printf("        max rx packet: %u\n", settingsp->maxrxpkt);
		printf("        max tx packet: %u\n", settingsp->maxtxpkt);
	}

	*status = 1;
}

/**
 * \brief Invoke interface settings get RPC
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           id_list         Pointer to interface id to fetch
 * \return Always returns 0.
 * \note An empty id list will query the settings of all available interfaces.
 */

static int rpc_interface_settings_get(ProtobufCService * service,
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

/**
 * \internal
 * \brief Invoke interface settings modify RPC
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           settings        Pointer to interface new settings settings
 * \return Always returns 0.
 */

static int rpc_interface_settings_modify(ProtobufCService * service,
					 const Dabba__InterfaceSettings *
					 settingsp)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(settingsp);

	dabba__dabba_service__interface_settings_modify(service, settingsp,
							rpc_error_code_print,
							&is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

/**
 * \brief Prepare interface settings modify RPC from \c argv
 * \param[in]           argc	        Argument counter
 * \param[in]           argv	        Argument vector
 * \return Returns 0 on success, else otherwise.
 */

static int cmd_interface_settings_modify(int argc, const char **argv)
{
	enum interface_option {
		/* option */
		OPT_INTERFACE_SPEED,
		OPT_INTERFACE_DUPLEX,
		OPT_INTERFACE_AUTONEG,
		OPT_INTERFACE_MTU,
		OPT_INTERFACE_TXQLEN,
		OPT_INTERFACE_PORT,
		OPT_INTERFACE_MAX_RXPKT,
		OPT_INTERFACE_MAX_TXPKT,
		OPT_INTERFACE_ID,
		OPT_TCP,
		OPT_LOCAL,
		OPT_HELP
	};

	const struct option interface_option[] = {
		{"speed", required_argument, NULL, OPT_INTERFACE_SPEED},
		{"duplex", required_argument, NULL, OPT_INTERFACE_DUPLEX},
		{"autoneg", required_argument, NULL, OPT_INTERFACE_AUTONEG},
		{"mtu", required_argument, NULL, OPT_INTERFACE_MTU},
		{"txqlen", required_argument, NULL, OPT_INTERFACE_TXQLEN},
		{"id", required_argument, NULL, OPT_INTERFACE_ID},
		{"tcp", optional_argument, NULL, OPT_TCP},
		{"local", optional_argument, NULL, OPT_LOCAL},
		{"help", no_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0},
	};

	int ret, rc = 0;
	const char *server_id = DABBA_RPC_DEFAULT_TCP_SERVER_NAME;
	ProtobufC_RPC_AddressType server_type = PROTOBUF_C_RPC_ADDRESS_TCP;
	ProtobufCService *service;
	Dabba__InterfaceSettings settings = DABBA__INTERFACE_SETTINGS__INIT;
	Dabba__ErrorCode err = DABBA__ERROR_CODE__INIT;

	/* HACK: getopt*() start to parse options at argv[1] */
	argc++;
	argv--;

	while ((ret =
		getopt_long_only(argc, (char **)argv, "", interface_option,
				 NULL)) != EOF) {
		switch (ret) {
		case OPT_INTERFACE_SPEED:
			rc = str2speed(optarg, &settings.speed);

			if (rc)
				goto out;

			settings.has_speed = 1;
			break;
		case OPT_INTERFACE_DUPLEX:
			rc = str2duplex(optarg, &settings.duplex);

			if (rc)
				goto out;

			settings.has_duplex = 1;
			break;
		case OPT_INTERFACE_AUTONEG:
			rc = str2bool(optarg, &settings.autoneg);

			if (rc)
				goto out;

			settings.has_autoneg = 1;
			break;
		case OPT_INTERFACE_MTU:
			settings.mtu = strtoul(optarg, NULL, 10);
			settings.has_mtu = 1;
			break;
		case OPT_INTERFACE_TXQLEN:
			settings.tx_qlen = strtoul(optarg, NULL, 10);
			settings.has_tx_qlen = 1;
			break;
		case OPT_INTERFACE_MAX_RXPKT:
			settings.maxrxpkt = strtoul(optarg, NULL, 10);
			settings.has_maxrxpkt = 1;
			break;
		case OPT_INTERFACE_MAX_TXPKT:
			settings.maxtxpkt = strtoul(optarg, NULL, 10);
			settings.has_maxtxpkt = 1;
			break;
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
			settings.id = malloc(sizeof(*settings.id));

			if (!settings.id)
				return ENOMEM;

			dabba__interface_id__init(settings.id);
			settings.id->name = optarg;
			break;
		case OPT_HELP:
		default:
			show_usage(interface_option);
			rc = -1;
			goto out;
		}
	}

	settings.status = &err;

	service = dabba_rpc_client_connect(server_id, server_type);

	if (service)
		rc = rpc_interface_settings_modify(service, &settings);
	else
		rc = EINVAL;
 out:
	free(settings.id);
	return rc;
}

static int cmd_interface_settings_get(int argc, const char **argv)
{
	return rpc_interface_get(argc, argv, rpc_interface_settings_get);
}

int cmd_interface_settings(int argc, const char **argv)
{
	static const struct cmd_struct cmd[] = {
		{"get", cmd_interface_settings_get},
		{"modify", cmd_interface_settings_modify}
	};

	return cmd_run_action(cmd, ARRAY_SIZE(cmd), argc, argv);
}
