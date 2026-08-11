/*
 * These functions have been copied from:
 * https://github.com/git/git/blob/master/compat/strlcpy.c
 */

#include "compat.h"
#include "banned.h"

size_t gitstrlcpy(char *dest, const char *src, size_t size)
{
	size_t ret = strlen(src);

	if (size) {
		size_t len = (ret >= size) ? size - 1 : ret;

		memcpy(dest, src, len);
		dest[len] = '\0';
	}
	return ret;
}
