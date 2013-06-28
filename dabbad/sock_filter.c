/**
 * \file sock_filter.c
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

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <linux/filter.h>

#include <libdabba/sock_filter.h>
#include <libdabba-rpc/rpc.h>

int dabbad_sock_filter_parse(Dabba__SockFprog * pbuf_sf, struct sock_fprog *sfp)
{
	size_t a;

	assert(pbuf_sf);
	assert(sfp);

	sfp->filter = calloc(pbuf_sf->n_filter, sizeof(*sfp->filter));

	if (sfp->filter)
		return ENOMEM;

	for (a = 0; a < pbuf_sf->n_filter; a++) {
		sfp->filter[a].code = pbuf_sf->filter[a]->code;
		sfp->filter[a].jt = pbuf_sf->filter[a]->jt;
		sfp->filter[a].jf = pbuf_sf->filter[a]->jf;
		sfp->filter[a].k = pbuf_sf->filter[a]->k;
	}

	sfp->len = pbuf_sf->n_filter;

	if (!sock_filter_is_valid(sfp)) {
		free(sfp->filter);
		memset(sfp, 0, sizeof(*sfp));
		return EINVAL;
	}

	return 0;
}
