/*
 * icon.c: Loads icons in background
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
#include <png.h>

#include "icon.h"
#include "icon-find.h"
#include "list.h"
#include "util.h"
#include "sbuf.h"
#include "xpm-loader.h"
#include "cache.h"
#include "config.h"
#include "icon.h"
#include "banned.h"

#define DEBUG_THEMES 0

struct icon {
	char *name;
	struct sbuf path;
	cairo_surface_t *surface;
	struct list_head list;
};

static struct list_head icon_cache;

static struct sbuf icon_theme;

void icon_init(void)
{
	INIT_LIST_HEAD(&icon_cache);
	sbuf_init(&icon_theme);
}

void icon_set_theme(const char *theme)
{
	if (!theme)
		die("icon theme has to be set before icon_set_theme()");
	sbuf_cpy(&icon_theme, theme);
	cache_set_icon_theme(theme);
}

void icon_set_size(int size)
{
	cache_set_icon_size(size);
}

#define PNG_BYTES_TO_CHECK (4)
static int ispng(const char *filename)
{
	unsigned char header[PNG_BYTES_TO_CHECK];
	FILE *fp;

	fp = fopen(filename, "rb");
	if (!fp)
		return 0;
	if (fread(header, 1, PNG_BYTES_TO_CHECK, fp) != PNG_BYTES_TO_CHECK) {
		fclose(fp);
		return 0;
	}
	if (png_sig_cmp(header, (png_size_t)0, PNG_BYTES_TO_CHECK)) {
		warn("file '%s' is not a recognised png file", filename);
		fclose(fp);
		return 0;
	}
	fclose(fp);
	return 1;
}

static cairo_surface_t *get_png_icon(const char *filename)
{
	cairo_surface_t *image = NULL;

	if (!ispng(filename))
		return NULL;
	image = cairo_image_surface_create_from_png(filename);
	if (cairo_surface_status(image)) {
		warn("error reading '%s'", filename);
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
	double scale, ratio;

	svg = rsvg_handle_new_from_file(filename, &err);
	if (err) {
		fprintf(stderr, "warning: problem loading svg %s-%s\n", filename, err->message);
		g_error_free(err);
		return NULL;
	}
	rsvg_handle_get_dimensions(svg, &dimensions);

	if (dimensions.width == dimensions.height) {
		surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
						     size, size);
		cr = cairo_create(surface);
		cairo_scale(cr, (double)size / dimensions.width,
			    (double)size / dimensions.height);
	} else if (dimensions.width > dimensions.height) {
		ratio = (double)dimensions.width / dimensions.height;
		scale = (double)size / dimensions.width;
		surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, size, size / ratio);
		cr = cairo_create(surface);
		cairo_scale(cr, scale, scale);
	} else {
		ratio = (double)dimensions.width / dimensions.height;
		scale = (double)size / dimensions.height;
		surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, size * ratio, size);
		cr = cairo_create(surface);
		cairo_scale(cr, scale, scale);
	}
	rsvg_handle_render_cairo(svg, cr);
	cairo_destroy(cr);
	g_object_unref(svg);

	return surface;
}

void icon_set_name(const char *name)
{
	struct icon *icon;

	/* Don't add if already exists in list */
	list_for_each_entry(icon, &icon_cache, list)
		if (!strcmp(name, icon->name))
			return;

	icon = xmalloc(sizeof(struct icon));

	icon->name = strdup(name);
	sbuf_init(&icon->path);
	icon->surface = NULL;
	list_add(&icon->list, &icon_cache);
}

cairo_surface_t *load_cairo_icon(const char *path, int icon_size)
{
	if (strstr(path, ".png"))
		return get_png_icon(path);
	else if (strstr(path, ".svg"))
		return get_svg_icon(path, icon_size);
	else if (strstr(path, ".xpm"))
		return get_xpm_icon(path);
	return NULL;
}

void icon_load(void)
{
	struct icon *icon;
	struct icon_path *path, *tmp_path;
	struct sbuf s;
	static int first_load = 1;
	struct list_head icon_paths;
	int nr_symlinks = 0;

	if (!icon_theme.len)
		die("icon theme has to be set before icon_load()");
	if (DEBUG_THEMES)
		fprintf(stderr, "%s:%d %s:\n", __FILE__, __LINE__, __FUNCTION__);
	if (first_load) {
		icon_find_init();
		icon_find_add_theme(icon_theme.buf);
		icon_find_add_theme("hicolor");
		if (DEBUG_THEMES)
			icon_find_print_themes();
		first_load = 0;
	}

	sbuf_init(&s);

	INIT_LIST_HEAD(&icon_paths);
	list_for_each_entry(icon, &icon_cache, list) {
		if (icon->name)
			sbuf_cpy(&icon->path, icon->name);
		/* Do not lookup icons with a NULL name or a full path. */
		if (!icon->name || icon->name[0] == '\0' ||
		    strchr(icon->name, '/'))
			continue;
		/* icon_load is run twice, so let's not duplicate effort */
		if (icon->surface)
			continue;
		/* Try to find icon symlink in jgmenu-cache, so save lookup */
		if (cache_strdup_path(icon->name, &icon->path))
			continue;
		path = xcalloc(1, sizeof(struct icon_path));
		sbuf_init(&path->name);
		sbuf_init(&path->path);
		path->icon = icon;
		sbuf_cpy(&path->name, icon->name);
		list_add(&path->list, &icon_paths);
	}
	icon_find_all(&icon_paths, config.icon_size);
	list_for_each_entry(path, &icon_paths, list) {
		icon = (struct icon *)path->icon;
		sbuf_cpy(&icon->path, path->path.buf);
		if (cache_create_symlink(icon->path.buf, icon->name) == 0) {
			nr_symlinks++;
		} else {
			warn("could not find icon '%s'", icon->name);
			if (cache_touch(icon->name) < 0)
				warn("error touching file '%s'", icon->name);
		}
	}
	if (nr_symlinks)
		fprintf(stderr, "info: created %d symlinks in ~/.cache/jgmenu/icons/\n", nr_symlinks);
	list_for_each_entry_safe(path, tmp_path, &icon_paths, list) {
		free(path->name.buf);
		free(path->path.buf);
		list_del(&path->list);
		free(path);
	}
	list_for_each_entry(icon, &icon_cache, list) {
		/* icon_load is run twice, so let's not duplicate effort */
		if (icon->surface)
			continue;
		icon->surface = load_cairo_icon(icon->path.buf,
						config.icon_size);
	}

	free(s.buf);
}

cairo_surface_t *icon_get_surface(const char *name)
{
	struct icon *icon;

	if (!name)
		return NULL;

	list_for_each_entry(icon, &icon_cache, list)
		if (!strcmp(icon->name, name))
			return icon->surface;

	return NULL;
}

void icon_cleanup(void)
{
	struct icon *icon, *tmp_icon;

	list_for_each_entry_safe(icon, tmp_icon, &icon_cache, list) {
		cairo_surface_destroy(icon->surface);
		xfree(icon->name);
		xfree(icon->path);
		list_del(&icon->list);
		xfree(icon);
	}
	icon_find_cleanup();
	cache_atexit_cleanup();
}
