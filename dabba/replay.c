/**
 * \file replay.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (C) 2013
 * \date 2013
 */


/*

=head1 NAME

dabba-replay - Manage replay threads

=head1 SYNOPSIS

dabba replay <command> [<arguments>...] [--help]

=head1 DESCRIPTION

Give the user the possibility to manage replay threads on the system
and to list information about currently running replays.

=head1 COMMANDS

=over

=item get

Fetch and print information about currently running replays.
The output is formatted in YAML.

=item start

Start a new replay.

=item stop

Stop a running replay.

=item stop-all

Stop all running replays.

=back

=head1 OPTIONS

=over

=item --interface <name>

Precise on which interface the replay must run.
Use "dabba interface get" to see the list of supported interfaces.

=item --pcap <path>

Read replayed traffic from pcap file at <path>.

=item --frame-number <number>

Configure the packet mmap area to contain <number> of frames.
This number must be a power of two. The default value is 32 frames.
The lowest frame number value is 8.

=item --id <thread-id>

Reference a replay by its unique thread id.
The replay id can be fetched using "dabba replay get".

=item --tcp[=<hostname>:<port>]

Query a running instance of dabbad using a TCP socket (default: localhost:55994)

=item --local[=<path>]

Query a running instance of dabbad using a Unix domain socket (default: /tmp/dabba)

=item --help

Prints the help message on the terminal

=back

=head1 EXAMPLES

=over

=item dabba replay get

Output information about all running replays

=item dabba replay start --interface eth0 --pcap eth0.pcap --frame-number 128

Starts a replay generating traffic on eth0 and read all frames from
the pcap file "eth0.pcap". The allocated packet mmap area can 
contain 128 frames.

=item dabba replay stop --id 123456789

Stop running replay which has the id "123456789"

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
#include <getopt.h>
#include <inttypes.h>
#include <errno.h>

#include <libdabba/macros.h>
#include <libdabba/packet-mmap.h>
#include <dabba/dabba.h>
#include <dabba/help.h>
#include <dabba/rpc.h>
#include <dabba/thread.h>

#define DEFAULT_REPLAY_FRAME_NUMBER 32

/**
 * \internal
 * \brief Print replay settings list to \c stdout
 * \param[in]           result	        Pointer to replay settings list
 * \param[in,out]       closure_data	Pointer to protobuf closure data
 */

static void replay_settings_print(const Dabba__ReplayList *
				  result, void *closure_data)
{
	const Dabba__Replay *replay;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;
	size_t a;

	assert(closure_data);

	rpc_header_print("replays");

	for (a = 0; result && a < result->n_list; a++) {
		replay = result->list[a];
		printf("    - id: %" PRIu64 "\n", (uint64_t) replay->id->id);
		printf("    ");
		__rpc_error_code_print(replay->status->code);
		printf("      packet mmap size: %" PRIu64 "\n",
		       replay->frame_nr * replay->frame_size);
		printf("      frame number: %" PRIu64 "\n", replay->frame_nr);
		printf("      pcap: %s\n", replay->pcap);
		printf("      interface: %s\n", replay->interface);
	}

	*status = 1;
}

/**
 * \brief Invoke replay start remote procedure call
 * \param[in]           service	        Pointer to protobuf service
 * \param[in]           replay          Pointer to replay settings to create
 * \return always returns zero.
 */

static int rpc_replay_start(ProtobufCService * service,
			    const Dabba__Replay * replay)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(replay);

	/* TODO Print create replay thread id ? */
	dabba__dabba_service__replay_start(service, replay,
					   rpc_error_code_print, &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

/**
 * \brief Invoke replay stop remote procedure call
 * \param[in]           service	        Pointer to protobuf service
 * \param[in]           id 	        Pointer to replay id to stop
 * \return always returns zero.
 */

static int rpc_replay_stop(ProtobufCService * service,
			   const Dabba__ThreadId * id)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(id);

	dabba__dabba_service__replay_stop(service, id,
					  rpc_error_code_print, &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

/**
 * \brief Invoke replay stop all remote procedure call
 * \param[in]           service	        Pointer to protobuf service
 * \param[in]           dummy 	        Pointer to unused dummy rpc message
 * \return always returns zero.
 */

static int rpc_replay_stop_all(ProtobufCService * service,
			       const Dabba__Dummy * dummy)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(dummy);

	dabba__dabba_service__replay_stop_all(service, dummy,
					      rpc_error_code_print, &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

/**
 * \brief Invoke replay settings get remote procedure call
 * \param[in]           service	        Pointer to protobuf service
 * \param[in]           id_list 	Pointer to replay id list
 * \return always returns zero.
 * \note An empty id list will query all replays currently running.
 */

static int rpc_replay_get(ProtobufCService * service,
			  const Dabba__ThreadIdList * id_list)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(id_list);

	dabba__dabba_service__replay_get(service, id_list,
					 replay_settings_print, &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

/**
 * \brief Parse argument vector to prepare a replay start query
 * \param[in]           argc	        Argument counter
 * \param[in]           argv	        Argument vector
 * \return 0 on success, \c EINVAL on invalid input.
 */

static int cmd_replay_start(int argc, const char **argv)
{
	enum replay_start_option {
		OPT_REPLAY_INTERFACE,
		OPT_REPLAY_PCAP,
		OPT_REPLAY_FRAME_NUMBER,
		OPT_REPLAY_FRAME_SIZE,
		OPT_REPLAY_APPEND,
		OPT_TCP,
		OPT_LOCAL,
		OPT_HELP
	};

	int ret;
	Dabba__Replay replay = DABBA__REPLAY__INIT;
	Dabba__ErrorCode err = DABBA__ERROR_CODE__INIT;
	const char *server_id = DABBA_RPC_DEFAULT_TCP_SERVER_NAME;
	ProtobufC_RPC_AddressType server_type = PROTOBUF_C_RPC_ADDRESS_TCP;
	ProtobufCService *service;

	static struct option replay_option[] = {
		{"interface", required_argument, NULL, OPT_REPLAY_INTERFACE},
		{"pcap", required_argument, NULL, OPT_REPLAY_PCAP},
		{"frame-number", required_argument, NULL,
		 OPT_REPLAY_FRAME_NUMBER},
		{"frame-size", required_argument, NULL, OPT_REPLAY_FRAME_SIZE},
		{"tcp", optional_argument, NULL, OPT_TCP},
		{"local", optional_argument, NULL, OPT_LOCAL},
		{"help", no_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0},
	};

	/* Assume conservative values for now */
	replay.has_frame_nr = replay.has_frame_size = 1;
	replay.frame_size = PACKET_MMAP_ETH_FRAME_LEN;
	replay.frame_nr = DEFAULT_REPLAY_FRAME_NUMBER;
	replay.status = &err;

	/* HACK: getopt*() start to parse options at argv[1] */
	argc++;
	argv--;

	/* parse replay options */
	while ((ret =
		getopt_long_only(argc, (char **)argv, "", replay_option,
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
		case OPT_REPLAY_INTERFACE:
			replay.interface = optarg;
			break;
		case OPT_REPLAY_PCAP:
			replay.pcap = optarg;
			break;
		case OPT_REPLAY_FRAME_NUMBER:
			replay.frame_nr = strtoull(optarg, NULL, 10);
			break;
		case OPT_REPLAY_FRAME_SIZE:
			replay.frame_size = strtoull(optarg, NULL, 10);
			break;
		case OPT_HELP:
		default:
			show_usage(replay_option);
			return -1;
		}
	}

	service = dabba_rpc_client_connect(server_id, server_type);

	/* Check error reporting */
	return service ? rpc_replay_start(service, &replay) : EINVAL;
}

/**
 * \brief Parse argument vector to prepare a replay stop query
 * \param[in]           argc	        Argument counter
 * \param[in]           argv	        Argument vector
 * \return 0 on success, \c EINVAL on invalid input.
 */

static int cmd_replay_stop(int argc, const char **argv)
{
	enum replay_start_option {
		OPT_REPLAY_ID,
		OPT_TCP,
		OPT_LOCAL,
		OPT_HELP
	};

	int ret;
	Dabba__ThreadId id = DABBA__THREAD_ID__INIT;
	const char *server_id = DABBA_RPC_DEFAULT_TCP_SERVER_NAME;
	ProtobufC_RPC_AddressType server_type = PROTOBUF_C_RPC_ADDRESS_TCP;
	ProtobufCService *service;

	static struct option replay_option[] = {
		{"id", required_argument, NULL, OPT_REPLAY_ID},
		{"tcp", optional_argument, NULL, OPT_TCP},
		{"local", optional_argument, NULL, OPT_LOCAL},
		{"help", no_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0},
	};

	/* HACK: getopt*() start to parse options at argv[1] */
	argc++;
	argv--;

	/* parse replay options */
	while ((ret =
		getopt_long_only(argc, (char **)argv, "", replay_option,
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
		case OPT_REPLAY_ID:
			id.id = strtoull(optarg, NULL, 10);
			break;
		case OPT_HELP:
		default:
			show_usage(replay_option);
			return -1;
		}
	}

	service = dabba_rpc_client_connect(server_id, server_type);

	/* Check error reporting */
	return service ? rpc_replay_stop(service, &id) : EINVAL;
}

/**
 * \brief Parse argument vector to prepare a replay reset query
 * \param[in]           argc	        Argument counter
 * \param[in]           argv	        Argument vector
 * \return 0 on success, \c EINVAL on invalid input.
 */

static int cmd_replay_stop_all(int argc, const char **argv)
{
	enum replay_start_option {
		OPT_TCP,
		OPT_LOCAL,
		OPT_HELP
	};

	int ret;
	Dabba__Dummy dummy = DABBA__DUMMY__INIT;
	const char *server_id = DABBA_RPC_DEFAULT_TCP_SERVER_NAME;
	ProtobufC_RPC_AddressType server_type = PROTOBUF_C_RPC_ADDRESS_TCP;
	ProtobufCService *service;

	static struct option replay_option[] = {
		{"tcp", optional_argument, NULL, OPT_TCP},
		{"local", optional_argument, NULL, OPT_LOCAL},
		{"help", no_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0},
	};

	/* HACK: getopt*() start to parse options at argv[1] */
	argc++;
	argv--;

	/* parse replay options */
	while ((ret =
		getopt_long_only(argc, (char **)argv, "", replay_option,
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
			show_usage(replay_option);
			return -1;
		}
	}

	service = dabba_rpc_client_connect(server_id, server_type);

	/* Check error reporting */
	return service ? rpc_replay_stop_all(service, &dummy) : EINVAL;
}

/**
 * \brief Parse argument vector to prepare a replay list get query
 * \param[in]           argc	        Argument counter
 * \param[in]           argv	        Argument vector
 * \return 0 on success, \c EINVAL on invalid input.
 */
static int cmd_replay_get(int argc, const char **argv)
{
	enum replay_option {
		/* option */
		OPT_REPLAY_ID,
		OPT_TCP,
		OPT_LOCAL,
		OPT_HELP
	};

	const struct option replay_option[] = {
		{"id", required_argument, NULL, OPT_REPLAY_ID},
		{"tcp", optional_argument, NULL, OPT_TCP},
		{"local", optional_argument, NULL, OPT_LOCAL},
		{"help", no_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0},
	};

	int ret, rc = 0;
	Dabba__ThreadIdList id_list = DABBA__THREAD_ID_LIST__INIT;
	Dabba__ThreadId **idpp;
	const char *server_id = DABBA_RPC_DEFAULT_TCP_SERVER_NAME;
	ProtobufC_RPC_AddressType server_type = PROTOBUF_C_RPC_ADDRESS_TCP;
	ProtobufCService *service;

	while ((ret =
		getopt_long_only(argc, (char **)argv, "", replay_option,
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
		case OPT_REPLAY_ID:
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
			show_usage(replay_option);
			rc = -1;
			goto out;
		}
	}

	service = dabba_rpc_client_connect(server_id, server_type);

	if (service)
		rc = rpc_replay_get(service, &id_list);
	else
		rc = EINVAL;

 out:
	free(id_list.list);

	/* Check error reporting */

	return rc;
}

/**
 * \brief Parse which replay sub-command.
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, \c ENOSYS if the sub-command does not exist,
 * else on failure.
 *
 * This function parses the replay sub-command string and the rest of the
 * argument vector to the proper sub-command handler.
 */

int cmd_replay(int argc, const char **argv)
{
	static const struct cmd_struct cmd[] = {
		{"start", cmd_replay_start},
		{"stop", cmd_replay_stop},
		{"stop-all", cmd_replay_stop_all},
		{"get", cmd_replay_get},
	};

	return cmd_run_command(cmd, ARRAY_SIZE(cmd), argc, argv);
}
