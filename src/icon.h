#ifndef ICON_H
#define ICON_H

#include <stdio.h>
#include <cairo.h>
#include <cairo-xlib.h>

void icon_init(void);
void icon_set_theme(const char *theme);
void icon_set_size(int size);
void icon_set_name(const char *name);
cairo_surface_t *load_cairo_icon(const char *path, int icon_size);
void icon_load(void);
cairo_surface_t *icon_get_surface(const char *name);
void icon_cleanup(void);

#endif /* ICON_H */
