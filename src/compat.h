#ifndef COMPAT_H
#define COMPAT_H

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define strcasestr gitstrcasestr
char *gitstrcasestr(const char *haystack, const char *needle);

#define strlcpy gitstrlcpy
size_t gitstrlcpy(char *, const char *, size_t);

#endif /* COMPAT_H */
