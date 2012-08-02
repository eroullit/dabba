/**
 * \file help.c
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
#include <assert.h>
#include <getopt.h>

static const char _usage[] =
    "usage: dabbad [<args>]\n\n" "The available options are:\n";

/**
 * \brief Show dabbad usage options
 */

void show_usage(const struct option * opt)
{
	assert(opt);

	printf("%s", _usage);

	if (opt != NULL) {
		while (opt->name != NULL) {
			printf("  --%s", opt->name);
			if (opt->has_arg == required_argument)
				printf(" <arg>\n");
			else if (opt->has_arg == optional_argument)
				printf(" [arg]\n");
			else
				printf("\n");
			opt++;
		}
	}
}
