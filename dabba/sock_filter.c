/**
 * \file sock_filter.c
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (C) 2013
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <libdabba-rpc/rpc.h>

void sock_filter_destroy(Dabba__SockFprog * pbuf_sfp)
{
	size_t a;

	assert(pbuf_sfp);

	for (a = 0; a < pbuf_sfp->n_filter; a++)
		free(pbuf_sfp->filter[a]);

	free(pbuf_sfp->filter);
	dabba__sock_fprog__init(pbuf_sfp);
}

int sock_filter_parse(const char *const sf_path, Dabba__SockFprog * pbuf_sfp)
{
	int ret;
	char buff[128] = { 0 };
	Dabba__SockFilter *sf;
	Dabba__SockFilter **tmp;

	assert(pbuf_sfp);
	assert(sf_path);

	FILE *fp = fopen(sf_path, "r");
	if (!fp) {
		return (-1);
	}

	memset(buff, 0, sizeof(buff));

	while (fgets(buff, sizeof(buff), fp) != NULL) {
		/* We're using evil sscanf, so we have to assure
		   that we don't get into a buffer overflow ... */
		buff[sizeof(buff) - 1] = 0;

		/* Not a socket filter instruction. Skip this line */
		if (buff[0] != '{')
			continue;

		sf = malloc(sizeof(*sf));

		if (!sf)
			return ENOMEM;

		dabba__sock_filter__init(sf);

		ret = sscanf(buff, "{ 0x%x, %d, %d, 0x%08x },",
			     (unsigned int *)((void *)&(sf->code)),
			     (int *)((void *)&(sf->jt)),
			     (int *)((void *)&(sf->jf)), &(sf->k));

		/* No valid bpf opcode format or a syntax error */
		if (ret != 4)
			return 0;

		tmp =
		    realloc(pbuf_sfp->filter,
			    sizeof(*pbuf_sfp->filter) * (pbuf_sfp->n_filter +
							 1));

		if (!tmp)
			return ENOMEM;

		pbuf_sfp->filter = tmp;
		pbuf_sfp->filter[pbuf_sfp->n_filter] = sf;
		pbuf_sfp->n_filter++;

		memset(buff, 0, sizeof(buff));
	}

	return fclose(fp);
}
