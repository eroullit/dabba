/**
 * \file interface.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (C) 2013
 * \date 2013
 */


/*

=head1 NAME

dabba-interface - Manage network interface

=head1 SYNOPSIS

dabba [--help] interface <subcommand> <action> [<arguments>...]

=head1 DESCRIPTION

Give the user the possibility to manage the available network interfaces.

=head1 SUBCOMMANDS

=over

=item status

Output network interfaces status information.

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

=back

=head1 ACTIONS

Most of the subcommands have the following actions available.
For more information, check the subcommand related manpage.

=over

=item get

Fetch interface information and print it on stdout in YAML format.

=item modify

Modify interface settings depending on given parameters.

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
#include <string.h>
#include <inttypes.h>
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

/**
 * \brief Selects interface get RPC from \c argv
 * \param[in]           argc	        Argument counter
 * \param[in]           argv	        Argument vector
 * \param[in]           rpc	        RPC getter to run
 * \return 0 on success, else otherwise.
 */

int rpc_interface_get(int argc, const char **argv,
		      int (*const rpc) (ProtobufCService * service,
					const Dabba__InterfaceIdList * id_list))
{
	enum interface_option {
		/* option */
		OPT_INTERFACE_ID,
		OPT_TCP,
		OPT_LOCAL,
		OPT_HELP
	};

	const struct option interface_option[] = {
		{"id", required_argument, NULL, OPT_INTERFACE_ID},
		{"tcp", optional_argument, NULL, OPT_TCP},
		{"local", optional_argument, NULL, OPT_LOCAL},
		{"help", no_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0},
	};

	int ret, rc = 0;
	size_t a;
	const char *server_id = DABBA_RPC_DEFAULT_TCP_SERVER_NAME;
	ProtobufC_RPC_AddressType server_type = PROTOBUF_C_RPC_ADDRESS_TCP;
	ProtobufCService *service;
	Dabba__InterfaceIdList id_list = DABBA__INTERFACE_ID_LIST__INIT;
	Dabba__InterfaceId **idpp;

	if (!rpc)
		return ENOSYS;

	/* HACK: getopt*() start to parse options at argv[1] */
	argc++;
	argv--;

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
		rc = rpc(service, &id_list);
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

/**
 * \brief Parse which interface sub-command.
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, \c ENOSYS if the sub-command does not exist,
 * else on failure.
 *
 * This function parses the interface sub-command string and the rest of the
 * argument vector to the proper sub-command handler.
 */

int cmd_interface(int argc, const char **argv)
{
	static const struct cmd_struct cmd[] = {
		{"capabilities", cmd_interface_capabilities},
		{"coalesce", cmd_interface_coalesce},
		{"driver", cmd_interface_driver},
		{"offload", cmd_interface_offload},
		{"pause", cmd_interface_pause},
		{"settings", cmd_interface_settings},
		{"statistics", cmd_interface_statistics},
		{"status", cmd_interface_status},
	};

	return cmd_run_command(cmd, ARRAY_SIZE(cmd), argc, argv);
}
