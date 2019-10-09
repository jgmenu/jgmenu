#ifndef T2CONF_H
#define T2CONF_H

#include <stdio.h>
#include <stdlib.h>

void t2conf_atexit(void);
void t2conf_parse(const char *filename, int screen_width, int screen_height);
void t2conf_get_font(char **f);
int t2conf_get_override_xsettings(void);
void t2conf_get_icon_theme(char **t);

#endif /* T2CONF_H */
