/*
 * theme.c - jgmenu's icon theme setter
 *
 * The icon theme is obtained in the following order or precedence:
 *   - xsettings
 *   - tint2rc
 *   - jgmenurc
 *   - set default ("Adwaita")
 *
 * The font name is first sought in jgmenurc and then xsettings.
 *
 * The reasons for this inconsistency is that it is anticipated that most
 * users will
 *   - change icon themes using gnome-settings, lxappearance or similar;
 *   - but will change font-settings in jgmenurc as this will be more specific
 *     to jgmenu (particularly the font size).
 */

#include "theme.h"
#include "xsettings-helper.h"
#include "t2conf.h"
#include "config.h"

/*
 * Search the following for 'gtk-icon-theme-name'
 *   - $XDG_CONFIG_HOME/gtk-3.0/settings.ini; or
 *   - $HOME/.config/gtk-3.0/settings.ini if $XDG_CONFIG_HOME is not set; or
 *   - /etc/gtk-3.0/settings.ini if the two above don't exist.
 */
static void get_gtk_theme(struct sbuf *theme)
{
	;
}

void theme_set(struct sbuf *theme)
{
	char *t = NULL;

	if (!config.ignore_xsettings && !t2conf_get_override_xsettings()) {
		if (xsettings_get(theme, "Net/IconThemeName") == 0) {
			fprintf(stderr, "info: set icon theme from xsettings\n");
			return;
		}
	}

	t2conf_get_icon_theme(&t);
	if (t) {
		sbuf_cpy(theme, t);
		fprintf(stderr, "info: set icon theme from tint2rc\n");
		return;
	}

	if (config.icon_theme) {
		sbuf_cpy(theme, config.icon_theme);
		fprintf(stderr, "info: set icon theme from jgmenurc\n");
		return;
	}

	get_gtk_theme(theme);
	if (theme->len) {
		fprintf(stderr, "info: set icon theme from gtk config file\n");
		return;
	}

	warn("set icon theme to 'Adwaita' because all else failed");
	sbuf_cpy(theme, "Adwaita");
}
