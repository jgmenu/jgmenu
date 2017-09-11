/*
 * xdgdirs.c
 *
 * Simply provides linked lists with "XDG" directories
 *
 * http://specifications.freedesktop.org/basedir-spec...
 *
 * The list will contain the directories listed below (in given order).
 * If the $XDG* variables are set, the remainder will not be appended to
 * the list.
 *
 * $XDG_DATA_HOME
 * $HOME/.local/share
 * $XDG_DATA_DIRS
 * /usr/local/share
 * /usr/share
 * /opt/share
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "xdgdirs.h"
#include "sbuf.h"
#include "list.h"
#include "util.h"

static char *xdg_base_dirs[] = {
	"$XDG_DATA_HOME", "$HOME/.local/share", "$XDG_DATA_DIRS",
	"/usr/share", "/usr/local/share", "/opt/share"
};

static char *xdg_config_dirs[] = {
	"$XDG_CONFIG_HOME", "$HOME/.config", "$XDG_CONFIG_DIRS", "/etc/xdg"
};

#define COUNT_OF(x) (sizeof(x) / sizeof(x)[0])

static void expand_env_vars(struct sbuf *s)
{
	char *p;
	struct sbuf env_name;

	sbuf_init(&env_name);
	sbuf_cpy(&env_name, s->buf);
	p = strchr(env_name.buf, '/');
	if (p)
		*p = '\0';
	p = getenv(env_name.buf + 1);
	if (!p) {
		sbuf_cpy(s, "");
		return;
	}
	sbuf_shift_left(s, strlen(env_name.buf));
	sbuf_prepend(s, p);
	free(env_name.buf);
}

void xdgdirs_get_basedirs(struct list_head *dir_list)
{
	size_t i;
	struct sbuf tmp;

	sbuf_init(&tmp);
	for (i = 0; i < COUNT_OF(xdg_base_dirs); i++) {
		sbuf_cpy(&tmp, xdg_base_dirs[i]);
		if (!strncmp(tmp.buf, "$", 1))
			expand_env_vars(&tmp);
		if (tmp.len)
			sbuf_list_append(dir_list, tmp.buf);
	}
}

void xdgdirs_get_configdirs(struct list_head *dir_list)
{
	size_t i;
	struct sbuf tmp;

	sbuf_init(&tmp);
	for (i = 0; i < COUNT_OF(xdg_config_dirs); i++) {
		sbuf_cpy(&tmp, xdg_config_dirs[i]);
		if (!strncmp(tmp.buf, "$", 1))
			expand_env_vars(&tmp);
		if (tmp.len)
			sbuf_list_append(dir_list, tmp.buf);
	}
}
