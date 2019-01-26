#ifndef ICON_H
#define ICON_H

#include <stdio.h>
#include <cairo.h>
#include <cairo-xlib.h>

extern void icon_init(void);
extern void icon_set_theme(const char *theme);
extern void icon_set_size(int size);
extern void icon_set_name(const char *name);
extern cairo_surface_t *load_cairo_icon(const char *path, int icon_size);
extern void icon_load(void);
extern cairo_surface_t *icon_get_surface(const char *name);
extern void icon_cleanup(void);

#endif /* ICON_H */
