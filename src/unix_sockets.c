/*
 * Copyright (C) Michael Kerrisk, 2016.
 *
 * This program is free software. You may use, modify, and redistribute it
 * under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 or (at your option)
 * any later version. This program is distributed without any warranty.
 * See the files COPYING.lgpl-v3 and COPYING.gpl-v3 for details.
 */

#include <unistd.h>

#include "unix_sockets.h"
#include "util.h"
#include "compat.h"
#include "banned.h"

/*
 * Build a UNIX domain socket address structure for 'path', returning
 * it in 'addr'. Returns -1 on success, or 0 on error.
 */
int unix_build_address(const char *path, struct sockaddr_un *addr)
{
	if (!addr || !path || strlen(path) >= sizeof(addr->sun_path) - 1) {
		errno = EINVAL;
		return -1;
	}
	memset(addr, 0, sizeof(struct sockaddr_un));
	addr->sun_family = AF_UNIX;
	if (strlen(path) < sizeof(addr->sun_path)) {
		strlcpy(addr->sun_path, path, sizeof(addr->sun_path));
		return 0;
	}
	errno = ENAMETOOLONG;
	return -1;
}

/*
 * Create a UNIX domain socket of type 'type' and connect it
 * to the remote address specified by the 'path'.
 * Return the socket descriptor on success, or -1 on error
 */
int unix_connect(const char *path, int type)
{
	int sd, saved_errno;
	struct sockaddr_un addr;

	if (unix_build_address(path, &addr) == -1)
		return -1;
	sd = socket(AF_UNIX, type, 0);
	if (sd == -1)
		return -1;
	if (connect(sd, (struct sockaddr *)&addr,
		    sizeof(struct sockaddr_un)) == -1) {
		saved_errno = errno;
		close(sd);
		errno = saved_errno;
		return -1;
	}
	return sd;
}

/*
 * Create a UNIX domain socket and bind it to 'path'. If 'do_listen'
 * is true, then call listen() with specified 'backlog'.
 * Return the socket descriptor on success, or -1 on error.
 */
static int unix_passive_socket(const char *path, int type, int do_listen,
			       int backlog)
{
	int sd, saved_errno;
	struct sockaddr_un addr;

	if (unix_build_address(path, &addr) == -1)
		return -1;
	sd = socket(AF_UNIX, type, 0);
	if (sd == -1)
		return -1;
	if (bind(sd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
		saved_errno = errno;
		close(sd);
		errno = saved_errno;
		return -1;
	}
	if (do_listen) {
		if (listen(sd, backlog) == -1) {
			saved_errno = errno;
			close(sd);
			errno = saved_errno;
			return -1;
		}
	}
	return sd;
}

/*
 * Create stream socket, bound to 'path'. Make the socket a listening
 * socket, with the specified 'backlog'. Return socket descriptor on
 * success, or -1 on error.
 */
int unix_listen(const char *path, int backlog)
{
	return unix_passive_socket(path, SOCK_STREAM, 1, backlog);
}
