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

#ifndef DABBAD_H
#define	DABBAD_H

#include <stdint.h>
#include <limits.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

enum dabba_msg_type {
	DABBA_IFCONF,
	DABBA_CAPTURE_START,
	DABBA_CAPTURE_LIST,
	DABBA_CAPTURE_STOP
};

struct dabba_msg_buf {
	uint8_t buf[1024];
};

struct dabba_ifconf {
	char name[IFNAMSIZ];
};

struct dabba_capture {
	char pcap_name[NAME_MAX];	/* find name length limit */
	char dev_name[IFNAMSIZ];
	pthread_t thread_id;
	uint64_t size;
	uint32_t frame_size;
	uint8_t page_order;
};

#define DABBA_IFCONF_MAX_SIZE (sizeof(struct dabba_msg_buf)/sizeof(struct dabba_ifconf))
#define DABBA_CAPTURE_MAX_SIZE (sizeof(struct dabba_msg_buf)/sizeof(struct dabba_capture))

struct dabba_ipc_msg {
	long mtype;

	struct dabba_msg {
		uint16_t type;
		uint16_t elem_nr;
		uint16_t offset;

		union dabba_info {
			struct dabba_msg_buf buf;
			struct dabba_ifconf ifconf[DABBA_IFCONF_MAX_SIZE];
			struct dabba_capture capture[DABBA_CAPTURE_MAX_SIZE];
		} msg;
	} msg_body;
};

#ifndef DABBAD_PID_FILE
#define DABBAD_PID_FILE "/tmp/dabba.pid"
#endif				/* DABBAD_PID_FILE */

static inline int dabba_get_ipc_queue_id(int flags)
{
	key_t key = ftok(DABBAD_PID_FILE, 0xDABADABA);

	if (key == -1)
		return key;

	return msgget(key, flags);
}
#endif				/* DABBAD_H */
