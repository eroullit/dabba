/**
 * \file macros.h
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2009-2011
 * \date 2011
 */

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

#ifndef MACROS_H
#define	MACROS_H

#include <stdint.h>

/**
 * \brief Abort build when expression is equal zero
 * \param[in] e expression
 *
 * Force a compilation error if condition is true, but also produce a
 * result (of value 0 and type size_t), so the expression can be used
 * e.g. in a structure initializer (or where-ever else comma expressions
 * aren't permitted).
 */

#define BUILD_BUG_ON_ZERO(e) (sizeof(char[1 - 2 * !!(e)]) - 1)

/**
 * \brief Check if the variable is an array
 * \param[in] e variable to check
 *
 * Force a compilation error if the variable is not an array
 */

#define __must_be_array(a) \
        BUILD_BUG_ON_ZERO(__builtin_types_compatible_p(typeof(a), typeof(&a[0])))

/**
 * \brief Returns the amount of element the array holds
 * \param[in] arr input array
 *
 * Force a compilation error if the variable is not an array
 */

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))

/**
 * \brief Tell if a integer is a power of 2
 * \param[in] n Integer to test
 * \return 1 if true, 0 otherwise
 */

static inline int is_power_of_2(const uint64_t n)
{
	return (n != 0 && ((n & (n - 1)) == 0));
}

/**
 * \brief Returns the minimum value between two variables
 * \param[in] x First variable
 * \param[in] y Second variable
 * \return minimum value between the two variable
 * \note Force a compilation error if the variable types don't match
 */

#define min(x, y)                               \
        __extension__                           \
        ({                                      \
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	(void) (&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; })

/**
 * \brief Returns the maximum value between two variables
 * \param[in] x First variable
 * \param[in] y Second variable
 * \return maximum value between the two variable
 * \note Force a compilation error if the variable types don't match
 */

#define max(x, y)                               \
        __extension__                           \
        ({                                      \
	typeof(x) _max1 = (x);			\
	typeof(y) _max2 = (y);			\
	(void) (&_max1 == &_max2);		\
	_max1 > _max2 ? _max1 : _max2; })

#endif				/* MACROS_H */
