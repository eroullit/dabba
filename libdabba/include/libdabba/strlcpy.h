/**
 * \file strlcpy.h
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

#ifndef _STRLCPY_H_
#define _STRLCPY_H_

#include <stddef.h>

size_t strlcpy(char *dest, const char *src, size_t size);

#endif				/* _STRLCPY_H_ */
