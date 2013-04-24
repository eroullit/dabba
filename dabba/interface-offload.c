/**
 * \file interface-offload.c
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

dabba-interface-offload - Manage network interface offload settings

=head1 SYNOPSIS

dabba interface offload <command> [<arguments>...] [--help]

=head1 DESCRIPTION

Give the user the possibility to manage the offload settings of supported
network interfaces.

=head1 COMMANDS

=over

=item get

Fetch and print offload information about currently supported interfaces.
The output is formatted in YAML.

=item modify

Apply new offload settings to a specific network interface.

=back

=head1 OPTIONS

=over

=item --id <name>

interface name to work on.

=item --rx-csum (true|false)

Activate or shutdown receive checksum offload.

=item --tx-csum (true|false)

Activate or shutdown transmit checksum offload.

=item --sg (true|false)

Activate or shutdown scatter gather.

=item --tso (true|false)

Activate or shutdown TCP segment offload.

=item --ufo (true|false)

Activate or shutdown UDP fragment offload.

=item --gso (true|false)

Activate or shutdown generic segmentation offload.

=item --gro (true|false)

Activate or shutdown generic receive offload.

=item --lro (true|false)

Activate or shutdown large receive offload.

=item --rx-hash (true|false)

Activate or shutdown receive hash offload.

=item --tcp[=<hostname>:<port>]

Query a running instance of dabbad using a TCP socket (default: localhost:55994)

=item --local[=<path>]

Query a running instance of dabbad using a Unix domain socket (default: /tmp/dabba)

=item --help

Prints the help message on the terminal

=back

=head1 EXAMPLES

=over

=item dabba interface offload get

Output the offload settings of all available network interfaces.

=item dabba interface offload get --id eth0

Output the offload settings of 'eth0'.

=item dabba interface offload modify --id eth0 --lro true

Enable the large receive offload on 'eth0' (if supported).

=item dabba interface offload modify --id eth0 --tso false

Disable TCP segment offload on 'eth0'.

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
 * \brief Protobuf closure to print interface offload list in YAML
 * \param[in]           result	        Pointer to interface offload list
 * \param[in]           closure_data	Pointer to protobuf closure data
 */

static void interface_offload_list_print(const Dabba__InterfaceOffloadList *
					 result, void *closure_data)
{
	Dabba__InterfaceOffload *offloadp;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;
	size_t a;

	assert(closure_data);

	rpc_header_print("interfaces");

	for (a = 0; result && a < result->n_list; a++) {
		offloadp = result->list[a];
		printf("    - name: %s\n", offloadp->id->name);
		printf("    ");
		__rpc_error_code_print(offloadp->status->code);
		printf("      offload:\n");
		printf("        rx checksum: %s\n",
		       print_tf(offloadp->rx_csum));
		printf("        tx checksum: %s\n",
		       print_tf(offloadp->tx_csum));
		printf("        scatter gather: %s\n", print_tf(offloadp->sg));
		printf("        tcp segment: %s\n", print_tf(offloadp->tso));
		printf("        udp fragment: %s\n", print_tf(offloadp->ufo));
		printf("        generic segmentation: %s\n",
		       print_tf(offloadp->gso));
		printf("        generic receive: %s\n",
		       print_tf(offloadp->gro));
		printf("        rx hashing: %s\n", print_tf(offloadp->rxhash));
	}

	*status = 1;
}

/**
 * \internal
 * \brief Invoke interface offload get RPC
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           id_list         Pointer to interface id to fetch
 * \return Always returns 0.
 * \note An empty id list will query the offload status of all available interfaces.
 */

static int rpc_interface_offload_get(ProtobufCService * service,
				     const Dabba__InterfaceIdList * id_list)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(id_list);

	dabba__dabba_service__interface_offload_get(service, id_list,
						    interface_offload_list_print,
						    &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

static int rpc_interface_offload_modify(ProtobufCService * service,
					const Dabba__InterfaceOffload *
					offloadp)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(offloadp);

	dabba__dabba_service__interface_offload_modify(service, offloadp,
						       rpc_error_code_print,
						       &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

static int cmd_interface_offload_modify(int argc, const char **argv)
{
	enum interface_option {
		/* option */
		OPT_INTERFACE_ID,
		OPT_RX_CSUM_OFFLOAD,
		OPT_TX_CSUM_OFFLOAD,
		OPT_SG,
		OPT_TSO,
		OPT_UFO,
		OPT_GSO,
		OPT_GRO,
		OPT_LRO,
		OPT_RXHASH_OFFLOAD,
		OPT_TCP,
		OPT_LOCAL,
		OPT_HELP
	};

	const struct option interface_option[] = {
		{"rx-csum", required_argument, NULL, OPT_RX_CSUM_OFFLOAD},
		{"tx-csum", required_argument, NULL, OPT_TX_CSUM_OFFLOAD},
		{"sg", required_argument, NULL, OPT_SG},
		{"tso", required_argument, NULL, OPT_TSO},
		{"ufo", required_argument, NULL, OPT_UFO},
		{"gso", required_argument, NULL, OPT_GSO},
		{"gro", required_argument, NULL, OPT_GRO},
		{"lro", required_argument, NULL, OPT_LRO},
		{"rxhash", required_argument, NULL, OPT_RXHASH_OFFLOAD},
		{"id", required_argument, NULL, OPT_INTERFACE_ID},
		{"tcp", optional_argument, NULL, OPT_TCP},
		{"local", optional_argument, NULL, OPT_LOCAL},
		{"help", no_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0},
	};

	int ret, rc = 0;
	const char *server_id = DABBA_RPC_DEFAULT_LOCAL_SERVER_NAME;
	ProtobufC_RPC_AddressType server_type = PROTOBUF_C_RPC_ADDRESS_LOCAL;
	ProtobufCService *service;
	Dabba__InterfaceOffload offload = DABBA__INTERFACE_OFFLOAD__INIT;
	Dabba__ErrorCode err = DABBA__ERROR_CODE__INIT;

	/* HACK: getopt*() start to parse options at argv[1] */
	argc++;
	argv--;

	while ((ret =
		getopt_long_only(argc, (char **)argv, "", interface_option,
				 NULL)) != EOF) {
		switch (ret) {
		case OPT_RX_CSUM_OFFLOAD:
			rc = str2bool(optarg, &offload.rx_csum);

			if (rc)
				goto out;

			offload.has_rx_csum = 1;
			break;
		case OPT_TX_CSUM_OFFLOAD:
			rc = str2bool(optarg, &offload.tx_csum);

			if (rc)
				goto out;

			offload.has_tx_csum = 1;
			break;
		case OPT_SG:
			rc = str2bool(optarg, &offload.sg);

			if (rc)
				goto out;

			offload.has_sg = 1;
			break;
		case OPT_TSO:
			rc = str2bool(optarg, &offload.tso);

			if (rc)
				goto out;

			offload.has_tso = 1;
			break;
		case OPT_UFO:
			rc = str2bool(optarg, &offload.ufo);

			if (rc)
				goto out;

			offload.has_ufo = 1;
			break;
		case OPT_GSO:
			rc = str2bool(optarg, &offload.gso);

			if (rc)
				goto out;

			offload.has_gso = 1;
			break;
		case OPT_GRO:
			rc = str2bool(optarg, &offload.gro);

			if (rc)
				goto out;

			offload.has_gro = 1;
			break;
		case OPT_LRO:
			rc = str2bool(optarg, &offload.lro);

			if (rc)
				goto out;

			offload.has_lro = 1;
			break;
		case OPT_RXHASH_OFFLOAD:
			rc = str2bool(optarg, &offload.rxhash);

			if (rc)
				goto out;

			offload.has_rxhash = 1;
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
			offload.id = malloc(sizeof(*offload.id));

			if (!offload.id)
				return ENOMEM;

			dabba__interface_id__init(offload.id);
			offload.id->name = optarg;
			break;

		case OPT_HELP:
		default:
			show_usage(interface_option);
			rc = -1;
			goto out;
		}
	}

	offload.status = &err;

	service = dabba_rpc_client_connect(server_id, server_type);

	if (service)
		rc = rpc_interface_offload_modify(service, &offload);
	else
		rc = EINVAL;
 out:
	free(offload.id);
	return rc;
}

static int cmd_interface_offload_get(int argc, const char **argv)
{
	return rpc_interface_get(argc, argv, rpc_interface_offload_get);
}

int cmd_interface_offload(int argc, const char **argv)
{
	static const struct cmd_struct cmd[] = {
		{"get", cmd_interface_offload_get},
		{"modify", cmd_interface_offload_modify}
	};

	return cmd_run_builtin(cmd, ARRAY_SIZE(cmd), argc, argv);
}
