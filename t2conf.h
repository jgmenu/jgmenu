#ifndef T2CONF_H
#define T2CONF_H

#include <stdio.h>
#include <stdlib.h>

extern void t2conf_atexit(void);
extern void t2conf_parse(const char *filename, int screen_width,
			 int screen_height);
extern int t2conf_is_horizontal_panel(void);
extern void t2conf_get_font(char **f);
extern int t2conf_get_override_xsettings(void);
extern void t2conf_get_icon_theme(char **t);

#endif /* T2CONF_H */
