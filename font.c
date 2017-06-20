/* font.c - a helper to find 'the' font */

#include "font.h"
#include "xsettings-helper.h"
#include "t2conf.h"
#include "config.h"
#include "gtkconf.h"
#include "sbuf.h"
#include "util.h"

static struct sbuf font;
static int font_has_been_set;

char *font_get(void)
{
	if (!font_has_been_set)
		die("font_get() requires font to have been set");
	return font.buf;
}

void font_set(const char *src_font)
{
	char *t = NULL;
	int i;

	if (font_has_been_set)
		warn("font_set() is only meant to be run once");
	font_has_been_set = 1;
	if (!src_font)
		die("src_font needs to be set");

	sbuf_init(&font);
	for (i = 0; src_font[i]; i++)
		switch (src_font[i]) {
		case 'x':
			if (t2conf_get_override_xsettings())
				break;
			if (xsettings_get(&font, "Net/FontName") == 0) {
				info("got font from xsettings");
				return;
			}
			break;

		case 't':
			t2conf_get_font(&t);
			if (t) {
				sbuf_cpy(&font, t);
				info("got font from tint2rc");
				return;
			}
			break;

		case 'g':
			gtkconf_get(&font, "gtk-font-name");
			if (font.len) {
				info("got font from gtk config file");
				return;
			}
			break;
		case 'j':
			if (config.font) {
				sbuf_cpy(&font, config.font);
				info("got font from jgmenurc");
				return;
			}
			break;
	}

	warn("set font to 'Sans 11' because all else failed");
	sbuf_cpy(&font, "Sans 11");
}

void font_cleanup(void)
{
	xfree(font.buf);
}
