/*
 * xdgdirs.c
 *
 * Simply provides linked lists with "XDG" directories
 *
 * http://specifications.freedesktop.org/basedir-spec...
 *
 * The list will contain the directories listed below (in given order).
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "xdgdirs.h"
#include "list.h"
#include "util.h"
#include "argv-buf.h"
#include "banned.h"

static char *xdg_base_dirs[] = {
	"$XDG_DATA_HOME", "$HOME/.local/share", "$XDG_DATA_DIRS",
	"/usr/share", "/usr/local/share", "/opt/share", NULL
};

static char *xdg_config_dirs[] = {
	"$XDG_CONFIG_HOME", "$HOME/.config", "$XDG_CONFIG_DIRS", "/etc/xdg",
	NULL
};

static void get_dirs(struct list_head *dir_list, char **dirs)
{
	size_t i;
	int j;
	struct sbuf tmp;
	struct argv_buf argv_buf;

	sbuf_init(&tmp);
	argv_set_delim(&argv_buf, ':');
	for (i = 0; dirs[i]; i++) {
		sbuf_cpy(&tmp, dirs[i]);
		if (!strncmp(tmp.buf, "$", 1))
			sbuf_expand_env_var(&tmp);
		if (!tmp.len)
			continue;
		argv_init(&argv_buf);
		argv_strdup(&argv_buf, tmp.buf);
		argv_parse(&argv_buf);
		for (j = 0; j < argv_buf.argc; j++)
			sbuf_list_append(dir_list, argv_buf.argv[j]);
		xfree(argv_buf.buf);
	}
}

void xdgdirs_get_basedirs(struct list_head *dir_list)
{
	get_dirs(dir_list, xdg_base_dirs);
}

void xdgdirs_get_configdirs(struct list_head *dir_list)
{
	get_dirs(dir_list, xdg_config_dirs);
}

void xdgdirs_find_menu_file(struct sbuf *filename)
{
	LIST_HEAD(config_dirs);
	struct sbuf *tmp;
	struct stat sb;
	int i;
	static const char * const prefix[] = { "gnome-", "lxde-", "lxqt-", "kde-",
					       NULL };

	xdgdirs_get_configdirs(&config_dirs);
	sbuf_init(filename);
	list_for_each_entry(tmp, &config_dirs, list) {
		if (getenv("XDG_MENU_PREFIX")) {
			sbuf_cpy(filename, tmp->buf);
			sbuf_addstr(filename, "/menus/");
			sbuf_addstr(filename, getenv("XDG_MENU_PREFIX"));
			sbuf_addstr(filename, "applications.menu");
			if (!stat(filename->buf, &sb))
				goto found;
		} else {
			for (i = 0; prefix[i]; i++) {
				sbuf_cpy(filename, tmp->buf);
				sbuf_addstr(filename, "/menus/");
				sbuf_addstr(filename, prefix[i]);
				sbuf_addstr(filename, "applications.menu");
				if (!stat(filename->buf, &sb))
					goto found;
			}
		}
	}
	sbuf_cpy(filename, "");
found:
	sbuf_list_free(&config_dirs);
}
