/*
 * Copyright (C) Michael Kerrisk, 2016.
 *
 * This program is free software. You may use, modify, and redistribute it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 or (at your option)
 * any later version. This program is distributed without any warranty.
 * See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.
 */

#ifndef UNIX_SOCKETS_H
#define UNIX_SOCKETS_H

#include <sys/socket.h>
#include <sys/un.h>

int unix_build_address(const char *path, struct sockaddr_un *addr);
int unix_connect(const char *path, int type);
int unix_listen(const char *path, int backlog);

#endif
