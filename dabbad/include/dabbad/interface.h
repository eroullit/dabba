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

#ifndef INTERFACE_H
#define	INTERFACE_H

#include <dabbad/dabbad.h>

struct nl_object;

void interface_list(struct nl_object *obj, void *arg);
void interface_driver(struct nl_object *obj, void *arg);
void interface_settings(struct nl_object *obj, void *arg);
void interface_pause(struct nl_object *obj, void *arg);
void interface_coalesce(struct nl_object *obj, void *arg);
void interface_offload(struct nl_object *obj, void *arg);

int dabbad_interface_get_all(struct dabba_ipc_msg *msg,
			     void (*msg_cb) (struct nl_object * obj,
					     void *arg));
int dabbad_interface_modify(struct dabba_ipc_msg *msg);

#endif				/* INTERFACE_H */
