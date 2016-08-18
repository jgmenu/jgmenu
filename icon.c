/*
 * icon-cache.c: Loads icons in background
 *
 * This library is designed to
 *	- ensure the core menu (without icons) runs fast
 *	- avoid the use of mutex-locks
 *
 * It requires the functions to be called in these distinct phases:
 *	- populate the cache early with icon names ("set_name");
 *	- using a separate thread, load the icons into cache ("load");
 *	- when the thread is complete, obtain pointers to the cairo surfaces
 *	  (using the "get_surface" functions).
 */

#include <librsvg/rsvg.h>

/* for xpm icons */
#include <gtk/gtk.h>

#include "icon.h"
#include "icon-find.h"
#include "list.h"
#include "util.h"
#include "sbuf.h"

#define DEBUG_THEMES 0


struct Icon {
	char *name;
	cairo_surface_t *surface;
	struct list_head list;
};

struct list_head icon_cache;

struct String icon_theme;
int icon_size;

void icon_init(void)
{
	INIT_LIST_HEAD(&icon_cache);
	sbuf_init(&icon_theme);
	sbuf_cpy(&icon_theme, "default");
	icon_size = 22;
}

void icon_set_theme(const char *theme)
{
	if (theme)
		sbuf_cpy(&icon_theme, theme);
}

void icon_set_size(int size)
{
	icon_size = size;
}

static cairo_surface_t *get_png_icon(const char *filename)
{
	cairo_surface_t *image = NULL;

	image = cairo_image_surface_create_from_png(filename);
	if (cairo_surface_status(image)) {
		fprintf(stderr, "warning: cannot find icon %s\n", filename);
		cairo_surface_destroy(image);
		return NULL;
	}

	return image;
}

static cairo_surface_t *get_svg_icon(const char *filename, int size)
{
	cairo_surface_t *surface;
	cairo_t *cr;
	RsvgHandle *svg;
	RsvgDimensionData  dimensions;
	GError *err = NULL;

	svg = rsvg_handle_new_from_file(filename, &err);
	if (err) {
		fprintf(stderr, "warning: problem loading svg %s-%s\n", filename, err->message);
		g_error_free(err);
		return NULL;
	}
	rsvg_handle_get_dimensions(svg, &dimensions);
	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, size, size);
	cr = cairo_create(surface);
	cairo_scale(cr, (double) size / dimensions.width, (double) size / dimensions.width);
	rsvg_handle_render_cairo(svg, cr);
	cairo_destroy(cr);
	g_object_unref(svg);

	return surface;
}

static cairo_surface_t *get_xpm_icon(const char *filename)
{
	GdkPixbuf *pixbuf;
	gint width;
	gint height;
	cairo_format_t format;
	cairo_surface_t *surface;
	cairo_t *cr;

	pixbuf = gdk_pixbuf_new_from_file(filename, NULL);
	if (!pixbuf) {
		fprintf(stderr, "warning: error loading icon %s\n", filename);
		return NULL;
	}

	format = gdk_pixbuf_get_has_alpha(pixbuf) ? CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_RGB24;
	width = gdk_pixbuf_get_width(pixbuf);
	height = gdk_pixbuf_get_height(pixbuf);
	surface = cairo_image_surface_create(format, width, height);
	if (!pixbuf) {
		fprintf(stderr, "warning: error loading icon %s\n", filename);
		return NULL;
	}

	cr = cairo_create(surface);
	gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
	cairo_paint(cr);
	cairo_destroy(cr);
	return surface;
}

void icon_set_name(const char *name)
{
	struct Icon *icon;

	/* Don't add if already exists in list */
	list_for_each_entry(icon, &icon_cache, list)
		if (!strcmp(name, icon->name))
			return;

	icon = xmalloc(sizeof(struct Icon));

	icon->name = strdup(name);
	icon->surface = NULL;
	list_add(&(icon->list), &icon_cache);
}

void icon_load(void)
{
	struct Icon *icon;
	struct String s;
	static int first_load = 1;

	if (first_load) {
		icon_find_init();
		icon_find_add_theme(icon_theme.buf);
		icon_find_add_theme("hicolor");
		if (DEBUG_THEMES)
			icon_find_print_themes();
		first_load = 0;
	}

	sbuf_init(&s);

	list_for_each_entry(icon, &icon_cache, list) {
		if (!icon->name)
			die("no icon name set\n");

		sbuf_cpy(&s, icon->name);
		icon_find(&s, icon_size);

		if (strstr(s.buf, ".png"))
			icon->surface = get_png_icon(s.buf);
		else if (strstr(s.buf, ".svg"))
			icon->surface = get_svg_icon(s.buf, icon_size);
		else if (strstr(s.buf, ".xpm"))
			icon->surface = get_xpm_icon(s.buf);
	}

	free(s.buf);
}

cairo_surface_t *icon_get_surface(const char *name)
{
	struct Icon *icon;

	if (!name)
		return NULL;

	list_for_each_entry(icon, &icon_cache, list)
		if (!strcmp(icon->name, name))
			return icon->surface;

	return NULL;
}
