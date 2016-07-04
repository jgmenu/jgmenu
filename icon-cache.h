#ifndef ICON_CACHE_H
#define ICON_CACHE_H

#include <stdio.h>
#include <cairo.h>
#include <cairo-xlib.h>

extern void icon_cache_init(void);
extern void icon_cache_set_theme(const char *theme);
extern void icon_cache_set_size(int size);
extern void icon_cache_set_name(const char *name);
extern void icon_cache_load(void);
extern cairo_surface_t *icon_cache_get_surface(const char *name);

#endif /* ICON_CACHE_H */
