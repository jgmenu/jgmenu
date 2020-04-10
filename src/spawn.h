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

/**
 * spawn_async - execute in shell wrapper; do not wait for child to finish
 * @command: command to be executed
 * @working_dir: working directory
 */
void spawn_async(const char *arg, const char *working_dir);

/**
 * spawn_sync - execute in shell wrapper; wait for child to finish
 * @command: command to be executed
 */
void spawn_sync(const char * const*command);

/**
 * spawn_async_no_shell - execute asyncronously
 * @command: cmd to be executed
 * @working_dir: working directory
 */
void spawn_async_no_shell(char const *cmd, char const *workdir);

/**
 * spawn_command_line_sync - execute syncronously
 * @command: cmd to be executed
 */
void spawn_command_line_sync(const char *command);

#endif /* SPAWN_H */
