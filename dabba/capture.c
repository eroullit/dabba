/**
 * \file capture.c
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

dabba-capture - Manage capture threads

=head1 SYNOPSIS

dabba capture <command> [<arguments>...] [--help]

=head1 DESCRIPTION

Give the user the possibility to manage capture threads on the system
and to list information about currently running captures.

=head1 COMMANDS

=over

=item list

Fetch and print information about currently running captures.
The output is formatted in YAML.

=item start

Start a new capture.

=item stop

Stop a running capture.

=back

=head1 OPTIONS

=over

=item --interface <name>

Precise on which interface the capture must run.
Use "dabba interface list" to see the list of supported interfaces.
(the special interface "any" captures on all up and running interfaces).

=item --pcap <path>

Write all captured traffic in pcap file at <path>.

=item --frame-number <number>

Configure the packet mmap area to contain <number> of frames.
This number must be a power of two. The default value is 32 frames.
The lowest frame number value is 8.

=item --id <thread-id>

Reference a capture by its unique thread id.
The capture id can be fetched using "dabba capture list".

=back

=head1 EXAMPLES

=over

=item dabba capture list

Output information about all running captures

=item dabba capture start --interface eth0 --pcap eth0.pcap --frame-number 128

Starts a capture listening on eth0 and dumps all data in
the pcap file "eth0.pcap". The allocated packet mmap area can
contain 128 frames.

=item dabba capture stop --id 123456789

Stop running capture which has the id "123456789"

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
#include <assert.h>
#include <getopt.h>
#include <inttypes.h>
#include <errno.h>

#include <libdabba/macros.h>
#include <libdabba/packet_mmap.h>
#include <dabba/dabba.h>
#include <dabba/help.h>
#include <dabba/rpc.h>
#include <dabba/thread.h>
#include <dabba/capture-list.h>
#include <dabba/capture-settings.h>

#define DEFAULT_CAPTURE_FRAME_NUMBER 32

int rpc_capture_start(ProtobufCService * service,
		      const Dabba__Capture * capture)
{
	protobuf_c_boolean is_done = 0;

	/* TODO Print create capture thread id ? */
	dabba__dabba_service__capture_start(service, capture,
					    rpc_dummy_print, &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

int rpc_capture_stop(ProtobufCService * service, const Dabba__ThreadId * id)
{
	protobuf_c_boolean is_done = 0;

	dabba__dabba_service__capture_stop(service, id,
					   rpc_dummy_print, &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

int cmd_capture_start(int argc, const char **argv)
{
	enum capture_start_option {
		OPT_CAPTURE_INTERFACE,
		OPT_CAPTURE_PCAP,
		OPT_CAPTURE_FRAME_NUMBER,
		OPT_CAPTURE_FRAME_SIZE,
		OPT_TCP,
		OPT_LOCAL,
		OPT_HELP
	};

	int ret;
	Dabba__Capture capture = DABBA__CAPTURE__INIT;
	const char *server_id = DABBA_RPC_DEFAULT_LOCAL_SERVER_NAME;
	ProtobufC_RPC_AddressType server_type = PROTOBUF_C_RPC_ADDRESS_LOCAL;
	ProtobufCService *service;

	static struct option capture_option[] = {
		{"interface", required_argument, NULL, OPT_CAPTURE_INTERFACE},
		{"pcap", required_argument, NULL, OPT_CAPTURE_PCAP},
		{"frame-number", required_argument, NULL,
		 OPT_CAPTURE_FRAME_NUMBER},
		{"frame-size", required_argument, NULL, OPT_CAPTURE_FRAME_SIZE},
		{"tcp", optional_argument, NULL, OPT_TCP},
		{"local", optional_argument, NULL, OPT_LOCAL},
		{"help", no_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0},
	};

	/* Assume conservative values for now */
	capture.has_frame_nr = capture.has_frame_size = 1;
	capture.frame_size = PACKET_MMAP_ETH_FRAME_LEN;
	capture.frame_nr = DEFAULT_CAPTURE_FRAME_NUMBER;

	/* parse capture options */
	while ((ret =
		getopt_long_only(argc, (char **)argv, "", capture_option,
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
		case OPT_CAPTURE_INTERFACE:
			capture.interface = optarg;
			break;
		case OPT_CAPTURE_PCAP:
			capture.pcap = optarg;
			break;
		case OPT_CAPTURE_FRAME_NUMBER:
			capture.frame_nr = strtoull(optarg, NULL, 10);
			break;
		case OPT_CAPTURE_FRAME_SIZE:
			capture.frame_size = strtoull(optarg, NULL, 10);
			break;
		case OPT_HELP:
		default:
			show_usage(capture_option);
			return -1;
		}
	}

	service = dabba_rpc_client_connect(server_id, server_type);

	/* Check error reporting */
	return service ? rpc_capture_start(service, &capture) : EINVAL;
}

int cmd_capture_stop(int argc, const char **argv)
{
	enum capture_start_option {
		OPT_CAPTURE_ID,
		OPT_TCP,
		OPT_LOCAL,
		OPT_HELP
	};

	int ret;
	Dabba__ThreadId id = DABBA__THREAD_ID__INIT;
	const char *server_id = DABBA_RPC_DEFAULT_LOCAL_SERVER_NAME;
	ProtobufC_RPC_AddressType server_type = PROTOBUF_C_RPC_ADDRESS_LOCAL;
	ProtobufCService *service;

	static struct option capture_option[] = {
		{"id", required_argument, NULL, OPT_CAPTURE_ID},
		{"tcp", optional_argument, NULL, OPT_TCP},
		{"local", optional_argument, NULL, OPT_LOCAL},
		{"help", no_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0},
	};

	/* parse capture options */
	while ((ret =
		getopt_long_only(argc, (char **)argv, "", capture_option,
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
		case OPT_CAPTURE_ID:
			id.id = strtoull(optarg, NULL, 10);
			break;
		case OPT_HELP:
		default:
			show_usage(capture_option);
			return -1;
		}
	}

	service = dabba_rpc_client_connect(server_id, server_type);

	/* Check error reporting */
	return service ? rpc_capture_stop(service, &id) : EINVAL;
}

int cmd_capture_get(int argc, const char **argv)
{
	enum capture_option {
		/* action */
		OPT_CAPTURE_LIST,
		OPT_CAPTURE_SETTINGS,
		/* option */
		OPT_CAPTURE_ID,
		OPT_TCP,
		OPT_LOCAL,
		OPT_HELP
	};

	static const struct rpc_struct {
		const char *const cmd;
		int (*const rpc) (ProtobufCService * service,
				  const Dabba__ThreadIdList * id_list);
	} capture_commands[] = {
		{
		"settings", rpc_capture_settings_get}, {
		"list", rpc_capture_list_get}
	};

	const struct option capture_option[] = {
		{"id", required_argument, NULL, OPT_CAPTURE_ID},
		{"tcp", optional_argument, NULL, OPT_TCP},
		{"local", optional_argument, NULL, OPT_LOCAL},
		{"help", no_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0},
	};

	int ret, rc = 0;
	size_t a;
	const char *cmd = argv[0];
	Dabba__ThreadIdList id_list = DABBA__THREAD_ID_LIST__INIT;
	Dabba__ThreadId **idpp;
	const char *server_id = DABBA_RPC_DEFAULT_LOCAL_SERVER_NAME;
	ProtobufC_RPC_AddressType server_type = PROTOBUF_C_RPC_ADDRESS_LOCAL;
	ProtobufCService *service;
	int (*rpc_get) (ProtobufCService * service,
			const Dabba__ThreadIdList * id_list) = NULL;

	if (argc || argv[0]) {
		/* Parse get action to run */
		for (a = 0; a < ARRAY_SIZE(capture_commands); a++)
			if (!strcmp(capture_commands[a].cmd, cmd)) {
				rpc_get = capture_commands[a].rpc;
				break;
			}
	}

	if (!rpc_get)
		return ENOSYS;

	while ((ret =
		getopt_long_only(argc, (char **)argv, "", capture_option,
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
		case OPT_CAPTURE_ID:
			idpp =
			    realloc(id_list.list,
				    sizeof(*id_list.list) * (id_list.n_list +
							     1));

			if (!idpp)
				return ENOMEM;

			id_list.list = idpp;

			dabba__thread_id__init(id_list.list[id_list.n_list]);

			id_list.list[id_list.n_list]->id =
			    strtoull(optarg, NULL, 10);
			id_list.n_list++;

			break;
		case OPT_HELP:
		default:
			show_usage(capture_option);
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
	free(id_list.list);

	/* Check error reporting */

	return rc;
}

/**
 * \brief Parse which capture sub-command.
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, ENOSYS if the sub-command does not exist,
 * else on failure.
 *
 * This function parses the capture sub-command string and the rest of the
 * argument vector to the proper sub-command handler.
 */

int cmd_capture(int argc, const char **argv)
{
	static const struct cmd_struct cmd[] = {
		{"start", cmd_capture_start},
		{"stop", cmd_capture_stop},
		{"get", cmd_capture_get},
	};

	return cmd_run_builtin(cmd, ARRAY_SIZE(cmd), argc, argv);
}
