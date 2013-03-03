/**
 * \file cli.c
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

#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <linux/ethtool.h>

int str2bool(const char *const str, int *const val)
{
	int rc = 0;

	assert(str);
	assert(val);

	if (!strcasecmp(str, "false"))
		*val = 0;
	else if (!strcasecmp(str, "true"))
		*val = 1;
	else
		rc = EINVAL;

	return rc;
}

int str2speed(const char *const str, uint32_t * const speed)
{
	int rc = 0;

	assert(str);
	assert(speed);

	if (!strcmp(str, "10"))
		*speed = SPEED_10;
	else if (!strcmp(str, "100"))
		*speed = SPEED_100;
	else if (!strcmp(str, "1000"))
		*speed = SPEED_1000;
	else if (!strcmp(str, "2500"))
		*speed = SPEED_2500;
	else if (!strcmp(str, "10000"))
		*speed = SPEED_10000;
	else {
		*speed = SPEED_UNKNOWN;
		rc = EINVAL;
	}

	return rc;
}

int str2duplex(const char *const str, int *const duplex)
{
	int rc = 0;

	assert(str);
	assert(duplex);

	if (!strcmp(str, "full"))
		*duplex = DUPLEX_FULL;
	else if (!strcmp(str, "half"))
		*duplex = DUPLEX_HALF;
	else {
		*duplex = DUPLEX_UNKNOWN;
		rc = EINVAL;
	}

	return rc;
}
