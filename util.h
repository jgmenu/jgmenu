#ifndef UTIL_H
#define UTIL_H

#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

void die(const char *err, ...);
void spawn(const char *arg);

static inline void *xmalloc(size_t size)
{
	void *ret = malloc(size);
	if (!ret)
		die("Out of memory, malloc failed");
	return ret;
}

static inline void *xrealloc(void *ptr, size_t size)
{
	void *ret = realloc(ptr, size);
	if (!ret)
		die("Out of memory, realloc failed");
	return ret;
}

static inline void *xcalloc(size_t nmemb, size_t size)
{
	void *ret = calloc(nmemb, size);
	if (!ret)
		die("Out of memory, calloc failed");
	return ret;
}

#endif /* UTIL_H */
