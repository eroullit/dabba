/**
 * \file thread.c
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

dabba-thread - Manage thread specific parameters

=head1 SYNOPSIS

dabba thread <command> [<arguments>...] [--help]

=head1 DESCRIPTION

Give the user the possibility to manage threads scheduling policy,
scheduling priority and CPU affinity running on the system.
It also lists information about currently running threads.

=head1 COMMANDS

=over

=item list

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

=item Copyright © 2012 Emmanuel Roullit.

=item License GPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/gpl.html>.

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
#include <sched.h>
#include <getopt.h>
#include <sys/resource.h>

#include <libdabba/macros.h>
#include <dabbad/thread.h>
#include <dabba/dabba.h>
#include <dabba/thread-list.h>
#include <dabba/thread-capabilities.h>
#include <dabba/thread-settings.h>
#include <dabba/rpc.h>
#include <dabba/help.h>

static const char sched_policy[][8] = {
	[SCHED_OTHER] = "other",
	[SCHED_FIFO] = "fifo",
	[SCHED_RR] = "rr"
};

/**
 * \brief Get the policy value out of the policy name.
 * \param[in]           policy_name	Policy name string
 * \return Related policy value
 */

int str2sched_policy(const char *const policy_name)
{
	const size_t max = ARRAY_SIZE(sched_policy);
	size_t a;

	assert(policy_name);

	for (a = 0; a < max; a++)
		if (sched_policy[a] && !strcmp(policy_name, sched_policy[a]))
			break;

	return a < max ? a : SCHED_OTHER;
}

/**
 * \brief Get the policy name out of the policy value.
 * \param[in]           policy_name	Policy value
 * \return Related policy name
 */

const char *sched_policy2str(const int policy)
{
	const int max = ARRAY_SIZE(sched_policy);

	return policy >= 0 && policy < max ? sched_policy[policy] : "unknown";
}

/**
 * \brief Get the thread name out of the thread type value
 * \param[in]           type	Thread type value
 * \return Thread name string
 */

const char *thread_type2str(const int type)
{
	static const char thread_type[][8] = {
		[CAPTURE_THREAD] = "capture"
	};

	int max = ARRAY_SIZE(thread_type);

	return type >= 0 && type < max
	    && thread_type[type] ? thread_type[type] : "unknown";
}

int cmd_thread_get(int argc, const char **argv)
{
	enum thread_option {
		/* option */
		OPT_THREAD_ID,
		OPT_TCP,
		OPT_LOCAL,
		OPT_HELP
	};

	static const struct rpc_struct {
		const char *const cmd;
		int (*const rpc) (ProtobufCService * service,
				  const Dabba__ThreadIdList * id_list);
	} thread_commands[] = {
		{
		"settings", rpc_thread_settings_get}, {
		"list", rpc_thread_list_get}, {
		"capabilities", rpc_thread_capabilities_get}
	};

	const struct option thread_option[] = {
		{"id", required_argument, NULL, OPT_THREAD_ID},
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
		for (a = 0; a < ARRAY_SIZE(thread_commands); a++)
			if (!strcmp(thread_commands[a].cmd, cmd)) {
				rpc_get = thread_commands[a].rpc;
				break;
			}
	} else
		rpc_get = rpc_thread_settings_get;

	if (!rpc_get)
		return ENOSYS;

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
		rc = rpc_get(service, &id_list);
	else
		rc = EINVAL;

 out:
	free(id_list.list);

	/* Check error reporting */

	return rc;
}

int cmd_thread_modify(int argc, const char **argv)
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
	const char *server_id = DABBA_RPC_DEFAULT_LOCAL_SERVER_NAME;
	ProtobufC_RPC_AddressType server_type = PROTOBUF_C_RPC_ADDRESS_LOCAL;
	ProtobufCService *service;

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

	service = dabba_rpc_client_connect(server_id, server_type);

	if (!service)
		return EINVAL;

	rpc_thread_settings_modify(service, &thread);
 out:
	free(thread.id);
	return rc;

}

/**
 * \brief Parse which thread sub-command.
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, ENOSYS if the sub-command does not exist,
 * else on failure.
 *
 * This function parses the thread sub-command string and the rest of the
 * argument vector to the proper sub-command handler.
 */

int cmd_thread(int argc, const char **argv)
{
	static const struct cmd_struct cmd[] = {
		{"get", cmd_thread_get},
		{"modify", cmd_thread_modify}
	};

	return cmd_run_builtin(cmd, ARRAY_SIZE(cmd), argc, argv);
}
