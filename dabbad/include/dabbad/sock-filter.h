/**
 * \file sock-filter.h
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */

/* __LICENSE_HEADER_BEGIN__ */

/*
 * Copyright (C) 2013	Emmanuel Roullit <emmanuel.roullit@gmail.com>
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

#ifndef DABBAD_SOCK_FILTER_H
#define	DABBAD_SOCK_FILTER_H

#include <libdabba-rpc/rpc.h>

struct sock_fprog;

void dabbad_sfp_destroy(struct sock_fprog *sfp);
void dabbad_pbuf_sfp_destroy(Dabba__SockFprog * pbuf_sfp);
int dabbad_pbuf_sfp_2_sfp(Dabba__SockFprog * pbuf_sf, struct sock_fprog *sfp);
int dabbad_sfp_2_pbuf_sfp(struct sock_fprog *sfp, Dabba__SockFprog * pbuf_sfp);

#endif				/* DABBAD_SOCK_FILTER_H */
