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

void spawn(const char *arg, const char *working_dir);

/**
 * spawn_sync - execute and wait for child to finish
 * @command: array of command+arguments to be executed
 */
void spawn_sync(const char * const*command);

#endif /* SPAWN_H */
