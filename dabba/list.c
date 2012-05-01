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
#include <assert.h>
#include <errno.h>
#include <dabbacore/macros.h>
#include <dabba/ipc.h>

static void display_interface_list_header(void)
{
	printf("---\n");
	printf("  interfaces:\n");
}

static void display_interface_list(const struct dabba_ipc_msg const *msg)
{
	size_t a;

	assert(msg);
	assert(msg->msg_body.elem_nr <= ARRAY_SIZE(msg->msg_body.msg.ifconf));

	for (a = 0; a < msg->msg_body.elem_nr; a++) {
		printf("    - %s\n", msg->msg_body.msg.ifconf[a].name);
	}
}

static void prepare_list_query(struct dabba_ipc_msg *msg)
{
	assert(msg);
	msg->mtype = 1;
	msg->msg_body.type = DABBA_IFCONF;
}

static void display_list_msg_header(const struct dabba_ipc_msg const *msg)
{
	assert(msg);

	switch (msg->msg_body.type) {
	case DABBA_IFCONF:
		display_interface_list_header();
		break;
	default:
		break;
	}
}

static void display_list_msg(const struct dabba_ipc_msg const *msg)
{
	assert(msg);

	switch (msg->msg_body.type) {
	case DABBA_IFCONF:
		display_interface_list(msg);
		break;
	default:
		break;
	}
}

int cmd_list(int argc, const char **argv)
{
	int rc;
	struct dabba_ipc_msg msg;

	assert(argc >= 0);
	assert(argv);

	memset(&msg, 0, sizeof(msg));
	prepare_list_query(&msg);
	display_list_msg_header(&msg);

	do {
		msg.msg_body.offset += msg.msg_body.elem_nr;
		msg.msg_body.elem_nr = 0;

		rc = dabba_ipc_msg(&msg);

		if (rc)
			break;

		display_list_msg(&msg);
	} while (msg.msg_body.elem_nr);

	return rc;
}
