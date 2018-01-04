#include <string.h>
#include <fcntl.h>

#include "util.h"
#include "config.h"
#include "sbuf.h"
#include "t2env.h"
#include "unix_sockets.h"
#include "socket.h"
#include "argv-buf.h"

static int sfd;

static void process_line(char *line)
{
	char *option, *value;

	if (!parse_config_line(line, &option, &value))
		return;
	if (!strcmp(option, "TINT2_BUTTON_ALIGNED_X1"))
		setenv("TINT2_BUTTON_ALIGNED_X1", value, 1);
	else if (!strcmp(option, "TINT2_BUTTON_ALIGNED_X2"))
		setenv("TINT2_BUTTON_ALIGNED_X2", value, 1);
	else if (!strcmp(option, "TINT2_BUTTON_ALIGNED_Y1"))
		setenv("TINT2_BUTTON_ALIGNED_Y1", value, 1);
	else if (!strcmp(option, "TINT2_BUTTON_ALIGNED_Y2"))
		setenv("TINT2_BUTTON_ALIGNED_Y2", value, 1);
	else if (!strcmp(option, "TINT2_BUTTON_PANEL_X1"))
		setenv("TINT2_BUTTON_PANEL_X1", value, 1);
	else if (!strcmp(option, "TINT2_BUTTON_PANEL_X2"))
		setenv("TINT2_BUTTON_PANEL_X2", value, 1);
	else if (!strcmp(option, "TINT2_BUTTON_PANEL_Y1"))
		setenv("TINT2_BUTTON_PANEL_Y1", value, 1);
	else if (!strcmp(option, "TINT2_BUTTON_PANEL_Y2"))
		setenv("TINT2_BUTTON_PANEL_Y2", value, 1);
}

static void process_buf(const char *buf)
{
	int i;
	struct argv_buf a;

	argv_set_delim(&a, '\n');
	argv_init(&a);
	argv_strdup(&a, buf);
	argv_parse(&a);
	for (i = 0; i < a.argc; i++)
		process_line(a.argv[i]);
}

void tint2env_read_socket(void)
{
	int client_fd;
	ssize_t num_read;
	char buf[SOCKET_BUF_SIZE];

	/* sfd is non-blocking */
	client_fd = accept(sfd, NULL, NULL);
	if (client_fd == -1) {
		fprintf(stderr, "info: tint2 button was not used\n");
		return;
	}
	while ((num_read = read(client_fd, buf, SOCKET_BUF_SIZE)) > 0) {
		buf[num_read] = '\0';
		process_buf(buf);
	}
	if (num_read == -1)
		die("read");
	if (close(client_fd) == -1)
		warn("close");
}

static void make_sfd_nonblocking(void)
{
	int flags;

	flags = fcntl(sfd, F_GETFL);
	if (flags == -1)
		die("error getting socket flags");
	flags |= O_NONBLOCK;
	if (fcntl(sfd, F_SETFL, flags) == -1)
		die("error setting socket flags");
}

static void socketfile_unlink(void)
{
	unlink(tint2_socket_path());
}

void tint2env_init_socket(void)
{
	socketfile_unlink();
	sfd = unix_listen(tint2_socket_path(), 5);
	if (sfd == -1)
		die("unix_listen");
	make_sfd_nonblocking();
	atexit(socketfile_unlink);
}
