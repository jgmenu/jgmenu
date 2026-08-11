#ifndef COMPAT_H
#define COMPAT_H

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define strlcpy gitstrlcpy
size_t gitstrlcpy(char *, const char *, size_t);

#endif /* COMPAT_H */
