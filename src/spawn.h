#ifndef SPAWN_H
#define SPAWN_H

#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>

extern void spawn(const char *arg, const char *working_dir);

#endif /* SPAWN_H */
