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
#include <dabbad/dabbad.h>
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
	enum capture_option {
		/* action */
		OPT_THREAD_LIST,
		OPT_THREAD_CAPABILITIES,
		OPT_THREAD_SETTINGS,
		/* option */
		OPT_THREAD_ID,
		OPT_TCP_SERVER_ID,
		OPT_HELP
	};

	int (*const rpc_thread_get[]) (ProtobufCService * service,
				       const Dabba__ThreadIdList * id_list) = {
	[OPT_THREAD_LIST] = rpc_thread_list_get,
		    [OPT_THREAD_CAPABILITIES] =
		    rpc_thread_capabilities_get,[OPT_THREAD_SETTINGS] =
		    rpc_thread_settings_get};

	const struct option thread_option[] = {
		{"id", required_argument, NULL, OPT_THREAD_ID},
		{"list", no_argument, NULL, OPT_THREAD_LIST},
		{"capabilities", no_argument, NULL, OPT_THREAD_CAPABILITIES},
		{"settings", no_argument, NULL, OPT_THREAD_SETTINGS},
		{"tcp-server", required_argument, NULL, OPT_TCP_SERVER_ID},
		{"help", no_argument, NULL, OPT_HELP},
		{NULL, 0, NULL, 0},
	};

	int ret, rc = 0, action = 0;
	size_t a;
	Dabba__ThreadIdList id_list = DABBA__THREAD_ID_LIST__INIT;
	Dabba__ThreadId **idpp;
	const char *server_name = DABBA_RPC_DEFAULT_SERVER_NAME;
	ProtobufC_RPC_AddressType server_type = PROTOBUF_C_RPC_ADDRESS_TCP;
	ProtobufCService *service;

	/* parse options and actions to run */
	while ((ret =
		getopt_long_only(argc, (char **)argv, "", thread_option,
				 NULL)) != EOF) {
		switch (ret) {
		case OPT_TCP_SERVER_ID:
			server_name = optarg;
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
		case OPT_THREAD_LIST:
		case OPT_THREAD_CAPABILITIES:
		case OPT_THREAD_SETTINGS:
			action |= (1 << ret);
			break;
		case OPT_HELP:
		default:
			show_usage(thread_option);
			rc = -1;
			goto out;
		}
	}

	service = dabba_rpc_client_connect(server_name, server_type);

	/* list threads as default action */
	if (!action)
		action = (1 << OPT_THREAD_LIST);

	/* run requested actions */
	for (a = 0; a < ARRAY_SIZE(rpc_thread_get); a++)
		if (action & (1 << a))
			rc = rpc_thread_get[a] (service, &id_list);
 out:
	free(id_list.list);

	/* Check error reporting */

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
	const char *cmd = argv[0];
	size_t i;
	static struct cmd_struct thread_commands[] = {
		{"get", cmd_thread_get}
	};

	if (argc == 0 || cmd == NULL || !strcmp(cmd, "--help"))
		cmd = "help";

	for (i = 0; i < ARRAY_SIZE(thread_commands); i++)
		if (!strcmp(thread_commands[i].cmd, cmd))
			return run_builtin(&thread_commands[i], argc, argv);

	return ENOSYS;
}
