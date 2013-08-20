/**
 * \file macros.h
 * \author written by Emmanuel Roullit emmanuel.roullit@gmail.com (c) 2013
 * \date 2013
 */


#ifndef MACROS_H
#define	MACROS_H

#include <stdint.h>

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif				/* offsetof */

/**
 * container_of - cast a member of a structure out to the containing structure
 * \param[in] ptr       the pointer to the member.
 * \param[in] type	the type of the container struct this is embedded in.
 * \param[in] member	the name of the member within the struct.
 */

#ifndef container_of
#define container_of(ptr, type, member)	((type *)(((uint8_t *)(ptr)) - offsetof(type, member)))
#endif				/* container_of */

/**
 * \brief Abort build when expression is equal zero
 * \param[in] e expression
 *
 * Force a compilation error if condition is true, but also produce a
 * result (of value 0 and type \c size_t), so the expression can be used
 * e.g. in a structure initializer (or where-ever else comma expressions
 * aren't permitted).
 */

#ifndef BUILD_BUG_ON_ZERO
#define BUILD_BUG_ON_ZERO(e) (sizeof(char[1 - 2 * !!(e)]) - 1)
#endif				/* BUILD_BUG_ON_ZERO */

/**
 * \brief Check if the variable is an array
 * \param[in] a variable to check
 *
 * Force a compilation error if the variable is not an array
 */

#ifndef __must_be_array
#define __must_be_array(a) \
        BUILD_BUG_ON_ZERO(__builtin_types_compatible_p(typeof(a), typeof(&a[0])))
#endif				/* __must_be_array */

/**
 * \brief Returns the amount of element the array holds
 * \param[in] arr input array
 *
 * Force a compilation error if the variable is not an array
 */

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))
#endif				/* ARRAY_SIZE */

#endif				/* MACROS_H */
