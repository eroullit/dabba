/* __LICENSE_HEADER_BEGIN__ */

/*
 * Copyright (C) 2009-2011	Emmanuel Roullit <emmanuel.roullit@gmail.com>
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
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <dabbacore/strlcpy.h>
#include <stdint.h>

#define LOREM_BEGIN "Lorem"
#define LOREM_END "ipsum dolor sit amet."

int main(int argc, char **argv)
{
	const char const double_lorem[] =
	    LOREM_BEGIN " " LOREM_END " " LOREM_BEGIN " " LOREM_END;
	const char const lorem[] = LOREM_BEGIN " " LOREM_END;
	const char const lorem_short[] = LOREM_BEGIN;

	char target[sizeof(lorem)] = { 0 };

	assert(argc);
	assert(argv);

	assert(strlcpy(NULL, lorem, 0) == sizeof(lorem) - 1);

	assert(strlcpy(target, lorem, 0) == sizeof(lorem) - 1);
	assert(strcmp(target, "") == 0);

	assert(strlcpy(target, lorem, 1) == sizeof(lorem) - 1);
	assert(strcmp(target, "") == 0);

	assert(strlcpy(target, lorem, sizeof(lorem_short)) ==
	       sizeof(lorem) - 1);
	assert(strcmp(target, lorem_short) == 0);

	assert(strlcpy(target, lorem, sizeof(target)) == sizeof(lorem) - 1);
	assert(strcmp(target, lorem) == 0);

	assert(strlcpy(target, double_lorem, sizeof(target)) ==
	       sizeof(double_lorem) - 1);
	assert(strcmp(target, lorem) == 0);

	assert(strlcpy(target, lorem, SIZE_MAX) == sizeof(lorem) - 1);
	assert(strcmp(target, lorem) == 0);

	return (EXIT_SUCCESS);
}
