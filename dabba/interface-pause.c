/**
 * \file interface-pause.c
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

dabba-interface-pause - Manage network interface pause settings

=head1 SYNOPSIS

dabba interface pause <command> [<arguments>...] [--help]

=head1 DESCRIPTION

Give the user the possibility to manage the pause settings of supported
network interfaces.

=head1 COMMANDS

=over

=item get

Fetch and print pause information about currently supported interfaces.
The output is formatted in YAML.

=item modify

Apply new pause settings to a specific network interface.

=back

=head1 OPTIONS

=over

=item --id <name>

interface name to work on.

=item --rx-pause (true|false)

Activate or shutdown receive pause.

=item --tx-pause (true|false)

Activate or shutdown transmit pause.

=item --autoneg (true|false)

Activate or shutdown pause settings autonegotiation.

=back

=head1 EXAMPLES

=over

=item dabba interface pause get

Output the pause settings of all available network interfaces.

=item dabba interface pause get --id eth0

Output the pause settings of 'eth0'.

=item dabba interface pause modify --id eth0 --tx-pause true

Enable transmit pause settings on 'eth0' (if supported).

=item dabba interface pause modify --id eth0 --autoneg false

Disable pause settings autonegotiation on 'eth0' (if supported).

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
#include <getopt.h>
#include <inttypes.h>
#include <assert.h>
#include <errno.h>
#include <libdabba/macros.h>
#include <dabba/macros.h>
#include <dabba/cli.h>
#include <dabba/rpc.h>
#include <dabba/help.h>
#include <dabba/interface.h>
#include <dabba/dabba.h>

/**
 * \internal
 * \brief Protobuf closure to print interface pause list in YAML
 * \param[in]           result	        Pointer to interface pause list
 * \param[in]           closure_data	Pointer to protobuf closure data
 */

static void interface_pause_list_print(const Dabba__InterfacePauseList *
				       result, void *closure_data)
{
	Dabba__InterfacePause *pausep;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;
	size_t a;

	assert(closure_data);

	rpc_header_print("interfaces");

	for (a = 0; result && a < result->n_list; a++) {
		pausep = result->list[a];
		printf("    - name: %s\n", pausep->id->name);
		printf("    ");
		__rpc_error_code_print(pausep->status->code);
		printf("      pause:\n");
		printf("        autoneg: %s\n", print_tf(pausep->autoneg));
		printf("        rx: %s\n", print_tf(pausep->rx_pause));
		printf("        tx: %s\n", print_tf(pausep->tx_pause));
	}

	*status = 1;
}

/**
 * \internal
 * \brief Invoke interface pause get RPC
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           id_list         Pointer to interface id to fetch
 * \return Always returns 0.
 * \note An empty id list will query the pause status of all available interfaces.
 */

static int rpc_interface_pause_get(ProtobufCService * service,
				   const Dabba__InterfaceIdList * id_list)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(id_list);

	dabba__dabba_service__interface_pause_get(service, id_list,
						  interface_pause_list_print,
						  &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

/**
 * \internal
 * \brief Invoke interface pause modify RPC
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           pausep        Pointer to interface new pause settings
 * \return Always returns 0.
 */

static int rpc_interface_pause_modify(ProtobufCService * service,
				      const Dabba__InterfacePause * pausep)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(pausep);

	dabba__dabba_service__interface_pause_modify(service, pausep,
						     rpc_error_code_print,
						     &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

/**
 * \brief Prepare interface pause modify RPC from \c argv
 * \param[in]           argc	        Argument counter
 * \param[in]           argv	        Argument vector
 * \return Returns 0 on success, else otherwise.
 */

static int cmd_interface_pause_modify(int argc, const char **argv)
{
	enum interface_option {
		/* option */
		OPT_INTERFACE_RX_PAUSE,
		OPT_INTERFACE_TX_PAUSE,
		OPT_INTERFACE_AUTONEG,
		OPT_INTERFACE_ID,
		OPT_TCP,
		OPT_LOCAL,
		OPT_HELP
	};

	const struct option interface_option[] = {
		{"id", required_argument, NULL, OPT_INTERFACE_ID},
		{"rx-pause", required_argument, NULL, OPT_INTERFACE_RX_PAUSE},
		{"tx-pause", required_argument, NULL, OPT_INTERFACE_TX_PAUSE},
		{"autoneg", required_argument, NULL, OPT_INTERFACE_AUTONEG},
		{"tcp", optional_argument, NULL, OPT_TCP},
		{"local", optional_argument, NULL, OPT_LOCAL},
		{"help", no_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0},
	};

	int ret, rc = 0;
	const char *server_id = DABBA_RPC_DEFAULT_LOCAL_SERVER_NAME;
	ProtobufC_RPC_AddressType server_type = PROTOBUF_C_RPC_ADDRESS_LOCAL;
	ProtobufCService *service;
	Dabba__InterfacePause pause = DABBA__INTERFACE_PAUSE__INIT;
	Dabba__ErrorCode err = DABBA__ERROR_CODE__INIT;

	/* HACK: getopt*() start to parse options at argv[1] */
	argc++;
	argv--;

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
			pause.id = malloc(sizeof(*pause.id));

			if (!pause.id)
				return ENOMEM;

			dabba__interface_id__init(pause.id);
			pause.id->name = optarg;
			break;

		case OPT_INTERFACE_RX_PAUSE:
			rc = str2bool(optarg, &pause.rx_pause);

			if (rc)
				goto out;

			pause.has_rx_pause = 1;
			break;
		case OPT_INTERFACE_TX_PAUSE:
			rc = str2bool(optarg, &pause.tx_pause);

			if (rc)
				goto out;

			pause.has_tx_pause = 1;
			break;
		case OPT_INTERFACE_AUTONEG:
			rc = str2bool(optarg, &pause.autoneg);

			if (rc)
				goto out;

			pause.has_autoneg = 1;
			break;
		case OPT_HELP:
		default:
			show_usage(interface_option);
			rc = -1;
			goto out;
		}
	}

	pause.status = &err;

	service = dabba_rpc_client_connect(server_id, server_type);

	if (service)
		rc = rpc_interface_pause_modify(service, &pause);
	else
		rc = EINVAL;
 out:
	free(pause.id);
	return rc;
}

static int cmd_interface_pause_get(int argc, const char **argv)
{
	return rpc_interface_get(argc, argv, rpc_interface_pause_get);
}

int cmd_interface_pause(int argc, const char **argv)
{
	static const struct cmd_struct cmd[] = {
		{"get", cmd_interface_pause_get},
		{"modify", cmd_interface_pause_modify}
	};

	return cmd_run_builtin(cmd, ARRAY_SIZE(cmd), argc, argv);
}
