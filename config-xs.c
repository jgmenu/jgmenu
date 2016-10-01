/*
 * config-xs.c - interfaces with xsettings-helper.c to get
 *               the icon theme and font name.
 *
 * The following config settings are respected
 *   - ignore_xsettings
 *   - ignore_icon_cache
 *
 * The icon theme is obtained in the following order or precedence:
 *   - xsettings
 *   - ~/.config/jgmenu/jgmenurc
 *   (- ~/.config/gtk3.0/settings.ini) TODO: Not implemented yet
 *   - set default ("Adwaita")
 *
 * The font name is first sought in jgmenurc and then xsettings.
 *
 * The reasons for the above inconsistency is that it is anticipated that
 * most users will
 *   - change icon themes using gnome-settings, lxappearance or similar;
 *   - but will change font-settings in jgmenurc as this will be more specific
 *     to jgmenu (particularly the font size).
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "config-xs.h"
#include "xsettings-helper.h"
#include "sbuf.h"

static int cache_exists(void)
{
	struct sbuf f;
	struct stat sb;
	int exists;

	sbuf_init(&f);
	sbuf_addstr(&f, getenv("HOME"));
	sbuf_addstr(&f, "/.local/share/icons/jgmenu-cache/index.theme");
	exists = stat(f.buf, &sb) == 0;
	if (!exists)
		fprintf(stderr, "warning: jgmenu icon-cache has not been created\n");
	return exists;
}

static void get_xsettings_icon_theme(char **theme)
{
	int ret;
	struct sbuf s;

	sbuf_init(&s);

	/* xsettings_get returns 0 for success */
	ret = xsettings_get(&s, "Net/IconThemeName");

	if (ret == 0)
		*theme = strdup(s.buf);

	free(s.buf);
}

void config_xs_get_theme(char **theme, int ignore_xsettings, int ignore_cache)
{
	/*
	 * At this point, *theme might be already have been strdup'd
	 * from jgmenurc.
	 */
	if (!ignore_cache && cache_exists()) {
		*theme = strdup("jgmenu-cache");
		return;
	}

	/* **theme will not be touched if an xsettings daemon is not running */
	if (!ignore_xsettings)
		get_xsettings_icon_theme(theme);

	/*
	 * If we have not yet "returned", let's use the jgmenurc icon_theme
	 * if one was specified
	 */
	if (*theme)
		return;

	/* TODO: Get ~/.config/gtk-3.0/settings.ini icon_theme */

	/*
	 * If *theme is still NULL here, we will leave it.
	 * The icon theme will be set to "default" in icon.c if a NULL pointer
	 * is passed to it.
	 */
}

void config_xs_get_font(char **font)
{
	int ret;
	struct sbuf s;

	sbuf_init(&s);
	ret = xsettings_get(&s, "Gtk/FontName");
	if (ret == 0)
		*font = strdup(s.buf);
	free(s.buf);
}
