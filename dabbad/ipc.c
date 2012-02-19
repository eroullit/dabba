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
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <dabbacore/macros.h>
#include <dabbad/ipc.h>
#include <dabbad/list.h>
#include <dabbad/capture.h>

static int dabbad_handle_msg(struct dabba_ipc_msg *msg)
{
	int rc;
	assert(msg);

	switch (msg->msg_body.type) {
	case DABBA_IFCONF:
		rc = dabbad_ifconf_get(msg);
		break;
	case DABBA_CAPTURE:
		rc = dabbad_capture_start(msg);
		break;
	default:
		rc = -1;
		errno = EINVAL;
		break;
	}

	return rc;
}

static int dabbad_ipc_sem_init(int qid)
{
	int semid;
	union semun {
		int val;
	} argument;

	semid = semget(qid, 1, IPC_CREAT | 0660);

	if (semid < 0)
		return semid;

	return semctl(semid, 0, SETVAL, argument);
}

int dabbad_ipc_msg_init(void)
{
	return msgctl(dabba_get_ipc_queue_id(0), IPC_RMID, NULL);
}

int dabbad_ipc_msg_poll(void)
{
	struct sembuf semops[1];
	int qid, semid, rc;
	ssize_t rcv;
	int snd;
	struct dabba_ipc_msg msg;

	qid = dabba_get_ipc_queue_id(IPC_CREAT | IPC_EXCL | 0660);

	if (qid < 0) {
		perror("Cannot get IPC id");
		return errno;
	}

	if (dabbad_ipc_sem_init(qid)) {
		perror("Cannot initialise IPC semaphore");
		return errno;
	}

	semid = semget(qid, 1, 0660);

	if (semid < 0) {
		perror("Cannot get IPC semaphore");
		return errno;
	}

	for (;;) {
		memset(&msg, 0, sizeof(msg));
		rcv = msgrcv(qid, &msg, sizeof(msg.msg_body), 0, 0);

		if (rcv <= 0) {
			perror("Error while receiving IPC msg");
			return errno;
		}

		rcv = dabbad_handle_msg(&msg);

		if (rcv != 0) {
			perror("Error while handling IPC msg");
		}

		snd = msgsnd(qid, &msg, sizeof(msg.msg_body), 0);

		if (snd < 0) {
			perror("Error while sending back IPC msg");
			return errno;
		}

		semops[0].sem_num = 0;
		semops[0].sem_op = 1;
		semops[0].sem_flg = 0;

		rc = semop(semid, semops, ARRAY_SIZE(semops));

		if (rc) {
			perror("Error on IPC semaphore operation");
			return errno;
		}
	}

	return 0;
}
