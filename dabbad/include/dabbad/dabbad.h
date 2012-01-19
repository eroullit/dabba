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
#include <net/if.h>

enum dabba_msg_type {
	DABBA_IFCONF
};

struct dabba_msg_buf {
	char buf[1024];
};

struct dabba_ifconf {
	char if_name[IFNAMSIZ];
};

struct dabba_ipc_msg {
	long mtype;

	struct dabba_msg {
		uint16_t type;
		union {
			struct dabba_msg_buf buf;
			struct dabba_ifconf ifconf[sizeof(struct dabba_msg_buf)
						   /
						   sizeof(struct dabba_ifconf)];
		} msg;
	} msg;
};

static inline int dabba_get_ipc_queue_id(int flags)
{
	return msgget(ftok("/tmp/dabba", 0xDABADABA), flags);
}
#endif				/* DABBAD_H */
