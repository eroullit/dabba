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

#ifndef DABBAD_THREAD_H
#define	DABBAD_THREAD_H

#include <libdabba/thread.h>

struct packet_thread *dabbad_thread_type_first(const enum packet_thread_type
					       type);
struct packet_thread *dabbad_thread_type_next(struct packet_thread *pkt_thread,
					      const enum packet_thread_type
					      type);
struct packet_thread *dabbad_thread_data_get(const pthread_t thread_id);
int dabbad_thread_start(struct packet_thread *pkt_thread,
			void *(*func) (void *arg), void *arg);
int dabbad_thread_stop(struct packet_thread *pkt_thread);

#endif				/* DABBAD_THREAD_H */
