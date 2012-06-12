/**
 * \file strlcpy.c
 * \author written by Linus Torvalds  torvalds@osdl.org (c) 1991, 1992
 * \date 1992
 */

/*
 * Copyright (C) 1991, 1992  Linus Torvalds
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
 */

/* strlcpy.c taken from the Linux kernel and adapted */

#include <string.h>

/**
 * \brief Copy a %NUL terminated string into a sized buffer
 * \param[in] dest	Where to copy the string to
 * \param[in] src	Where to copy the string from
 * \param[in] size	Size of destination buffer
 *
 * Compatible with *BSD: the result is always a valid
 * NUL-terminated string that fits in the buffer (unless,
 * of course, the buffer size is zero). It does not pad
 * out the result like \c strncpy(3) does.
 */

size_t strlcpy(char *dest, const char *src, size_t size)
{
	size_t ret = strlen(src);

	if (size) {
		size_t len = (ret >= size) ? size - 1 : ret;
		memcpy(dest, src, len);
		dest[len] = '\0';
	}

	return ret;
}
