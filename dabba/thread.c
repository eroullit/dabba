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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <assert.h>
#include <sched.h>
#include <sys/resource.h>

#include <libdabba/macros.h>
#include <dabbad/dabbad.h>
#include <dabbad/thread.h>
#include <dabba/dabba.h>
#include <dabba/ipc.h>

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

int sched_policy_value_get(const char *const policy_name)
{
	size_t a;

	assert(policy_name);

	for (a = 0; a < ARRAY_SIZE(sched_policy_mapping) - 1; a++)
		if (!strcmp(policy_name, sched_policy_mapping[a].key))
			break;

	return sched_policy_mapping[a].value;
}

const char *sched_policy_key_get(const int policy_value)
{
	size_t a;

	for (a = 0; a < ARRAY_SIZE(sched_policy_mapping) - 1; a++)
		if (policy_value == sched_policy_mapping[a].value)
			break;

	return sched_policy_mapping[a].key;
}

const char *thread_key_get(const int type)
{
	size_t a, max = ARRAY_SIZE(thread_name_mapping);

	for (a = 0; a < max; a++)
		if (type == thread_name_mapping[a].value)
			break;

	return a < max ? thread_name_mapping[a].key : "unknown";
}

int sched_policy_default_get(void)
{
	return SCHED_OTHER;
}

int sched_prio_default_get(void)
{
	return 0;
}

void sched_cpu_affinty_default_get(cpu_set_t *mask)
{
    size_t a;
    
    for(a = 0; a < CPU_SETSIZE; a++)
        CPU_SET(a, mask);
}

static char *nexttoken(char *q,  int sep)
{
	if (q)
		q = strchr(q, sep);
	if (q)
		q++;
	return q;
}

int str_to_cpu_affinity(char *str, cpu_set_t *mask)
{
	char *p, *q;

        assert(str);
        assert(mask);
        
	q = str;

	CPU_ZERO(mask);

	while (p = q, q = nexttoken(q, ','), p) {
		unsigned int a;	 /* Beginning of range */
		unsigned int b;	 /* End of range */
		unsigned int s;	 /* Stride */
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

static void display_thread_cpu_affinity(const cpu_set_t * const cpu)
{
      
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
                        
			if (!run)
				printf("%zu,", i);
			else if (run == 1) {
				printf("%zu,%zu,", i, i + 1);
				i++;
			} else {
				printf("%zu-%zu,", i, i + run);
				i += run;
			}
		}
}

static void display_thread_list_header(void)
{
	printf("---\n");
	printf("  threads:\n");
}

static void display_thread_list(struct dabba_ipc_msg *const msg)
{
	struct dabba_thread *thread_msg;
	size_t a;

	assert(msg);
	assert(msg->msg_body.elem_nr <= ARRAY_SIZE(msg->msg_body.msg.thread));

	for (a = 0; a < msg->msg_body.elem_nr; a++) {
		thread_msg = &msg->msg_body.msg.thread[a];
		printf("    - id: %" PRIu64 "\n", (uint64_t) thread_msg->id);
		printf("      type: %s\n", thread_key_get(thread_msg->type));
		printf("      scheduling policy: %s\n",
		       sched_policy_key_get(thread_msg->sched_policy));
		printf("      scheduling priority: %i\n",
		       thread_msg->sched_prio);
                printf("      cpu affinity: ");
                display_thread_cpu_affinity(&thread_msg->cpu);
                printf("\n");
	}
}

static void prepare_thread_list_query(struct dabba_ipc_msg *msg)
{
	assert(msg);
	msg->mtype = 1;
	msg->msg_body.type = DABBA_THREAD_LIST;
}

static void display_thread_list_msg(struct dabba_ipc_msg *const msg)
{
	assert(msg);

	switch (msg->msg_body.type) {
	case DABBA_THREAD_LIST:
		display_thread_list(msg);
		break;
	default:
		break;
	}
}

/**
 * \brief Request the current supported interface list
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 *
 * This function prepares an IPC message to query the supported network
 * interfaces present on the system. Once the message is sent, it waits for the
 * dabba daemon to reply.
 */

int cmd_thread_list(int argc, const char **argv)
{
	int rc;
	struct dabba_ipc_msg msg;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));
	prepare_thread_list_query(&msg);
	display_thread_list_header();

	do {
		msg.msg_body.offset += msg.msg_body.elem_nr;
		msg.msg_body.elem_nr = 0;

		rc = dabba_ipc_msg(&msg);

		if (rc)
			break;

		display_thread_list_msg(&msg);
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
		{"list", cmd_thread_list}
	};

	if (argc == 0 || cmd == NULL || !strcmp(cmd, "--help"))
		cmd = "help";

	for (i = 0; i < ARRAY_SIZE(thread_commands); i++) {
		struct cmd_struct *p = thread_commands + i;
		if (strcmp(p->cmd, cmd))
			continue;
		return (run_builtin(p, argc, argv));
	}

	return ENOSYS;
}
