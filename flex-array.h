#ifndef FLEX_ARRAY_H
#define FLEX_ARRAY_H

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>

#include "util.h"

/*
 * flex-array.h
 *
 * Some notes to make sense of the code.
 *
 * Flex-arrays:
 *   - Flex-arrays became standard in C99 and are defined by "array[]" (at the
 *     end of a struct)
 *   - Pre-C99 flex-arrays can be accomplished by "array[1]"
 *   - There is a GNU extension where they are defined using "array[0]"
 *     Allegedly there is/was a bug in gcc whereby foo[1] generated incorrect
 *     code, so it's safest to use [0] (https://lkml.org/lkml/2015/2/18/407).
 *
 * For C89 and C90, __STDC__ is 1
 * For later standards, __STDC_VERSION__ is defined according to the standard.
 * For example: 199901L or 201112L
 *
 * Whilst we're on the subject, in version 5 of gcc, the default std was
 * changed from gnu89 to gnu11. In jgmenu, CFLAGS therefore contains -std=gnu89
 * You can check your default gcc std by doing:
 * gcc -dM -E - </dev/null | grep '__STDC_VERSION__\|__STDC__'
 *
 * The code below is copied from git's git-compat-util.h in support of
 * hashmap.c
 */

#ifndef FLEX_ARRAY
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L) && \
	(!defined(__SUNPRO_C) || (__SUNPRO_C > 0x580))
# define FLEX_ARRAY /* empty */
#elif defined(__GNUC__)
# if (__GNUC__ >= 3)
#  define FLEX_ARRAY /* empty */
# else
#  define FLEX_ARRAY 0 /* older GNU extension */
# endif
#endif

/* Otherwise, default to safer but a bit wasteful traditional style */
#ifndef FLEX_ARRAY
# define FLEX_ARRAY 1
#endif
#endif

#define bitsizeof(x) (CHAR_BIT * sizeof(x))

#define maximum_signed_value_of_type(a) \
	(INTMAX_MAX >> (bitsizeof(intmax_t) - bitsizeof(a)))

#define maximum_unsigned_value_of_type(a) \
	(UINTMAX_MAX >> (bitsizeof(uintmax_t) - bitsizeof(a)))

/*
 * Signed integer overflow is undefined in C, so here's a helper macro
 * to detect if the sum of two integers will overflow.
 * Requires: a >= 0, typeof(a) equals typeof(b)
 */
#define signed_add_overflows(a, b) \
	((b) > maximum_signed_value_of_type(a) - (a))

#define unsigned_add_overflows(a, b) \
	((b) > maximum_unsigned_value_of_type(a) - (a))

static inline size_t st_add(size_t a, size_t b)
{
	if (unsigned_add_overflows(a, b))
		die("size_t overflow: %llu + %llu", (uintmax_t)a, (uintmax_t)b);
	return a + b;
}

#define st_add3(a, b, c) st_add(st_add((a), (b)), (c))
#define st_add4(a, b, c, d) st_add(st_add3((a), (b), (c)), (d))

/*
 * These functions help you allocate structs with flex arrays, and copy
 * the data directly into the array. For example, if you had:
 *
 *   struct foo {
 *     int bar;
 *     char name[FLEX_ARRAY];
 *   };
 *
 * you can do:
 *
 *   struct foo *f;
 *   FLEX_ALLOC_MEM(f, name, src, len);
 *
 * to allocate a "foo" with the contents of "src" in the "name" field.
 * The resulting struct is automatically zero'd, and the flex-array field
 * is NUL-terminated (whether the incoming src buffer was or not).
 *
 * The FLEXPTR_* variants operate on structs that don't use flex-arrays,
 * but do want to store a pointer to some extra data in the same allocated
 * block. For example, if you have:
 *
 *   struct foo {
 *     char *name;
 *     int bar;
 *   };
 *
 * you can do:
 *
 *   struct foo *f;
 *   FLEXPTR_ALLOC_STR(f, name, src);
 *
 * and "name" will point to a block of memory after the struct, which will be
 * freed along with the struct (but the pointer can be repointed anywhere).
 *
 * The *_STR variants accept a string parameter rather than a ptr/len
 * combination.
 *
 * Note that these macros will evaluate the first parameter multiple
 * times, and it must be assignable as an lvalue.
 */
#define FLEX_ALLOC_MEM(x, flexname, buf, len) do { \
	size_t flex_array_len_ = (len); \
	(x) = xcalloc(1, st_add3(sizeof(*(x)), flex_array_len_, 1)); \
	memcpy((void *)(x)->flexname, (buf), flex_array_len_); \
} while (0)
#define FLEXPTR_ALLOC_MEM(x, ptrname, buf, len) do { \
	size_t flex_array_len_ = (len); \
	(x) = xcalloc(1, st_add3(sizeof(*(x)), flex_array_len_, 1)); \
	memcpy((x) + 1, (buf), flex_array_len_); \
	(x)->ptrname = (void *)((x) + 1); \
} while (0)
#define FLEX_ALLOC_STR(x, flexname, str) \
	FLEX_ALLOC_MEM((x), flexname, (str), strlen(str))
#define FLEXPTR_ALLOC_STR(x, ptrname, str) \
	FLEXPTR_ALLOC_MEM((x), ptrname, (str), strlen(str))

#endif
