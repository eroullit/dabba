/**
 * \file interface-statistics.c
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

dabba-interface-statistics - Manage network interface traffic statistics

=head1 SYNOPSIS

dabba interface statistics <action> [<arguments>...] [--help]

=head1 DESCRIPTION

Give the user the possibility to manage the statistics of supported
network interfaces.

=head1 ACTIONS

=over

=item get

Fetch and print statistic information about currently supported interfaces.
The output is formatted in YAML.

=back

=head1 OPTIONS

=over

=item --id <name>

interface name to work on.

=item --tcp[=<hostname>:<port>]

Query a running instance of dabbad using a TCP socket (default: localhost:55994)

=item --local[=<path>]

Query a running instance of dabbad using a Unix domain socket (default: /tmp/dabba)

=item --help

Prints the help message on the terminal

=back

=head1 EXAMPLES

=over

=item dabba interface statistics get

Output the traffic statistics of all available network interfaces.

=item dabba interface statistics get --id eth0

Output the traffic statistics of 'eth0'.

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
#include <inttypes.h>
#include <assert.h>
#include <errno.h>
#include <libdabba/macros.h>
#include <dabba/macros.h>
#include <dabba/rpc.h>
#include <dabba/help.h>
#include <dabba/interface.h>
#include <dabba/dabba.h>

/**
 * \internal
 * \brief Protobuf closure to print interface statistics list in YAML
 * \param[in]           result	        Pointer to interface statistics list
 * \param[in]           closure_data	Pointer to protobuf closure data
 */

static void interface_statistics_list_print(const Dabba__InterfaceStatisticsList
					    * result, void *closure_data)
{
	Dabba__InterfaceStatistics *statisticsp;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;
	size_t a;

	assert(closure_data);

	rpc_header_print("interfaces");

	for (a = 0; result && a < result->n_list; a++) {
		statisticsp = result->list[a];
		printf("    - name: %s\n", statisticsp->id->name);
		printf("    ");
		__rpc_error_code_print(statisticsp->status->code);
		printf("      statistics:\n");
		printf("          rx: {");
		printf("byte: %" PRIu64 ", ", statisticsp->rx_byte);
		printf("packet: %" PRIu64 ", ", statisticsp->rx_packet);
		printf("error: %" PRIu64 ", ", statisticsp->rx_error);
		printf("dropped: %" PRIu64 ", ", statisticsp->rx_dropped);
		printf("compressed: %" PRIu64 "", statisticsp->rx_compressed);
		printf("}\n");
		printf("          tx: {");
		printf("byte: %" PRIu64 ", ", statisticsp->tx_byte);
		printf("packet: %" PRIu64 ", ", statisticsp->tx_packet);
		printf("error: %" PRIu64 ", ", statisticsp->tx_error);
		printf("dropped: %" PRIu64 ", ", statisticsp->tx_dropped);
		printf("compressed: %" PRIu64 "", statisticsp->tx_compressed);
		printf("}\n");
		printf("          rx error: {");
		printf("fifo: %" PRIu64 ", ", statisticsp->rx_error_fifo);
		printf("frame: %" PRIu64 ", ", statisticsp->rx_error_frame);
		printf("crc: %" PRIu64 ", ", statisticsp->rx_error_crc);
		printf("length: %" PRIu64 ", ", statisticsp->rx_error_length);
		printf("missed: %" PRIu64 ", ", statisticsp->rx_error_missed);
		printf("overflow: %" PRIu64 "", statisticsp->rx_error_over);
		printf("}\n");
		printf("          tx error: {");
		printf("fifo: %" PRIu64 ", ", statisticsp->tx_error_fifo);
		printf("carrier: %" PRIu64 ", ", statisticsp->tx_error_carrier);
		printf("heartbeat: %" PRIu64 ", ",
		       statisticsp->tx_error_heartbeat);
		printf("window: %" PRIu64 ", ", statisticsp->tx_error_window);
		printf("aborted: %" PRIu64 "", statisticsp->tx_error_aborted);
		printf("}\n");
	}

	*status = 1;
}

/**
 * \internal
 * \brief Invoke interface statistics get RPC
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           id_li
 * st         Pointer to interface id to fetch
 * \return Always returns 0.
 * \note An empty id list will query the statistics counters of all available interfaces.
 */

static int rpc_interface_statistics_get(ProtobufCService * service,
					const Dabba__InterfaceIdList * id_list)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(id_list);

	dabba__dabba_service__interface_statistics_get(service, id_list,
						       interface_statistics_list_print,
						       &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

static int cmd_interface_statistics_get(int argc, const char **argv)
{
	return rpc_interface_get(argc, argv, rpc_interface_statistics_get);
}

int cmd_interface_statistics(int argc, const char **argv)
{
	static const struct cmd_struct cmd[] = {
		{"get", cmd_interface_statistics_get}
	};

	return cmd_run_builtin(cmd, ARRAY_SIZE(cmd), argc, argv);
}
