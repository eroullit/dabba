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
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/param.h>
#include <sys/types.h>
#include <ifaddrs.h>

#include <dabbacore/macros.h>
#include <dabbacore/strlcpy.h>
#include <dabbad/dabbad.h>

int dabbad_ifconf_get(struct dabba_ipc_msg *msg)
{
	size_t a = 0;
	struct ifaddrs *ifaddr, *ifa;

	if (getifaddrs(&ifaddr) != 0)
		return -1;

	for (ifa = ifaddr;
	     ifa != NULL && a < ARRAY_SIZE(msg->msg_body.msg.ifconf);
	     ifa = ifa->ifa_next) {
		if (ifa->ifa_addr->sa_family != AF_PACKET)
			continue;

		strlcpy(msg->msg_body.msg.ifconf[a].name, ifa->ifa_name,
			IFNAMSIZ);
		a++;
	}

	msg->msg_body.elem_nr = a;

	freeifaddrs(ifaddr);

	return 0;
}

int dabbad_handle_msg(struct dabba_ipc_msg *msg)
{
	int rc;
	assert(msg);

	switch (msg->msg_body.type) {
	case DABBA_IFCONF:
		rc = dabbad_ifconf_get(msg);
		break;
	default:
		rc = -1;
		errno = EINVAL;
		break;
	}

	return rc;
}

int main(int argc, char **argv)
{
	int qid;
	ssize_t rcv;
	int snd;
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
		rcv = msgrcv(qid, &msg, sizeof(msg.msg_body), 0, 0);

		if (rcv <= 0) {
			perror("Error while receiving IPC msg");
			return EXIT_FAILURE;
		}

		rcv = dabbad_handle_msg(&msg);

		if (rcv != 0) {
			perror("Error while handling IPC msg");
		}

		snd = msgsnd(qid, &msg, sizeof(msg.msg_body), 0);

		if (snd < 0) {
			perror("Error while sending back IPC msg");
			return EXIT_FAILURE;
		}
	}

	return EXIT_SUCCESS;
}
