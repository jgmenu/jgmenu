#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "socket.h"

/*
 * Add UID to path to support multi-user system
 * Allow for 32-bit UID (i.e. 10 digits)
 */
#define SOCKET_PATH_LEN (40)
static char socket_path[SOCKET_PATH_LEN];

static void socket_init(void)
{
	static int done;

	if (done)
		return;
	snprintf(socket_path, SOCKET_PATH_LEN, "/tmp/jgmenu_unix_socket_%d",
		 getuid());
	done = 1;
}

char *tint2_socket_path(void)
{
	socket_init();
	return socket_path;
}
