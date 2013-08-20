/**
 * \file interface-driver.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (C) 2013
 * \date 2013
 */


/*

=head1 NAME

dabba-interface-driver - Manage network interface driver settings

=head1 SYNOPSIS

dabba interface driver <action> [<arguments>...] [--help]

=head1 DESCRIPTION

Give the user the possibility to manage the driver settings of supported
network interfaces.

=head1 ACTIONS

=over

=item get

Fetch and print driver information about currently supported interfaces.
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

=item dabba interface driver get

Output the driver settings of all available network interfaces.

=item dabba interface driver get --id eth0

Output the driver settings of 'eth0'.

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

=item Copyright (C) 2013 Emmanuel Roullit.

=item License MIT: <www.opensource.org/licenses/MIT>

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
#include <dabba/help.h>
#include <dabba/interface.h>
#include <dabba/dabba.h>

/**
 * \internal
 * \brief Protobuf closure to print interface driver list in YAML
 * \param[in]           result	        Pointer to interface driver list
 * \param[in]           closure_data	Pointer to protobuf closure data
 */

static void interface_driver_list_print(const Dabba__InterfaceDriverList *
					result, void *closure_data)
{
	Dabba__InterfaceDriver *driverp;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;
	size_t a;

	assert(closure_data);

	rpc_header_print("interfaces");

	for (a = 0; result && a < result->n_list; a++) {
		driverp = result->list[a];
		printf("    - name: %s\n", driverp->id->name);
		printf("    ");
		__rpc_error_code_print(driverp->status->code);
		printf("      driver name: %s\n", driverp->name);
		printf("      driver version: %s\n", driverp->version);
		printf("      firmware version: %s\n", driverp->fw_version);
		printf("      bus info: %s\n", driverp->bus_info);
	}

	*status = 1;
}

/**
 * \internal
 * \brief Invoke interface driver get RPC
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           id_list         Pointer to interface id to fetch
 * \return Always returns 0.
 * \note An empty id list will query the driver information of all available interfaces.
 */

static int rpc_interface_driver_get(ProtobufCService * service,
				    const Dabba__InterfaceIdList * id_list)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(id_list);

	dabba__dabba_service__interface_driver_get(service, id_list,
						   interface_driver_list_print,
						   &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

static int cmd_interface_driver_get(int argc, const char **argv)
{
	return rpc_interface_get(argc, argv, rpc_interface_driver_get);
}

int cmd_interface_driver(int argc, const char **argv)
{
	static const struct cmd_struct cmd[] = {
		{"get", cmd_interface_driver_get}
	};

	return cmd_run_action(cmd, ARRAY_SIZE(cmd), argc, argv);
}
