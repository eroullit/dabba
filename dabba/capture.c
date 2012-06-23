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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <inttypes.h>
#include <errno.h>

#include <dabbacore/macros.h>
#include <dabbacore/strlcpy.h>
#include <dabbacore/packet_mmap.h>
#include <dabba/dabba.h>
#include <dabba/help.h>
#include <dabba/ipc.h>
#include <dabbad/dabbad.h>

enum capture_start_option {
	OPT_CAPTURE_INTERFACE,
	OPT_CAPTURE_PCAP,
	OPT_CAPTURE_FRAME_NUMBER
};

enum capture_stop_option {
	OPT_CAPTURE_ID
};

static struct option *capture_start_options_get(void)
{
	static struct option capture_start_option[] = {
		{"interface", required_argument, NULL, OPT_CAPTURE_INTERFACE},
		{"pcap", required_argument, NULL, OPT_CAPTURE_PCAP},
		{"frame-number", required_argument, NULL,
		 OPT_CAPTURE_FRAME_NUMBER},
		{NULL, 0, NULL, 0},
	};

	return capture_start_option;
}

static int prepare_capture_start_query(int argc, char **argv,
				       struct dabba_ipc_msg *msg)
{
	struct dabba_capture *capture_start_msg = msg->msg_body.msg.capture;
	int ret = 0;
	int rc = 0;

	assert(msg);
	msg->mtype = 1;
	msg->msg_body.type = DABBA_CAPTURE_START;

	while ((ret =
		getopt_long_only(argc, argv, "", capture_start_options_get(),
				 NULL)) != EOF) {
		switch (ret) {
		case OPT_CAPTURE_INTERFACE:
			if (strlen(optarg) >=
			    sizeof(capture_start_msg->dev_name))
				rc = EINVAL;

			strlcpy(capture_start_msg->dev_name, optarg,
				sizeof(capture_start_msg->dev_name));
			break;

		case OPT_CAPTURE_PCAP:
			if (strlen(optarg) >=
			    sizeof(capture_start_msg->pcap_name))
				rc = EINVAL;

			strlcpy(capture_start_msg->pcap_name, optarg,
				sizeof(capture_start_msg->pcap_name));
			break;
		case OPT_CAPTURE_FRAME_NUMBER:
			capture_start_msg->frame_nr =
			    strtoull(optarg, NULL, 10);
			break;
		default:
			show_usage(capture_start_options_get());
			rc = -1;
			break;
		}
	}

	/* Assume conservative values for now */
	capture_start_msg->frame_size = PACKET_MMAP_ETH_FRAME_LEN;

	return rc;
}

static void prepare_capture_list_query(struct dabba_ipc_msg *msg)
{
	assert(msg);
	msg->mtype = 1;
	msg->msg_body.type = DABBA_CAPTURE_LIST;
}

static void display_capture_list_msg_header(void)
{
	printf("---\n");
	printf("  captures:\n");
}

static void display_capture_list(const struct dabba_ipc_msg const *msg)
{
	size_t a;

	assert(msg);
	assert(msg->msg_body.elem_nr <= ARRAY_SIZE(msg->msg_body.msg.capture));

	for (a = 0; a < msg->msg_body.elem_nr; a++) {
		printf("    - id: %" PRIu64 "\n",
		       (uint64_t) msg->msg_body.msg.capture[a].thread_id);
		printf("      packet mmap size: %" PRIu64 "\n",
		       msg->msg_body.msg.capture[a].frame_nr *
		       msg->msg_body.msg.capture[a].frame_size);
		printf("      frame number: %" PRIu64 "\n",
		       msg->msg_body.msg.capture[a].frame_nr);
		printf("      pcap: %s\n",
		       msg->msg_body.msg.capture[a].pcap_name);
		printf("      interface: %s\n",
		       msg->msg_body.msg.capture[a].dev_name);
	}
}

/**
 * \brief Prepare a command to start a capture.
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 */

int cmd_capture_start(int argc, const char **argv)
{
	struct dabba_ipc_msg msg;
	int rc;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));

	rc = prepare_capture_start_query(argc, (char **)argv, &msg);

	if (rc)
		return rc;

	/* For now, just one capture request at a time */
	msg.msg_body.elem_nr = 1;

	return dabba_ipc_msg(&msg);
}

/**
 * \brief Prepare a command to list current captures.
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 */

int cmd_capture_list(int argc, const char **argv)
{
	int rc;
	struct dabba_ipc_msg msg;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));
	prepare_capture_list_query(&msg);
	display_capture_list_msg_header();

	do {
		msg.msg_body.offset += msg.msg_body.elem_nr;
		msg.msg_body.elem_nr = 0;

		rc = dabba_ipc_msg(&msg);

		if (rc)
			break;

		display_capture_list(&msg);
	} while (msg.msg_body.elem_nr);

	return rc;
}

static struct option *capture_stop_options_get(void)
{
	static struct option capture_stop_option[] = {
		{"id", required_argument, NULL, OPT_CAPTURE_ID},
		{NULL, 0, NULL, 0},
	};

	return capture_stop_option;
}

static int prepare_capture_stop_query(int argc, char **argv,
				      struct dabba_ipc_msg *msg)
{
	struct dabba_capture *capture_stop_msg = msg->msg_body.msg.capture;
	int ret, rc = 0;

	assert(msg);

	msg->mtype = 1;
	msg->msg_body.type = DABBA_CAPTURE_STOP;

	while ((ret =
		getopt_long_only(argc, argv, "", capture_stop_options_get(),
				 NULL)) != EOF) {
		switch (ret) {
		case OPT_CAPTURE_ID:
			capture_stop_msg->thread_id =
			    strtoull(optarg, NULL, 10);
			break;
		default:
			show_usage(capture_stop_options_get());
			rc = -1;
			break;
		}
	}

	return rc;
}

/**
 * \brief Prepare a command to stop a active capture.
 * \param[in]           argc	        Argument counter
 * \param[in]           argv		Argument vector
 * \return 0 on success, else on failure.
 */

int cmd_capture_stop(int argc, const char **argv)
{
	struct dabba_ipc_msg msg;
	int rc;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));

	rc = prepare_capture_stop_query(argc, (char **)argv, &msg);

	if (rc)
		return rc;

	/* For now, just one capture request at a time */
	msg.msg_body.elem_nr = 1;

	return dabba_ipc_msg(&msg);
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
	const char *cmd = argv[0];
	size_t i;
	static struct cmd_struct capture_commands[] = {
		{"list", cmd_capture_list},
		{"start", cmd_capture_start},
		{"stop", cmd_capture_stop},
	};

	if (argc == 0 || cmd == NULL || !strcmp(cmd, "--help"))
		cmd = "help";

	for (i = 0; i < ARRAY_SIZE(capture_commands); i++) {
		struct cmd_struct *p = capture_commands + i;
		if (strcmp(p->cmd, cmd))
			continue;
		return (run_builtin(p, argc, argv));
	}

	return ENOSYS;
}
