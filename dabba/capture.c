/**
 * \file capture.c
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

dabba-capture - Manage capture threads

=head1 SYNOPSIS

dabba capture <command> [<arguments>...] [--help]

=head1 DESCRIPTION

Give the user the possibility to manage capture threads on the system
and to list information about currently running captures.

=head1 COMMANDS

=over

=item get

Fetch and print information about currently running captures.
The output is formatted in YAML.

=item start

Start a new capture.

=item stop

Stop a running capture.

=item stop-all

Stop all running captures.

=back

=head1 OPTIONS

=over

=item --interface <name>

Precise on which interface the capture must run.
Use "dabba interface get" to see the list of supported interfaces.
(the special interface "any" captures on all up and running interfaces).

=item --pcap <path>

Write all captured traffic in pcap file at <path>.

=item --frame-number <number>

Configure the packet mmap area to contain <number> of frames.
This number must be a power of two. The default value is 32 frames.
The lowest frame number value is 8.

=item --append

Append capture to existing pcap file

=item --id <thread-id>

Reference a capture by its unique thread id.
The capture id can be fetched using "dabba capture get".

=item --tcp[=<hostname>:<port>]

Query a running instance of dabbad using a TCP socket (default: localhost:55994)

=item --local[=<path>]

Query a running instance of dabbad using a Unix domain socket (default: /tmp/dabba)

=item --help

Prints the help message on the terminal

=back

=head1 EXAMPLES

=over

=item dabba capture get

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

=item Copyright (C) 2012 Emmanuel Roullit.

=item License GPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html>.

=item This is free software: you are free to change and redistribute it.

=item There is NO WARRANTY, to the extent permitted by law.

=back

=cut

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <inttypes.h>
#include <errno.h>

#include <libdabba/macros.h>
#include <libdabba/packet_mmap.h>
#include <dabba/dabba.h>
#include <dabba/help.h>
#include <dabba/macros.h>
#include <dabba/rpc.h>
#include <dabba/sock-filter.h>
#include <dabba/thread.h>

#define DEFAULT_CAPTURE_FRAME_NUMBER 32

/**
 * \internal
 * \brief Print capture settings list to \c stdout
 * \param[in]           result	        Pointer to capture settings list
 * \param[in,out]       closure_data	Pointer to protobuf closure data
 */

static void capture_settings_print(const Dabba__CaptureList *
				   result, void *closure_data)
{
	const Dabba__Capture *capture;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;
	size_t a;

	assert(closure_data);

	rpc_header_print("captures");

	for (a = 0; result && a < result->n_list; a++) {
		capture = result->list[a];
		printf("    - id: %" PRIu64 "\n", (uint64_t) capture->id->id);
		printf("    ");
		__rpc_error_code_print(capture->status->code);
		printf("      packet mmap size: %" PRIu64 "\n",
		       capture->frame_nr * capture->frame_size);
		printf("      frame number: %" PRIu64 "\n", capture->frame_nr);
		printf("      pcap: %s\n", capture->pcap);
		printf("      interface: %s\n", capture->interface);
		printf("      socket filter: %s\n", print_tf(capture->sfp));
	}

	*status = 1;
}

/**
 * \brief Invoke capture start remote procedure call
 * \param[in]           service	        Pointer to protobuf service
 * \param[in]           capture 	Pointer to capture settings to create
 * \return always returns zero.
 */

static int rpc_capture_start(ProtobufCService * service,
			     const Dabba__Capture * capture)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(capture);

	/* TODO Print create capture thread id ? */
	dabba__dabba_service__capture_start(service, capture,
					    rpc_error_code_print, &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

/**
 * \brief Invoke capture stop remote procedure call
 * \param[in]           service	        Pointer to protobuf service
 * \param[in]           id 	        Pointer to capture id to stop
 * \return always returns zero.
 */

static int rpc_capture_stop(ProtobufCService * service,
			    const Dabba__ThreadId * id)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(id);

	dabba__dabba_service__capture_stop(service, id,
					   rpc_error_code_print, &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

/**
 * \brief Invoke capture stop all remote procedure call
 * \param[in]           service	        Pointer to protobuf service
 * \param[in]           dummy 	        Pointer to unused dummy rpc message
 * \return always returns zero.
 */

static int rpc_capture_stop_all(ProtobufCService * service,
				const Dabba__Dummy * dummy)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(dummy);

	dabba__dabba_service__capture_stop_all(service, dummy,
					       rpc_error_code_print, &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

/**
 * \brief Invoke capture settings get remote procedure call
 * \param[in]           service	        Pointer to protobuf service
 * \param[in]           id_list 	Pointer to capture id list
 * \return always returns zero.
 * \note An empty id list will query all captures currently running.
 */

static int rpc_capture_get(ProtobufCService * service,
			   const Dabba__ThreadIdList * id_list)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(id_list);

	dabba__dabba_service__capture_get(service, id_list,
					  capture_settings_print, &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

/**
 * \brief Parse argument vector to prepare a capture start query
 * \param[in]           argc	        Argument counter
 * \param[in]           argv	        Argument vector
 * \return 0 on success, \c EINVAL on invalid input.
 */

static int cmd_capture_start(int argc, const char **argv)
{
	enum capture_start_option {
		OPT_CAPTURE_INTERFACE,
		OPT_CAPTURE_PCAP,
		OPT_CAPTURE_FRAME_NUMBER,
		OPT_CAPTURE_FRAME_SIZE,
		OPT_CAPTURE_SOCK_FILTER,
		OPT_CAPTURE_APPEND,
		OPT_TCP,
		OPT_LOCAL,
		OPT_HELP
	};

	int ret, rc;
	Dabba__Capture capture = DABBA__CAPTURE__INIT;
	Dabba__SockFprog sfp = DABBA__SOCK_FPROG__INIT;
	Dabba__ErrorCode err = DABBA__ERROR_CODE__INIT;
	const char *server_id = DABBA_RPC_DEFAULT_LOCAL_SERVER_NAME;
	ProtobufC_RPC_AddressType server_type = PROTOBUF_C_RPC_ADDRESS_LOCAL;
	ProtobufCService *service;

	static struct option capture_option[] = {
		{"interface", required_argument, NULL, OPT_CAPTURE_INTERFACE},
		{"pcap", required_argument, NULL, OPT_CAPTURE_PCAP},
		{"frame-number", required_argument, NULL,
		 OPT_CAPTURE_FRAME_NUMBER},
		{"frame-size", required_argument, NULL, OPT_CAPTURE_FRAME_SIZE},
		{"sock-filter", required_argument, NULL,
		 OPT_CAPTURE_SOCK_FILTER},
		{"append", no_argument, NULL, OPT_CAPTURE_APPEND},
		{"tcp", optional_argument, NULL, OPT_TCP},
		{"local", optional_argument, NULL, OPT_LOCAL},
		{"help", no_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0},
	};

	/* Assume conservative values for now */
	capture.has_frame_nr = capture.has_frame_size = 1;
	capture.frame_size = PACKET_MMAP_ETH_FRAME_LEN;
	capture.frame_nr = DEFAULT_CAPTURE_FRAME_NUMBER;
	capture.status = &err;

	/* HACK: getopt*() start to parse options at argv[1] */
	argc++;
	argv--;

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
		case OPT_CAPTURE_APPEND:
			capture.has_append = 1;
			capture.append = 1;
			break;
		case OPT_CAPTURE_FRAME_SIZE:
			capture.frame_size = strtoull(optarg, NULL, 10);
			break;
		case OPT_CAPTURE_SOCK_FILTER:
			rc = sock_filter_parse(optarg, &sfp);

			if (rc)
				return rc;

			capture.sfp = &sfp;
			break;
		case OPT_HELP:
		default:
			show_usage(capture_option);
			return -1;
		}
	}

	service = dabba_rpc_client_connect(server_id, server_type);

	if (service)
		rc = rpc_capture_start(service, &capture);
	else
		rc = EINVAL;

	sock_filter_destroy(&sfp);

	return rc;
}

/**
 * \brief Parse argument vector to prepare a capture stop query
 * \param[in]           argc	        Argument counter
 * \param[in]           argv	        Argument vector
 * \return 0 on success, \c EINVAL on invalid input.
 */

static int cmd_capture_stop(int argc, const char **argv)
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

	/* HACK: getopt*() start to parse options at argv[1] */
	argc++;
	argv--;

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

/**
 * \brief Parse argument vector to prepare a capture reset query
 * \param[in]           argc	        Argument counter
 * \param[in]           argv	        Argument vector
 * \return 0 on success, \c EINVAL on invalid input.
 */

static int cmd_capture_stop_all(int argc, const char **argv)
{
	enum capture_start_option {
		OPT_TCP,
		OPT_LOCAL,
		OPT_HELP
	};

	int ret;
	Dabba__Dummy dummy = DABBA__DUMMY__INIT;
	const char *server_id = DABBA_RPC_DEFAULT_LOCAL_SERVER_NAME;
	ProtobufC_RPC_AddressType server_type = PROTOBUF_C_RPC_ADDRESS_LOCAL;
	ProtobufCService *service;

	static struct option capture_option[] = {
		{"tcp", optional_argument, NULL, OPT_TCP},
		{"local", optional_argument, NULL, OPT_LOCAL},
		{"help", no_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0},
	};

	/* HACK: getopt*() start to parse options at argv[1] */
	argc++;
	argv--;

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
		case OPT_HELP:
		default:
			show_usage(capture_option);
			return -1;
		}
	}

	service = dabba_rpc_client_connect(server_id, server_type);

	/* Check error reporting */
	return service ? rpc_capture_stop_all(service, &dummy) : EINVAL;
}

/**
 * \brief Parse argument vector to prepare a capture list get query
 * \param[in]           argc	        Argument counter
 * \param[in]           argv	        Argument vector
 * \return 0 on success, \c EINVAL on invalid input.
 */
static int cmd_capture_get(int argc, const char **argv)
{
	enum capture_option {
		/* option */
		OPT_CAPTURE_ID,
		OPT_TCP,
		OPT_LOCAL,
		OPT_HELP
	};

	const struct option capture_option[] = {
		{"id", required_argument, NULL, OPT_CAPTURE_ID},
		{"tcp", optional_argument, NULL, OPT_TCP},
		{"local", optional_argument, NULL, OPT_LOCAL},
		{"help", no_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0},
	};

	int ret, rc = 0;
	Dabba__ThreadIdList id_list = DABBA__THREAD_ID_LIST__INIT;
	Dabba__ThreadId **idpp;
	const char *server_id = DABBA_RPC_DEFAULT_LOCAL_SERVER_NAME;
	ProtobufC_RPC_AddressType server_type = PROTOBUF_C_RPC_ADDRESS_LOCAL;
	ProtobufCService *service;

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
		rc = rpc_capture_get(service, &id_list);
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
 * \return 0 on success, \c ENOSYS if the sub-command does not exist,
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
		{"stop-all", cmd_capture_stop_all},
		{"get", cmd_capture_get},
	};

	return cmd_run_command(cmd, ARRAY_SIZE(cmd), argc, argv);
}
