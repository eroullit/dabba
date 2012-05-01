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
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <dabba/ipc.h>

int dabba_ipc_msg(struct dabba_ipc_msg *msg)
{
	int qid;
	ssize_t rcv;
	int snd;

	assert(msg);

	qid = dabba_get_ipc_queue_id(0660);

	if (qid < 0) {
		perror("Cannot get IPC id");
		return errno;
	}

	snd = msgsnd(qid, msg, sizeof(msg->msg_body), 0);

	if (snd < 0) {
		perror("Error while sending IPC msg");
		return errno;
	}

	usleep(100);

	rcv = msgrcv(qid, msg, sizeof(msg->msg_body), 0, 0);

	if (rcv <= 0) {
		perror("Error while receiving IPC msg");
		return errno;
	}

	if (msg->msg_body.error != 0) {
		printf("The daemon reported: %s\n",
		       strerror(msg->msg_body.error));
		return msg->msg_body.error;
	}

	return 0;
}
