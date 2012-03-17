/* __LICENSE_HEADER_BEGIN__ */

/*
 * Copyright (C) 2009-2012	Emmanuel Roullit <emmanuel.roullit@gmail.com>
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
#include <dabbacore/strlcpy.h>
#include <dabbacore/packet_mmap.h>
#include <dabba/help.h>
#include <dabba/ipc.h>
#include <dabbad/dabbad.h>

enum capture_option {
	OPT_CAPTURE_DEVICE,
	OPT_CAPTURE_PCAP,
	OPT_CAPTURE_SIZE
};

static struct option *capture_options_get(void)
{
	static struct option capture_option[] = {
		{"device", required_argument, NULL, OPT_CAPTURE_DEVICE},
		{"pcap", required_argument, NULL, OPT_CAPTURE_PCAP},
		{"size", required_argument, NULL, OPT_CAPTURE_SIZE},
		{NULL, 0, NULL, 0},
	};

	return capture_option;
}

static int prepare_capture_query(int argc, char **argv,
				 struct dabba_ipc_msg *msg)
{
	struct dabba_capture *capture_msg = msg->msg_body.msg.capture;
	int ret = 0;
	int rc = 0;

	assert(msg);
	msg->mtype = 1;
	msg->msg_body.type = DABBA_CAPTURE_START;

	while ((ret =
		getopt_long_only(argc, argv, "", capture_options_get(),
				 NULL)) != EOF) {
		switch (ret) {
		case OPT_CAPTURE_DEVICE:
			strlcpy(capture_msg->dev_name, optarg,
				sizeof(capture_msg->dev_name));
			break;

		case OPT_CAPTURE_PCAP:
			strlcpy(capture_msg->pcap_name, optarg,
				sizeof(capture_msg->pcap_name));
			break;
		case OPT_CAPTURE_SIZE:
			capture_msg->size = strtoull(optarg, NULL, 10);
			break;
		default:
			show_usage(capture_options_get());
			rc = -1;
			break;
		}
	}

	/* Assume conservative values for now */
	capture_msg->page_order = 10;
	capture_msg->frame_size = PACKET_MMAP_ETH_FRAME_LEN;

	return rc;
}

int cmd_capture(int argc, const char **argv)
{
	struct dabba_ipc_msg msg;
	int rc;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));

	rc = prepare_capture_query(argc, (char **)argv, &msg);

	if (rc)
		return rc;

	/* For now, just one capture request at a time */
	msg.msg_body.elem_nr = 1;

	return dabba_ipc_msg(&msg);
}
