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
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <dabbad/ipc.h>
#include <dabbad/interface.h>
#include <dabbad/capture.h>
#include <dabbad/thread.h>

static int dabbad_handle_bulk_get_msg(struct dabba_ipc_msg *msg)
{
	int rc;

	switch (msg->msg_body.type) {
	case DABBA_INTERFACE_LIST:
		rc = dabbad_interface_bulk_get(msg, interface_list);
		break;
	case DABBA_INTERFACE_DRIVER:
		rc = dabbad_interface_bulk_get(msg, interface_driver);
		break;
	case DABBA_INTERFACE_SETTINGS:
		rc = dabbad_interface_bulk_get(msg, interface_settings);
		break;
	case DABBA_INTERFACE_PAUSE:
		rc = dabbad_interface_bulk_get(msg, interface_pause);
		break;
	case DABBA_INTERFACE_COALESCE:
		rc = dabbad_interface_bulk_get(msg, interface_coalesce);
		break;
	case DABBA_INTERFACE_OFFLOAD:
		rc = dabbad_interface_bulk_get(msg, interface_offload);
		break;
	case DABBA_THREAD_LIST:
		rc = dabbad_thread_list(msg);
		break;
	case DABBA_THREAD_CAP_LIST:
		rc = dabbad_thread_cap_list(msg);
		break;
	default:
		rc = -1;
		errno = ENOSYS;
		break;
	}

	return rc;
}

static int dabbad_handle_filter_get_msg(struct dabba_ipc_msg *msg)
{
	int rc;

	switch (msg->msg_body.type) {
	case DABBA_INTERFACE_LIST:
		rc = dabbad_interface_filter_get(msg, interface_list_name_get,
						 interface_list);
		break;
	default:
		rc = -1;
		errno = ENOSYS;
		break;
	}

	return rc;
}

static int dabbad_handle_filter_modify_msg(struct dabba_ipc_msg *msg)
{
	int rc;

	switch (msg->msg_body.type) {
	case DABBA_INTERFACE_MODIFY:
		rc = dabbad_interface_modify(msg);
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

static int dabbad_handle_get_msg(struct dabba_ipc_msg *msg)
{
	int rc;

	switch (msg->msg_body.method_type) {
	case MT_BULK:
		rc = dabbad_handle_bulk_get_msg(msg);
		break;
	case MT_FILTERED:
		rc = dabbad_handle_filter_get_msg(msg);
		break;
	default:
		rc = -1;
		errno = ENOSYS;
		break;
	}

	return rc;
}

static int dabbad_handle_modify_msg(struct dabba_ipc_msg *msg)
{
	return dabbad_handle_filter_modify_msg(msg);
}

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

	printf("op type %i\n", msg->msg_body.op_type);
	switch (msg->msg_body.op_type) {
	case OP_GET:
		rc = dabbad_handle_get_msg(msg);
		break;

	case OP_MODIFY:
		rc = dabbad_handle_modify_msg(msg);
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

int dabbad_ipc_msg_destroy(void)
{
	return msgctl(dabba_get_ipc_queue_id(0), IPC_RMID, NULL);
}

/**
 * \brief Create dabbad Inter Process Communication message queue
 * \return 0 on success, msgget(2) return code on failure.
 *
 * This function removes all previously created IPC message queue.
 */

int dabbad_ipc_msg_create(void)
{
	return dabba_get_ipc_queue_id(IPC_CREAT | IPC_EXCL | 0660);
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

int dabbad_ipc_msg_poll(const int qid)
{
	ssize_t rcv;
	int snd;
	struct dabba_ipc_msg msg;

	if (qid < 0) {
		perror("Cannot get IPC id");
		return errno;
	}

	for (;;) {
		memset(&msg, 0, sizeof(msg));
		rcv =
		    msgrcv(qid, &msg, sizeof(msg.msg_body), DABBA_CLIENT_MSG,
			   0);

		if (rcv <= 0) {
			perror("Error while receiving IPC msg");
			return errno;
		}

		assert(msg.mtype == DABBA_CLIENT_MSG);

		rcv = dabbad_handle_msg(&msg);

		msg.mtype = DABBA_DAEMON_MSG;
		msg.msg_body.error = rcv;

		if (rcv != 0) {
			perror("Error while handling IPC msg");
		}

		usleep(100);

		snd = msgsnd(qid, &msg, sizeof(msg.msg_body), 0);

		if (snd < 0) {
			perror("Error while sending back IPC msg");
			return errno;
		}
	}

	return 0;
}
