#include <unistd.h>
#include <stdlib.h>

#include "unix_sockets.h"
#include "socket.h"
#include "util.h"
#include "banned.h"

int main(int argc, char **argv)
{
	int sfd;
	ssize_t num_read;
	char buf[SOCKET_BUF_SIZE];
	int verbosity = 0;
	char *jgmenu_verbosity;

	jgmenu_verbosity = getenv("JGMENU_VERBOSITY");
	if (jgmenu_verbosity)
		verbosity = atoi(jgmenu_verbosity);

	sfd = unix_connect(tint2_socket_path(), SOCK_STREAM);
	if (sfd == -1)
		/* error connecting to tint2 UNIX socket */
		exit(1);
	if (verbosity == 4)
		fprintf(stderr, "[jgmenu-socket] sent:\n");
	while ((num_read = read(STDIN_FILENO, buf, SOCKET_BUF_SIZE)) > 0) {
		if (write(sfd, buf, num_read) != num_read)
			die("partial/failed write to tint2 UNIX socket");
		if (verbosity == 4)
			fprintf(stderr, "%s", buf);
	}
	if (num_read == -1)
		die("read");
	exit(0);
}
