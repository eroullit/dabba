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

/*

=head1 NAME

dabba-interface-coalesce - Manage network interface coalesce settings

=head1 SYNOPSIS

dabba interface coalesce <command> [<arguments>...] [--help]

=head1 DESCRIPTION

Give the user the possibility to manage the coalesce settings of supported
network interfaces.

=head1 COMMANDS

=over

=item get

Fetch and print coalesce information about currently supported interfaces.
The output is formatted in YAML.

=item modify

Apply new coalesce settings to a specific network interface.

=back

=head1 OPTIONS

=over

=item --id <name>

interface name to work on.

=item --rx-usecs <time-interval>

Time interval in microseconds to delay a receive interrupt after a packet arrives

=item --rx-usecs-irq <time-interval>

Same as --rx-usecs, except that this value applies while an interrupt is being serviced by the host.

=item --rx-usecs-low <time-interval>

Time interval in microseconds to delay a receive interrupt after a packet arrives,
when the packet rate is below "packet rate low".

=item --rx-usecs-high <time-interval>

Time interval in microseconds to delay a receive interrupt after a packet arrives,
when the packet rate is above "packet rate high".

=item --rx-frames <frame-interval>

Maximum number of packets to receive before a receive interrupt is triggered.

=item --rx-frames-irq <frame-interval>

Same as --rx-frames, except that this value applies while an interrupt is being serviced by the host.

=item --rx-frames-low <frame-interval>

Number of frames to delay a receive interrupt after a packet arrives,
when the packet rate is below "packet rate low".

=item --rx-frames-high <frame-interval>

Number of frames to delay a receive interrupt after a packet arrives,
when the packet rate is above "packet rate high".

=item --tx-usecs <time-interval>

Time interval in microseconds to delay a transmit interrupt after a packet arrives

=item --tx-usecs-irq <time-interval>

Same as --tx-usecs, except that this value applies while an interrupt is being serviced by the host.

=item --tx-usecs-low <time-interval>

Time interval in microseconds to delay a transmit interrupt after a packet arrives,
when the packet rate is below "packet rate low".

=item --tx-usecs-high <time-interval>

Time interval in microseconds to delay a transmit interrupt after a packet arrives,
when the packet rate is above "packet rate high".

=item --tx-frames <frame-interval>

Maximum number of packets to transmit before a transmit interrupt is triggered.

=item --tx-frames-irq <frame-interval>

Same as --tx-frames, except that this value applies while an interrupt is being serviced by the host.

=item --tx-frames-low <frame-interval>

Number of frames to delay a transmit interrupt after a packet arrives,
when the packet rate is below "packet rate low".

=item --tx-frames-high <frame-interval>

Number of frames to delay a transmit interrupt after a packet arrives,
when the packet rate is above "packet rate high".

=item --adaptive-rx (true|false)

Activate or shutdown adaptive receive coalescing.

=item --adaptive-tx (true|false)

Activate or shutdown adaptive transmit coalescing.

=item --stats-block-usecs <time-interval>

How many usecs to delay in-memory statistics block updates.
Some drivers do not have an in-memory statistic block, 
and in such cases this value is ignored. This value must not be zero.

=item --sample-interval <time-interval>

How often to do adaptive coalescing packet rate sampling, measured in seconds. This value must not be zero.

=item --tcp[=<hostname>:<port>]

Query a running instance of dabbad using a TCP socket (default: localhost:55994)

=item --local[=<path>]

Query a running instance of dabbad using a Unix domain socket (default: /tmp/dabba)

=item --help

Prints the help message on the terminal

=back

=head1 EXAMPLES

=over

=item dabba interface coalesce get

Output the coalesce settings of all available network interfaces.

=item dabba interface coalesce get --id eth0

Output the coalesce settings of 'eth0'.

=item dabba interface coalesce modify --id eth0 --tx-frames 128

Set the transmit interrupt coalescing to 128 frames on 'eth0' (if supported).

=item dabba interface coalesce modify --id eth0 --adaptive-rx true

Enable adaptive receive interrupt coalescing on 'eth0' (if supported).

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
#include <assert.h>
#include <errno.h>
#include <libdabba/macros.h>
#include <dabba/rpc.h>
#include <dabba/cli.h>
#include <dabba/help.h>
#include <dabba/macros.h>
#include <dabba/interface.h>
#include <dabba/dabba.h>

/**
 * \internal
 * \brief Protobuf closure to print interface coalesce list in YAML
 * \param[in]           result	        Pointer to interface coalesce list
 * \param[in]           closure_data	Pointer to protobuf closure data
 */

static void interface_coalesce_list_print(const Dabba__InterfaceCoalesceList *
					  result, void *closure_data)
{
	Dabba__InterfaceCoalesce *coalescep;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;
	size_t a;

	assert(closure_data);

	rpc_header_print("interfaces");

	for (a = 0; result && a < result->n_list; a++) {
		coalescep = result->list[a];
		printf("    - name: %s\n", coalescep->id->name);
		printf("    ");
		__rpc_error_code_print(coalescep->status->code);
		printf("      coalesce:\n");
		printf("        packet rate high: %u\n",
		       coalescep->pkt_rate_high);
		printf("        packet rate low: %u\n",
		       coalescep->pkt_rate_low);
		printf("        rate sample interval: %u\n",
		       coalescep->rate_sample_interval);
		printf("        stats block: %u\n",
		       coalescep->stats_block_coalesce_usecs);
		printf("        rx:\n");
		printf("            adaptive: %s\n",
		       print_tf(coalescep->use_adaptive_rx_coalesce));
		printf("            usec: {");
		printf("normal: %u, ", coalescep->rx_coalesce_usecs);
		printf("irq: %u, ", coalescep->rx_coalesce_usecs_irq);
		printf("high: %u, ", coalescep->rx_coalesce_usecs_high);
		printf("low: %u", coalescep->rx_coalesce_usecs_low);
		printf("}\n");
		printf("            max frame: {");
		printf("normal: %u, ", coalescep->rx_max_coalesced_frames);
		printf("irq: %u, ", coalescep->rx_max_coalesced_frames_irq);
		printf("high: %u, ", coalescep->rx_max_coalesced_frames_high);
		printf("low: %u", coalescep->rx_max_coalesced_frames_low);
		printf("}\n");
		printf("        tx:\n");
		printf("            adaptive: %s\n",
		       print_tf(coalescep->use_adaptive_tx_coalesce));
		printf("            usec: {");
		printf("normal: %u, ", coalescep->tx_coalesce_usecs);
		printf("irq: %u, ", coalescep->tx_coalesce_usecs_irq);
		printf("high: %u, ", coalescep->tx_coalesce_usecs_high);
		printf("low: %u", coalescep->tx_coalesce_usecs_low);
		printf("}\n");
		printf("            max frame: {");
		printf("normal: %u, ", coalescep->tx_max_coalesced_frames);
		printf("irq: %u, ", coalescep->tx_max_coalesced_frames_irq);
		printf("high: %u, ", coalescep->tx_max_coalesced_frames_high);
		printf("low: %u", coalescep->tx_max_coalesced_frames_low);
		printf("}\n");
	}

	*status = 1;
}

/**
 * \internal
 * \brief Invoke interface coalesce get RPC
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           id_list         Pointer to interface id to fetch
 * \return Always returns 0.
 * \note An empty id list will query the coalesce status of all available interfaces.
 */

static int rpc_interface_coalesce_get(ProtobufCService * service,
				      const Dabba__InterfaceIdList * id_list)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(id_list);

	dabba__dabba_service__interface_coalesce_get(service, id_list,
						     interface_coalesce_list_print,
						     &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

/**
 * \internal
 * \brief Invoke interface coalesce modify RPC
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           coalesce        Pointer to interface new coalesce settings
 * \return Always returns 0.
 */

static int rpc_interface_coalesce_modify(ProtobufCService * service,
					 const Dabba__InterfaceCoalesce *
					 coalescep)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(coalescep);

	dabba__dabba_service__interface_coalesce_modify(service, coalescep,
							rpc_error_code_print,
							&is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

/**
 * \brief Prepare interface coalesce modify RPC from \c argv
 * \param[in]           argc	        Argument counter
 * \param[in]           argv	        Argument vector
 * \return Returns 0 on success, else otherwise.
 */

static int cmd_interface_coalesce_modify(int argc, const char **argv)
{
	enum interface_option {
		OPT_ADAPTIVE_RX,
		OPT_ADAPTIVE_TX,
		OPT_RX_USECS,
		OPT_RX_USECS_IRQ,
		OPT_RX_USECS_LOW,
		OPT_RX_USECS_HIGH,
		OPT_RX_FRAMES,
		OPT_RX_FRAMES_IRQ,
		OPT_RX_FRAMES_LOW,
		OPT_RX_FRAMES_HIGH,
		OPT_TX_USECS,
		OPT_TX_USECS_IRQ,
		OPT_TX_USECS_LOW,
		OPT_TX_USECS_HIGH,
		OPT_TX_FRAMES,
		OPT_TX_FRAMES_IRQ,
		OPT_TX_FRAMES_LOW,
		OPT_TX_FRAMES_HIGH,
		OPT_PACKET_RATE_HIGH,
		OPT_PACKET_RATE_LOW,
		OPT_SAMPLE_INTERVAL,
		OPT_STATS_BLOCK,
		OPT_INTERFACE_ID,
		OPT_TCP,
		OPT_LOCAL,
		OPT_HELP
	};

	const struct option interface_option[] = {
		{"adaptive-rx", required_argument, NULL, OPT_ADAPTIVE_RX},
		{"adaptive-tx", required_argument, NULL, OPT_ADAPTIVE_TX},
		{"rx-usecs", required_argument, NULL, OPT_RX_USECS},
		{"rx-usecs-irq", required_argument, NULL, OPT_RX_USECS_IRQ},
		{"rx-usecs-low", required_argument, NULL, OPT_RX_USECS_LOW},
		{"rx-usecs-high", required_argument, NULL, OPT_RX_USECS_HIGH},
		{"rx-frames", required_argument, NULL, OPT_RX_FRAMES},
		{"rx-frames-irq", required_argument, NULL, OPT_RX_FRAMES_IRQ},
		{"rx-frames-low", required_argument, NULL, OPT_RX_FRAMES_LOW},
		{"rx-frames-high", required_argument, NULL, OPT_RX_FRAMES_HIGH},
		{"tx-usecs", required_argument, NULL, OPT_TX_USECS},
		{"tx-usecs-irq", required_argument, NULL, OPT_TX_USECS_IRQ},
		{"tx-usecs-low", required_argument, NULL, OPT_TX_USECS_LOW},
		{"tx-usecs-high", required_argument, NULL, OPT_TX_USECS_HIGH},
		{"tx-frames", required_argument, NULL, OPT_TX_FRAMES},
		{"tx-frames-irq", required_argument, NULL, OPT_TX_FRAMES_IRQ},
		{"tx-frames-low", required_argument, NULL, OPT_TX_FRAMES_LOW},
		{"tx-frames-high", required_argument, NULL, OPT_TX_FRAMES_HIGH},
		{"pkt-rate-low", required_argument, NULL, OPT_PACKET_RATE_LOW},
		{"pkt-rate-high", required_argument, NULL,
		 OPT_PACKET_RATE_HIGH},
		{"stats-block-usecs", required_argument, NULL, OPT_STATS_BLOCK},
		{"sample-interval", required_argument, NULL,
		 OPT_SAMPLE_INTERVAL},
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
	Dabba__InterfaceCoalesce coalesce = DABBA__INTERFACE_COALESCE__INIT;
	Dabba__ErrorCode err = DABBA__ERROR_CODE__INIT;

	/* HACK: getopt*() start to parse options at argv[1] */
	argc++;
	argv--;

	while ((ret =
		getopt_long_only(argc, (char **)argv, "", interface_option,
				 NULL)) != EOF) {
		switch (ret) {
		case OPT_ADAPTIVE_RX:
			rc = str2bool(optarg,
				      &coalesce.use_adaptive_rx_coalesce);

			if (rc)
				goto out;

			coalesce.has_use_adaptive_rx_coalesce = 1;
			break;
		case OPT_ADAPTIVE_TX:
			rc = str2bool(optarg,
				      &coalesce.use_adaptive_tx_coalesce);

			if (rc)
				goto out;

			coalesce.has_use_adaptive_tx_coalesce = 1;
			break;
		case OPT_RX_USECS:
			coalesce.rx_coalesce_usecs = strtoul(optarg, NULL, 10);
			coalesce.has_rx_coalesce_usecs = 1;
			break;
		case OPT_RX_USECS_IRQ:
			coalesce.rx_coalesce_usecs_irq =
			    strtoul(optarg, NULL, 10);
			coalesce.has_rx_coalesce_usecs_irq = 1;
			break;
		case OPT_RX_USECS_HIGH:
			coalesce.rx_coalesce_usecs_high =
			    strtoul(optarg, NULL, 10);
			coalesce.has_rx_coalesce_usecs_high = 1;
			break;
		case OPT_RX_USECS_LOW:
			coalesce.rx_coalesce_usecs_low =
			    strtoul(optarg, NULL, 10);
			coalesce.has_rx_coalesce_usecs_low = 1;
			break;
		case OPT_RX_FRAMES:
			coalesce.rx_max_coalesced_frames =
			    strtoul(optarg, NULL, 10);
			coalesce.has_rx_max_coalesced_frames = 1;
			break;
		case OPT_RX_FRAMES_IRQ:
			coalesce.rx_max_coalesced_frames_irq =
			    strtoul(optarg, NULL, 10);
			coalesce.has_rx_max_coalesced_frames_irq = 1;
			break;
		case OPT_RX_FRAMES_HIGH:
			coalesce.rx_max_coalesced_frames_high =
			    strtoul(optarg, NULL, 10);
			coalesce.has_rx_max_coalesced_frames_high = 1;
			break;
		case OPT_RX_FRAMES_LOW:
			coalesce.rx_max_coalesced_frames_low =
			    strtoul(optarg, NULL, 10);
			coalesce.has_rx_max_coalesced_frames_low = 1;
			break;
		case OPT_TX_USECS:
			coalesce.tx_coalesce_usecs = strtoul(optarg, NULL, 10);
			coalesce.has_tx_coalesce_usecs = 1;
			break;
		case OPT_TX_USECS_IRQ:
			coalesce.tx_coalesce_usecs_irq =
			    strtoul(optarg, NULL, 10);
			coalesce.has_tx_coalesce_usecs_irq = 1;
			break;
		case OPT_TX_USECS_HIGH:
			coalesce.tx_coalesce_usecs_high =
			    strtoul(optarg, NULL, 10);
			coalesce.has_tx_coalesce_usecs_high = 1;
			break;
		case OPT_TX_USECS_LOW:
			coalesce.tx_coalesce_usecs_low =
			    strtoul(optarg, NULL, 10);
			coalesce.has_tx_coalesce_usecs_low = 1;
			break;
		case OPT_TX_FRAMES:
			coalesce.tx_max_coalesced_frames =
			    strtoul(optarg, NULL, 10);
			coalesce.has_tx_max_coalesced_frames = 1;
			break;
		case OPT_TX_FRAMES_IRQ:
			coalesce.tx_max_coalesced_frames_irq =
			    strtoul(optarg, NULL, 10);
			coalesce.has_tx_max_coalesced_frames_irq = 1;
			break;
		case OPT_TX_FRAMES_HIGH:
			coalesce.tx_max_coalesced_frames_high =
			    strtoul(optarg, NULL, 10);
			coalesce.has_tx_max_coalesced_frames_high = 1;
			break;
		case OPT_TX_FRAMES_LOW:
			coalesce.tx_max_coalesced_frames_low =
			    strtoul(optarg, NULL, 10);
			coalesce.has_tx_max_coalesced_frames_low = 1;
			break;
		case OPT_PACKET_RATE_HIGH:
			coalesce.pkt_rate_high = strtoul(optarg, NULL, 10);
			coalesce.has_pkt_rate_high = 1;
			break;
		case OPT_PACKET_RATE_LOW:
			coalesce.pkt_rate_low = strtoul(optarg, NULL, 10);
			coalesce.has_pkt_rate_low = 1;
			break;
		case OPT_SAMPLE_INTERVAL:
			coalesce.rate_sample_interval =
			    strtoul(optarg, NULL, 10);
			coalesce.has_rate_sample_interval = 1;
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
			coalesce.id = malloc(sizeof(*coalesce.id));

			if (!coalesce.id)
				return ENOMEM;

			dabba__interface_id__init(coalesce.id);
			coalesce.id->name = optarg;
			break;
		case OPT_HELP:
		default:
			show_usage(interface_option);
			rc = -1;
			goto out;
		}
	}

	coalesce.status = &err;

	service = dabba_rpc_client_connect(server_id, server_type);

	if (service)
		rc = rpc_interface_coalesce_modify(service, &coalesce);
	else
		rc = EINVAL;
 out:
	free(coalesce.id);
	return rc;
}

static int cmd_interface_coalesce_get(int argc, const char **argv)
{
	return rpc_interface_get(argc, argv, rpc_interface_coalesce_get);
}

int cmd_interface_coalesce(int argc, const char **argv)
{
	static const struct cmd_struct cmd[] = {
		{"get", cmd_interface_coalesce_get},
		{"modify", cmd_interface_coalesce_modify}
	};

	return cmd_run_builtin(cmd, ARRAY_SIZE(cmd), argc, argv);
}
