/**
 * \file ipc.c
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
#include <assert.h>
#include <dabbad/ipc.h>
#include <dabbad/list.h>
#include <dabbad/capture.h>
#include <dabbad/thread.h>

/**
 * \internal
 * \brief Handle request received in incoming IPC message
 * \return 0 on success, -1 on failure. Set errno accordingly on failure.
 *
 * This function checks the operation to perform in the IPC message and passes 
 * it to the right processing function.
 */

static int dabbad_handle_msg(struct dabba_ipc_msg *msg)
{
	int rc;
	assert(msg);

	switch (msg->msg_body.type) {
	case DABBA_IFCONF:
		rc = dabbad_ifconf_get(msg);
		break;
	case DABBA_CAPTURE_START:
		rc = dabbad_capture_start(msg);
		break;
	case DABBA_CAPTURE_LIST:
		rc = dabbad_capture_list(msg);
		break;
	case DABBA_CAPTURE_STOP:
		rc = dabbad_capture_stop(msg);
		break;
	case DABBA_THREAD_LIST:
		rc = dabbad_thread_list(msg);
		break;
	case DABBA_THREAD_MODIFY:
		rc = dabbad_thread_modify(msg);
		break;
	default:
		rc = -1;
		errno = ENOSYS;
		break;
	}

	return rc;
}

/**
 * \brief Initialize dabbad Inter Process Communication message queue
 * \return 0 on success, msgctl(2) return code on failure.
 *
 * This function removes all previously created IPC message queue.
 */

int dabbad_ipc_msg_init(void)
{
	return msgctl(dabba_get_ipc_queue_id(0), IPC_RMID, NULL);
}

/**
 * \brief Flush dabbad Inter Process Communication message queue
 * \param[in]       qid	        Dabba daemon message queue
 *
 * This function removes all previously created IPC message queue.
 */

void dabbad_ipc_msg_flush(const int qid)
{
	struct dabba_ipc_msg msg;
	ssize_t rcv;

	do {
		rcv =
		    msgrcv(qid, &msg, sizeof(msg.msg_body), 0,
			   IPC_NOWAIT | MSG_NOERROR);
	} while (rcv > 0);
}

/**
 * \brief Wait for IPC messages for dabbad.
 * \return 0 on success, else otherwise.
 *
 * Dabbad main loop. It awaits for incoming IPC messages.
 * When a message has been received, it is passed to a message handler which
 * performs the request.
 * 
 * When done, an IPC message is sent back with the requested information, 
 * error code, etc.
 */

int dabbad_ipc_msg_poll(void)
{
	int qid;
	ssize_t rcv;
	int snd;
	struct dabba_ipc_msg msg;

	qid = dabba_get_ipc_queue_id(IPC_CREAT | IPC_EXCL | 0660);

	if (qid < 0) {
		perror("Cannot get IPC id");
		return errno;
	}

	dabbad_ipc_msg_flush(qid);

	for (;;) {
		memset(&msg, 0, sizeof(msg));
		rcv = msgrcv(qid, &msg, sizeof(msg.msg_body), 0, 0);

		if (rcv <= 0) {
			perror("Error while receiving IPC msg");
			return errno;
		}

		rcv = dabbad_handle_msg(&msg);

		msg.msg_body.error = rcv;

		if (rcv != 0) {
			perror("Error while handling IPC msg");
		}

		snd = msgsnd(qid, &msg, sizeof(msg.msg_body), 0);

		if (snd < 0) {
			perror("Error while sending back IPC msg");
			return errno;
		}
	}

	return 0;
}
