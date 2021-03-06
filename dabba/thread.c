/**
 * \file thread.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (C) 2013
 * \date 2013
 */


/*

=head1 NAME

dabba-thread - Manage thread specific parameters

=head1 SYNOPSIS

dabba thread <command> [<arguments>...] [--help]

=head1 DESCRIPTION

Give the user the possibility to manage threads scheduling policy,
scheduling priority and CPU affinity running on the system.
It also lists information about currently running threads.

=head1 COMMANDS

=over

=item get

Fetch and print information about currently running threads.
The output is formatted in YAML.

=item modify

Modify scheduling parameter of a running thread.

=item capabilities

Fetch currently supported minimum and maximum scheduling priorities
for each scheduling policy.

=back

=head1 OPTIONS

=over

=item --sched-prio <priority>

Configure which scheduling priority level to set on specific thread.
This value is an signed integer. Possible scheduling priority range
can be printed with using "dabba thread capabilities".

=item --sched-policy <policy>

Configure which scheduling policy to set on specific thread.
The supported scheduling policies are:

=over

=item - fifo

First In First Out (also known as First Come, First Served)

=item - rr

Round Robin

=item - other

Other (default)

=back

=item --cpu-affinity <cpu-list>

Configure a numerical list of processors on which a specific thread 
is allowed to be scheduled on.
CPU numbers can be separated by commas and may include ranges.
For instance: 0,5,7,9-11.

=item --id <thread-id>

Reference a thread by its unique thread id.
The thread id can be fetched using "dabba thread list".

=item --tcp[=<hostname>:<port>]

Query a running instance of dabbad using a TCP socket (default: localhost:55994)

=item --local[=<path>]

Query a running instance of dabbad using a Unix domain socket (default: /tmp/dabba)

=item --help

Prints the help message on the terminal

=back

=head1 EXAMPLES

=over

=item dabba thread list

Output information about all running threads

=item dabba thread capabilities

Output information about currently support scheduling capabilities.

=item dabba thread modify --cpu-list 0-2,4 --sched-policy rr --sched-prio 10 --id 123456789

Change scheduling parameters of thread id "123456789" to:

=over

=over

=item - Round Robin scheduling policy

=item - Scheduling priority 10

=item - Allowed to be scheduled on CPU 0,1,2,4

=back

=back

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

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif				/* _GNU_SOURCE */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <inttypes.h>
#include <assert.h>
#include <getopt.h>

#include <libdabba/macros.h>
#include <dabbad/thread.h>
#include <dabba/thread-capabilities.h>
#include <dabba/rpc.h>
#include <dabba/cli.h>
#include <dabba/help.h>
#include <dabba/dabba.h>

/**
 * \internal
 * \brief Protobuf closure to print thread settings list in YAML
 * \param[in]           result	        Pointer to thread settings list
 * \param[in]           closure_data	Pointer to protobuf closure data
 */

static void thread_print(const Dabba__ThreadList * result, void *closure_data)
{
	const Dabba__Thread *thread;
	size_t a;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;

	assert(closure_data);

	rpc_header_print("threads");

	for (a = 0; result && a < result->n_list; a++) {
		thread = result->list[a];
		printf("    - id: %" PRIu64 "\n", thread->id->id);
		printf("    ");
		__rpc_error_code_print(thread->status->code);
		printf("      type: %s\n", thread_type2str(thread->type));
		printf("      scheduling policy: %s\n",
		       sched_policy2str(thread->sched_policy));
		printf("      scheduling priority: %i\n",
		       thread->sched_priority);
		printf("      cpu affinity: %s\n", thread->cpu_set);
		/* TODO map priority/policy protobuf enums to string */
	}

	*status = 1;
}

/**
 * \internal
 * \brief Invoke thread settings get RPC
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           id_list         Pointer to thread id to fetch
 * \return Always returns 0.
 * \note An empty id list will query the status of all available thread.
 */

static int rpc_thread_get(ProtobufCService * service,
			  const Dabba__ThreadIdList * id_list)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(id_list);

	dabba__dabba_service__thread_get(service, id_list,
					 thread_print, &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

/**
 * \internal
 * \brief Invoke thread settings modify RPC
 * \param[in]           service	        Pointer to protobuf service structure
 * \param[in]           status          Pointer to thread new status settings
 * \return Always returns 0.
 */

static int rpc_thread_modify(ProtobufCService * service,
			     const Dabba__Thread * thread)
{
	protobuf_c_boolean is_done = 0;

	assert(service);
	assert(thread);

	dabba__dabba_service__thread_modify(service, thread,
					    rpc_error_code_print, &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

/**
 * \brief Parse argument vector to prepare a thread list get query
 * \param[in]           argc	        Argument counter
 * \param[in]           argv	        Argument vector
 * \return 0 on success, \c EINVAL on invalid input.
 */

static int cmd_thread_get(int argc, const char **argv)
{
	enum thread_option {
		/* option */
		OPT_THREAD_ID,
		OPT_TCP,
		OPT_LOCAL,
		OPT_HELP
	};

	const struct option thread_option[] = {
		{"id", required_argument, NULL, OPT_THREAD_ID},
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

	/* HACK: getopt*() start to parse options at argv[1] */
	argc++;
	argv--;

	/* parse options and actions to run */
	while ((ret =
		getopt_long_only(argc, (char **)argv, "", thread_option,
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
		case OPT_THREAD_ID:
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
			show_usage(thread_option);
			rc = -1;
			goto out;
		}
	}

	service = dabba_rpc_client_connect(server_id, server_type);

	if (service)
		rc = rpc_thread_get(service, &id_list);
	else
		rc = EINVAL;

 out:
	free(id_list.list);

	/* Check error reporting */

	return rc;
}

/**
 * \brief Parse argument vector to prepare a thread modify query
 * \param[in]           argc	        Argument counter
 * \param[in]           argv	        Argument vector
 * \return 0 on success, \c EINVAL on invalid input.
 */

static int cmd_thread_modify(int argc, const char **argv)
{
	enum thread_option {
		/* action */
		OPT_THREAD_SETTINGS,
		/* option */
		OPT_THREAD_SCHED_PRIORITY,
		OPT_THREAD_SCHED_POLICY,
		OPT_THREAD_CPU_AFFINITY,
		OPT_THREAD_ID,
		OPT_TCP,
		OPT_LOCAL,
		OPT_HELP
	};

	const struct option thread_option[] = {
		{"id", required_argument, NULL, OPT_THREAD_ID},
		{"settings", no_argument, NULL, OPT_THREAD_SETTINGS},
		{"sched-prio", required_argument, NULL,
		 OPT_THREAD_SCHED_PRIORITY},
		{"sched-policy", required_argument, NULL,
		 OPT_THREAD_SCHED_POLICY},
		{"cpu-affinity", required_argument, NULL,
		 OPT_THREAD_CPU_AFFINITY},
		{"tcp", optional_argument, NULL, OPT_TCP},
		{"local", optional_argument, NULL, OPT_LOCAL},
		{"help", no_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0},
	};

	int ret, rc = 0;
	Dabba__Thread thread = DABBA__THREAD__INIT;
	Dabba__ErrorCode err = DABBA__ERROR_CODE__INIT;
	const char *server_id = DABBA_RPC_DEFAULT_TCP_SERVER_NAME;
	ProtobufC_RPC_AddressType server_type = PROTOBUF_C_RPC_ADDRESS_TCP;
	ProtobufCService *service;

	/* HACK: getopt*() start to parse options at argv[1] */
	argc++;
	argv--;

	/* parse options and actions to run */
	while ((ret =
		getopt_long_only(argc, (char **)argv, "", thread_option,
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
		case OPT_THREAD_ID:
			thread.id = malloc(sizeof(*thread.id));
			if (!thread.id)
				return ENOMEM;

			dabba__thread_id__init(thread.id);
			thread.id->id = strtoull(optarg, NULL, 10);
			break;
		case OPT_THREAD_SCHED_POLICY:
			thread.has_sched_policy = 1;
			thread.sched_policy = str2sched_policy(optarg);
			break;
		case OPT_THREAD_SCHED_PRIORITY:
			thread.has_sched_priority = 1;
			thread.sched_priority = strtol(optarg, NULL, 10);
			break;
		case OPT_THREAD_CPU_AFFINITY:
			thread.cpu_set = optarg;
			break;
		case OPT_HELP:
		default:
			show_usage(thread_option);
			rc = -1;
			goto out;
		}
	}

	thread.status = &err;

	service = dabba_rpc_client_connect(server_id, server_type);

	if (!service)
		return EINVAL;

	rpc_thread_modify(service, &thread);
 out:
	free(thread.id);
	return rc;

}

/**
 * \brief Parse which thread sub-command.
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, \c ENOSYS if the sub-command does not exist,
 * else on failure.
 *
 * This function parses the thread sub-command string and the rest of the
 * argument vector to the proper sub-command handler.
 */

int cmd_thread(int argc, const char **argv)
{
	static const struct cmd_struct cmd[] = {
		{"capabilities", cmd_thread_capabilities},
		{"get", cmd_thread_get},
		{"modify", cmd_thread_modify}
	};

	return cmd_run_command(cmd, ARRAY_SIZE(cmd), argc, argv);
}
