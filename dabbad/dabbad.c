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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/param.h>

#include <dabbad/dabbad.h>

int main(int argc, char **argv)
{
	int qid;
	ssize_t rcv;
	struct dabba_ipc_msg msg;

	assert(argc);
	assert(argv);
	assert(daemon(0, 0) == 0);

	memset(&msg, 0, sizeof(msg));

	msgctl(dabba_get_ipc_queue_id(0), IPC_RMID, NULL);

	qid = dabba_get_ipc_queue_id(IPC_CREAT | IPC_EXCL | 0660);

	if (qid < 0) {
		perror("Cannot get IPC id");
		return EXIT_FAILURE;
	}

	for (;;) {
		rcv = msgrcv(qid, &msg, sizeof(msg.msg), 0, 0);

		if (rcv <= 0) {
			perror("Error while receiving IPC msg");
			return EXIT_FAILURE;
		}

		/* TODO Message handling */
	}

	return EXIT_SUCCESS;
}
