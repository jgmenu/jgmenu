#include "unix_sockets.h"
#include "socket.h"
#include "util.h"

int main(int argc, char **argv)
{
	int sfd;
	ssize_t num_read;
	char buf[SOCKET_BUF_SIZE];

	sfd = unix_connect(tint2_socket_path(), SOCK_STREAM);
	if (sfd == -1)
		die("error connecting to tint2 UNIX socket");
	while ((num_read = read(STDIN_FILENO, buf, SOCKET_BUF_SIZE)) > 0)
		if (write(sfd, buf, num_read) != num_read)
			die("partial/failed write to tint2 UNIX socket");
	if (num_read == -1)
		die("read");
	exit(0);
}
