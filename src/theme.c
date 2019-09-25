/* theme.c - a helper to find the icon theme */

#include "theme.h"
#include "xsettings-helper.h"
#include "t2conf.h"
#include "config.h"
#include "gtkconf.h"

void theme_set(struct sbuf *theme)
{
	char *t = NULL;
	char *fb = config.icon_theme_fallback;
	int i;

	if (!theme)
		die("theme_set() needs an initiated buffer");
	if (theme->len)
		die("theme_set() relies on *theme pointing to an empty buffer");

	if (config.icon_theme && config.icon_theme[0]) {
		sbuf_cpy(theme, config.icon_theme);
		info("got icon theme from jgmenurc");
		return;
	}

	/* Use fall back options */
	for (i = 0; fb[i]; i++) {
		switch (fb[i]) {
		case 'x':
			if (t2conf_get_override_xsettings())
				break;
			xsettings_get(theme, "Net/IconThemeName");
			if (theme->len) {
				info("got icon theme from xsettings");
				return;
			}
			break;

		case 't':
			t2conf_get_icon_theme(&t);
			if (t) {
				sbuf_cpy(theme, t);
				info("got icon theme from tint2rc");
				return;
			}
			break;

		case 'g':
			gtkconf_get(theme, "gtk-icon-theme-name");
			if (theme->len) {
				info("got icon theme from gtk config file");
				return;
			}
			break;
		}
	}

	warn("set icon theme to 'Adwaita' because all else failed");
	sbuf_cpy(theme, "Adwaita");
}
