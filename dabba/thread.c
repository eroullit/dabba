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
#include <dabba/rpc.h>
#include <dabba/help.h>

enum thread_modify_option {
	OPT_THREAD_ID,
	OPT_THREAD_SCHED_PRIO,
	OPT_THREAD_SCHED_POLICY,
	OPT_THREAD_CPU_AFFINITY
};

static struct sched_policy_name_mapping {
	const char key[8];
	int value;
} sched_policy_mapping[] = {
	{
	.key = "rr",.value = SCHED_RR}, {
	.key = "fifo",.value = SCHED_FIFO}, {
	.key = "other",.value = SCHED_OTHER}
};

static struct thread_name_mapping {
	const char key[8];
	int value;
} thread_name_mapping[] = {
	{
	.key = "capture",.value = CAPTURE_THREAD}
};

/**
 * \brief Get the policy value out of the policy name.
 * \param[in]           policy_name	Policy name string
 * \return Related policy value
 */

int sched_policy_value_get(const char *const policy_name)
{
	size_t a;

	assert(policy_name);

	for (a = 0; a < ARRAY_SIZE(sched_policy_mapping) - 1; a++)
		if (!strcmp(policy_name, sched_policy_mapping[a].key))
			break;

	return sched_policy_mapping[a].value;
}

/**
 * \brief Get the policy name out of the policy value.
 * \param[in]           policy_name	Policy value
 * \return Related policy name
 */

const char *sched_policy_key_get(const int policy_value)
{
	size_t a;

	for (a = 0; a < ARRAY_SIZE(sched_policy_mapping) - 1; a++)
		if (policy_value == sched_policy_mapping[a].value)
			break;

	return sched_policy_mapping[a].key;
}

/**
 * \brief Get the thread name out of the thread type value
 * \param[in]           type	Thread type value
 * \return Thread name string
 */

const char *thread_key_get(const int type)
{
	size_t a, max = ARRAY_SIZE(thread_name_mapping);

	for (a = 0; a < max; a++)
		if (type == thread_name_mapping[a].value)
			break;

	return a < max ? thread_name_mapping[a].key : "unknown";
}

static void thread_header_print(void)
{
	printf("---\n");
	printf("  threads:\n");
}

static void thread_id_print(const Dabba__ThreadIdList * result,
			    void *closure_data)
{
	size_t a;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;

	thread_header_print();

	for (a = 0; result && a < result->n_list; a++)
		printf("    - %" PRIu64 "\n", result->list[a]->id);

	*status = 1;
}

static void thread_print(const Dabba__ThreadList * result, void *closure_data)
{
	size_t a;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;

	thread_header_print();

	for (a = 0; result && a < result->n_list; a++) {
		printf("    - id: %" PRIu64 "\n", result->list[a]->id->id);
		printf("      type: %s\n",
		       thread_key_get(result->list[a]->type));
		printf("      scheduling policy: %s\n",
		       sched_policy_key_get(result->list[a]->sched_policy));
		printf("      scheduling priority: %i\n",
		       result->list[a]->sched_priority);
		printf("      cpu affinity: %s\n", result->list[a]->cpu_set);
		/* TODO map priority/policy protobuf enums to string */
	}

	*status = 1;
}

static void display_thread_capabilities_header(void)
{
	printf("---\n");
	printf("  thread capabilities:\n");
}

static void thread_capabilities_list_print(const Dabba__ThreadCapabilitiesList *
					   result, void *closure_data)
{
	const Dabba__ThreadCapabilities *cap;
	protobuf_c_boolean *status = (protobuf_c_boolean *) closure_data;
	size_t a;

	display_thread_capabilities_header();

	for (a = 0; a < result->n_list; a++) {
		cap = result->list[a];
		printf("    %s:\n", sched_policy_key_get(cap->policy));
		printf("        scheduling priority:\n");
		printf("            minimum: %i\n", cap->prio_min);
		printf("            maximum: %i\n", cap->prio_max);
	}

	*status = 1;
}

/**
 * \brief Request the current list of running threads.
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 */

int cmd_thread_list(int argc, const char **argv)
{
	ProtobufCService *service;
	protobuf_c_boolean is_done = 0;
	Dabba__Dummy dummy = DABBA__DUMMY__INIT;

	assert(argc >= 0);
	assert(argv);

	/* TODO Make server name configurable */
	service = dabba_rpc_client_connect(NULL);

	dabba__dabba_service__thread_id_get(service, &dummy, thread_id_print,
					    &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

int cmd_thread_settings(int argc, const char **argv)
{
	ProtobufCService *service;
	protobuf_c_boolean is_done = 0;
	Dabba__ThreadIdList id_list = DABBA__THREAD_ID_LIST__INIT;

	assert(argc >= 0);
	assert(argv);

	/* TODO Allow thread id argument to filter output */
	/* TODO Make server name configurable */
	service = dabba_rpc_client_connect(NULL);

	dabba__dabba_service__thread_get(service, &id_list,
					 thread_print, &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
}

/**
 * \brief Prepare a command to list scheduling capabilities.
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 */

int cmd_thread_capabilities(int argc, const char **argv)
{
	ProtobufCService *service;
	protobuf_c_boolean is_done = 0;
	Dabba__Dummy id_list = DABBA__DUMMY__INIT;

	assert(argc >= 0);
	assert(argv);

	/* TODO Make server name configurable */
	service = dabba_rpc_client_connect(NULL);

	dabba__dabba_service__thread_capabilities_get(service, &id_list,
						      thread_capabilities_list_print,
						      &is_done);

	dabba_rpc_call_is_done(&is_done);

	return 0;
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
		{"modify", NULL},
		{"list", cmd_thread_list},
		{"settings", cmd_thread_settings},
		{"capabilities", cmd_thread_capabilities},
	};

	if (argc == 0 || cmd == NULL || !strcmp(cmd, "--help"))
		cmd = "help";

	for (i = 0; i < ARRAY_SIZE(thread_commands); i++)
		if (!strcmp(thread_commands[i].cmd, cmd))
			return run_builtin(&thread_commands[i], argc, argv);

	return ENOSYS;
}
