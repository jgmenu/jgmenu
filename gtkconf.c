/*
 * gtkconf.c
 *
 * Gets a variable from one of the following:
 *   - $XDG_CONFIG_HOME/gtk-3.0/settings.ini; or
 *   - $HOME/.config/gtk-3.0/settings.ini if $XDG_CONFIG_HOME is not set; or
 *   - /etc/gtk-3.0/settings.ini if the two above do not exist.
 */

#include <stdlib.h>

#include "theme.h"
#include "config.h"
#include "util.h"

#include "gtkconf.h"

static int parse_gtk_file(struct sbuf *settingsini, struct sbuf *buf,
			  const char *key)
{
	FILE *fp;
	char line[2048];

	fp = fopen(settingsini->buf, "r");
	if (!fp)
		return -1;
	while (fgets(line, sizeof(line), fp)) {
		char *option, *value, *p;

		p = strchr(line, '\n');
		if (p)
			*p = '\0';
		if (!parse_config_line(line, &option, &value))
			continue;
		if (!option)
			continue;
		if (!strcmp(option, key)) {
			sbuf_cpy(buf, value);
			break;
		}
	}
	fclose(fp);
	return 0;
}

void gtkconf_get(struct sbuf *buf, const char *key)
{
	struct sbuf settingsini;
	char *xdg_config_home;

	sbuf_init(&settingsini);

	xdg_config_home = getenv("XDG_CONFIG_HOME");
	if (xdg_config_home) {
		sbuf_cpy(&settingsini, xdg_config_home);
		sbuf_addstr(&settingsini, "/gtk-3.0/settings.ini");
		parse_gtk_file(&settingsini, buf, key);
		if (buf->len)
			goto cleanup;
	}

	sbuf_cpy(&settingsini, "~/.config/gtk-3.0/settings.ini");
	sbuf_expand_tilde(&settingsini);
	parse_gtk_file(&settingsini, buf, key);
	if (buf->len)
		goto cleanup;

	sbuf_cpy(&settingsini, "/etc/gtk-3.0/settings.ini");
	parse_gtk_file(&settingsini, buf, key);

cleanup:
	xfree(settingsini.buf);
}
