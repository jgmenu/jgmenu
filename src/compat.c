/*
 * These functions have been copied from:
 * https://github.com/git/git/blob/master/compat/strcasestr.c
 * https://github.com/git/git/blob/master/compat/strlcpy.c
 */

#include "compat.h"
#include "banned.h"

/* clang-format off */
char *gitstrcasestr(const char *haystack, const char *needle)
{
	int nlen = strlen(needle);
	int hlen = strlen(haystack) - nlen + 1;
	int i;

	for (i = 0; i < hlen; i++) {
		int j;

		for (j = 0; j < nlen; j++) {
			unsigned char c1 = haystack[i + j];
			unsigned char c2 = needle[j];

			if (toupper(c1) != toupper(c2))
				goto next;
		}
		return (char *)haystack + i;
next:
		;
	}
	return NULL;
}

/* clang-format on */

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
