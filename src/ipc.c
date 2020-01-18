#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

#include "util.h"
#include "config.h"
#include "sbuf.h"
#include "ipc.h"
#include "unix_sockets.h"
#include "socket.h"
#include "argv-buf.h"
#include "geometry.h"
#include "banned.h"

struct rect {
	int x1, x2, y1, y2;
};

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

void ipc_read_socket(void)
{
	int client_fd;
	ssize_t num_read;
	char buf[SOCKET_BUF_SIZE];

	/* sfd is non-blocking */
	client_fd = accept(sfd, NULL, NULL);
	/* only proceed if tint2 button/launcher was used */
	if (client_fd == -1)
		return;
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

void ipc_init_socket(void)
{
	socketfile_unlink();
	sfd = unix_listen(tint2_socket_path(), 5);
	if (sfd == -1)
		die("unix_listen");
	make_sfd_nonblocking();
	atexit(socketfile_unlink);
}

static bool ishorizontal(struct rect rect)
{
	return (rect.x2 - rect.x1) > (rect.y2 - rect.y1) ? true : false;
}

static int _getenv(int *var, const char *key)
{
	char *s;

	s = getenv(key);
	if (!s) {
		warn("environment variable (%s) not set", key);
		return -1;
	}
	xatoi(var, s, XATOI_NONNEG, key);
	return 0;
}

static void align_to_horizontal_panel(struct rect panel, struct rect button)
{
	geo_update_monitor_coords();
	if (button.x1 >= geo_get_screen_x0() + geo_get_screen_width() ||
	    button.x1 < geo_get_screen_x0()) {
		warn("pointer outside IPC varable range");
		return;
	}
	if (config.verbosity == 4)
		info("align to horizontal panel");
	if (button.x1 < panel.x2 - geo_get_menu_width()) {
		geo_set_menu_margin_x(button.x1);
		geo_set_menu_halign(LEFT);
	} else {
		geo_set_menu_margin_x(geo_get_screen_width() - panel.x2);
		geo_set_menu_halign(RIGHT);
	}
	if ((geo_get_screen_y0() + geo_get_screen_height() / 2) >= button.y1) {
		geo_set_menu_valign(TOP);
		geo_set_menu_margin_y(panel.y2);
	} else {
		geo_set_menu_valign(BOTTOM);
		geo_set_menu_margin_y(geo_get_screen_height() - panel.y1);
	}
}

static void align_to_vertical_panel(struct rect panel, struct rect button)
{
	geo_update_monitor_coords();
	if (button.y1 >= geo_get_screen_y0() + geo_get_screen_height() ||
	    button.y1 < geo_get_screen_y0()) {
		warn("pointer outside IPC varable range");
		return;
	}
	if (config.verbosity == 4)
		info("align to vertical panel");
	if (button.y1 < panel.y2 - geo_get_menu_height()) {
		geo_set_menu_margin_y(button.y1);
		geo_set_menu_valign(TOP);
	} else {
		geo_set_menu_margin_y(geo_get_screen_height() - button.y2);
		geo_set_menu_valign(BOTTOM);
	}
	if ((geo_get_menu_x0() + geo_get_screen_width() / 2) >= button.x1) {
		geo_set_menu_margin_x(panel.x1);
		geo_set_menu_halign(LEFT);
	} else {
		geo_set_menu_margin_x(geo_get_screen_width() - panel.x1);
		geo_set_menu_halign(RIGHT);
	}
}

/*
 * With a horizontal panel, button.y1 == button.y2 (because they're aligned
 * to the edge of the panel.
 */
void ipc_align_based_on_env_vars(void)
{
	struct rect button, panel;

	if (_getenv(&button.x1, "TINT2_BUTTON_ALIGNED_X1") < 0 ||
	    _getenv(&button.x2, "TINT2_BUTTON_ALIGNED_X2") < 0 ||
	    _getenv(&button.y1, "TINT2_BUTTON_ALIGNED_Y1") < 0 ||
	    _getenv(&button.y2, "TINT2_BUTTON_ALIGNED_Y2") < 0 ||
	    _getenv(&panel.x1, "TINT2_BUTTON_PANEL_X1") < 0 ||
	    _getenv(&panel.x2, "TINT2_BUTTON_PANEL_X2") < 0 ||
	    _getenv(&panel.y1, "TINT2_BUTTON_PANEL_Y1") < 0 ||
	    _getenv(&panel.y2, "TINT2_BUTTON_PANEL_Y2") < 0) {
		warn("cannot align based on environment variables");
		return;
	}

	if (ishorizontal(panel))
		align_to_horizontal_panel(panel, button);
	else
		align_to_vertical_panel(panel, button);
}
