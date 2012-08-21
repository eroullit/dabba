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
#include <dabba/ipc.h>
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

/**
 * \brief Get the default used thread scheduling policy
 * \return \c SCHED_OTHER
 */

int sched_policy_default_get(void)
{
	return SCHED_OTHER;
}

/**
 * \brief Get the default used thread scheduling priority
 * \return scheduling priority 0. Default value for \c SCHED_OTHER
 */

int sched_prio_default_get(void)
{
	return 0;
}

/**
 * \brief Get the default used CPU affinity
 * \param[in]           mask		Output CPU set pointer
 * \return All set CPU set
 */

void sched_cpu_affinty_default_get(cpu_set_t * mask)
{
	size_t a;

	for (a = 0; a < CPU_SETSIZE; a++)
		CPU_SET(a, mask);
}

static char *nexttoken(char *q, int sep)
{
	if (q)
		q = strchr(q, sep);
	if (q)
		q++;
	return q;
}

/**
 * \brief Parse a CPU number list to a CPU set.
 * \param[in]           str	        Input CPU list
 * \param[in]           mask		Output CPU set pointer
 * \return 0 on success, -EINVAL on failure.
 */

static int str_to_cpu_affinity(char *str, cpu_set_t * mask)
{
	char *p, *q;

	assert(str);
	assert(mask);

	q = str;

	CPU_ZERO(mask);

	while (p = q, q = nexttoken(q, ','), p) {
		unsigned int a;	/* Beginning of range */
		unsigned int b;	/* End of range */
		unsigned int s;	/* Stride */
		char *c1, *c2;

		if (sscanf(p, "%u", &a) < 1)
			return -EINVAL;

		b = a;
		s = 1;

		c1 = nexttoken(p, '-');
		c2 = nexttoken(p, ',');

		if (c1 != NULL && (c2 == NULL || c1 < c2)) {
			if (sscanf(c1, "%u", &b) < 1)
				return -EINVAL;

			c1 = nexttoken(c1, ':');

			if (c1 != NULL && (c2 == NULL || c1 < c2))
				if (sscanf(c1, "%u", &s) < 1)
					return -EINVAL;
		}

		if (!(a <= b))
			return -EINVAL;

		while (a <= b) {
			CPU_SET(a, mask);
			a += s;
		}
	}

	return 0;
}

/**
 * \brief Print a CPU number list from a CPU set.
 * \param[in]           cpu	        Pointer to a CPU set
 */

static void display_thread_cpu_affinity(const cpu_set_t * const cpu)
{
	int trail_sep = 0;
	size_t i, j, run = 0;

	assert(cpu);

	for (i = 0; i < CPU_SETSIZE; i++)
		if (CPU_ISSET(i, cpu)) {
			for (j = i + 1; j < CPU_SETSIZE; j++) {
				if (CPU_ISSET(j, cpu))
					run++;
				else
					break;
			}

			/*
			 * Add a trailing comma at new entries but the first
			 * to get an cpu list like: 0,1-4,5,7
			 */

			if (trail_sep) {
				printf(",");
				trail_sep = 1;
			}

			if (!run)
				printf("%zu", i);
			else if (run == 1) {
				printf("%zu,%zu", i, i + 1);
				i++;
			} else {
				printf("%zu-%zu", i, i + run);
				i += run;
			}
		}
}

static void display_thread_list_header(void)
{
	printf("---\n");
	printf("  threads:\n");
}

static void display_thread_list(const struct dabba_thread *const ifconf_msg,
				const size_t elem_nr)
{
	size_t a;

	assert(ifconf_msg);
	assert(elem_nr <= DABBA_THREAD_MAX_SIZE);

	for (a = 0; a < elem_nr; a++) {
		printf("    - id: %" PRIu64 "\n", (uint64_t) ifconf_msg[a].id);
		printf("      type: %s\n", thread_key_get(ifconf_msg[a].type));
		printf("      scheduling policy: %s\n",
		       sched_policy_key_get(ifconf_msg[a].sched_policy));
		printf("      scheduling priority: %i\n",
		       ifconf_msg[a].sched_prio);
		printf("      cpu affinity: ");
		display_thread_cpu_affinity(&ifconf_msg[a].cpu);
		printf("\n");
	}
}

static void display_thread_capabilities_header(void)
{
	printf("---\n");
	printf("  thread capabilities:\n");
}

static void display_thread_capabilities(const struct dabba_thread_cap
					*const thread_cap, const size_t elem_nr)
{
	size_t a;
	assert(thread_cap);

	for (a = 0; a < elem_nr; a++) {
		printf("    %s:\n", sched_policy_key_get(thread_cap[a].policy));
		printf("        scheduling priority:\n");
		printf("            minimum: %i\n", thread_cap[a].prio_min);
		printf("            maximum: %i\n", thread_cap[a].prio_max);
	}
}

static struct option *thread_modify_options_get(void)
{
	static struct option thread_modify_option[] = {
		{"id", required_argument, NULL, OPT_THREAD_ID},
		{"sched-prio", required_argument, NULL, OPT_THREAD_SCHED_PRIO},
		{"sched-policy", required_argument, NULL,
		 OPT_THREAD_SCHED_POLICY},
		{"cpu-affinity", required_argument, NULL,
		 OPT_THREAD_CPU_AFFINITY},
		{NULL, 0, NULL, 0},
	};

	return thread_modify_option;
}

static int prepare_thread_modify_query(int argc, char **argv,
				       struct dabba_thread *thread_msg)
{
	int ret, rc = 0;

	assert(thread_msg);

	thread_msg->sched_prio = sched_prio_default_get();
	thread_msg->sched_policy = sched_policy_default_get();
	sched_cpu_affinty_default_get(&thread_msg->cpu);

	while ((ret =
		getopt_long_only(argc, argv, "", thread_modify_options_get(),
				 NULL)) != EOF) {
		switch (ret) {
		case OPT_THREAD_ID:
			thread_msg->id = strtoull(optarg, NULL, 10);
			break;
		case OPT_THREAD_SCHED_PRIO:
			thread_msg->sched_prio = strtol(optarg, NULL, 10);
			thread_msg->usage_flags |= USE_SCHED_PRIO;
			break;
		case OPT_THREAD_SCHED_POLICY:
			thread_msg->sched_policy =
			    sched_policy_value_get(optarg);
			thread_msg->usage_flags |= USE_SCHED_POLICY;
			break;
		case OPT_THREAD_CPU_AFFINITY:
			str_to_cpu_affinity(optarg, &thread_msg->cpu);
			thread_msg->usage_flags |= USE_CPU_MASK;
			break;
		default:
			show_usage(thread_modify_options_get());
			rc = -1;
			break;
		}
	}

	return rc;
}

/**
 * \brief Request the current list of running threads.
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 */

int cmd_thread_list(int argc, const char **argv)
{
	int rc;
	struct dabba_ipc_msg msg;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));

	msg.mtype = 1;
	msg.msg_body.type = DABBA_THREAD_LIST;

	display_thread_list_header();

	do {
		msg.msg_body.offset += msg.msg_body.elem_nr;
		msg.msg_body.elem_nr = 0;

		rc = dabba_ipc_msg(&msg);

		if (rc)
			break;

		display_thread_list(msg.msg_body.msg.thread,
				    msg.msg_body.elem_nr);
	} while (msg.msg_body.elem_nr);

	return rc;
}

/**
 * \brief Prepare a command to modify scheduling paramters of a specific thread.
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 */

int cmd_thread_modify(int argc, const char **argv)
{
	int rc;
	struct dabba_ipc_msg msg;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));

	msg.mtype = 1;
	msg.msg_body.type = DABBA_THREAD_MODIFY;

	rc = prepare_thread_modify_query(argc, (char **)argv,
					 msg.msg_body.msg.thread);

	if (rc)
		return rc;

	/* For now, just one thread request at a time */
	msg.msg_body.elem_nr = 1;

	return dabba_ipc_msg(&msg);
}

/**
 * \brief Prepare a command to list scheduling capabilities.
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 */

int cmd_thread_capabilities(int argc, const char **argv)
{
	int rc;
	struct dabba_ipc_msg msg;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));

	msg.mtype = 1;
	msg.msg_body.type = DABBA_THREAD_CAP_LIST;

	display_thread_capabilities_header();

	do {
		msg.msg_body.offset += msg.msg_body.elem_nr;
		msg.msg_body.elem_nr = 0;

		rc = dabba_ipc_msg(&msg);

		if (rc)
			break;

		display_thread_capabilities(msg.msg_body.msg.thread_cap,
					    msg.msg_body.elem_nr);
	} while (msg.msg_body.elem_nr);

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
		{"modify", cmd_thread_modify},
		{"list", cmd_thread_list},
		{"capabilities", cmd_thread_capabilities},
	};

	if (argc == 0 || cmd == NULL || !strcmp(cmd, "--help"))
		cmd = "help";

	for (i = 0; i < ARRAY_SIZE(thread_commands); i++) {
		if (!strcmp(thread_commands[i].cmd, cmd))
			return run_builtin(&thread_commands[i], argc, argv);
	}

	return ENOSYS;
}
