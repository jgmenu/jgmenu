/* x11-ui.c
 *
 * x11-ui.c provides a simple X11 interface.
 * It uses cairo for shapes and pango for text.
 *
 * It has been inspired by:
 *  - dmenu 4.5 (http://tools.suckless.org/dmenu/)
 *  - tint2 (https://gitlab.com/o9000/tint2)
 *  - dzen2 (https://github.com/robm/dzen)
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif

#include <unistd.h>		/* for usleep */

#include "x11-ui.h"
#include "util.h"

/* INTERSECT is required by Xinerama.  MAX and MIN are defined in glib */
#define INTERSECT(x, y, w, h, r)  (MAX(0, MIN((x)+(w), (r).x_org+(r).width)  - \
				   MAX((x), (r).x_org)) * \
				   MAX(0, MIN((y)+(h), (r).y_org+(r).height) - \
				   MAX((y), (r).y_org)))

void ui_clear_canvas(void)
{
	cairo_save(ui->c);
	cairo_set_source_rgba(ui->c, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator(ui->c, CAIRO_OPERATOR_SOURCE);
	cairo_paint(ui->c);
	cairo_restore(ui->c);
}

void grabkeyboard(void)
{
	/* The "waiting" trick has been copied from dmenu-4.5 */
	int i;

	for (i = 0; i < 1000; i++) {
		if (XGrabKeyboard(ui->dpy, DefaultRootWindow(ui->dpy), True,
				  GrabModeAsync, GrabModeAsync,
				  CurrentTime) == GrabSuccess)
			return;
		usleep(1000);
	}
	die("cannot grab keyboard");
}


/*
 * grabpointer() is needed in order to detect when pointer is clicked
 * outside menu
 */
void grabpointer(void)
{
	XGrabPointer(ui->dpy, DefaultRootWindow(ui->dpy),
		     False,	/* False gives (x,y) wrt root window. */
		     ButtonPressMask | ButtonReleaseMask |
		     PointerMotionMask | FocusChangeMask |
		     EnterWindowMask | LeaveWindowMask,
		     GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
}



void ui_init_cairo(int canvas_width, int canvas_height, const char *font)
{
	ui->cs = cairo_xlib_surface_create(ui->dpy, ui->canvas, ui->vinfo.visual, canvas_width, canvas_height);
	ui->c = cairo_create(ui->cs);

	ui->pangolayout = pango_cairo_create_layout(ui->c);
	ui->pangofont = pango_font_description_from_string(font);

	ui->font_height_actual = ui_get_text_height(font);
}

void ui_init(void)
{
	/*
	 * In order to enable the creation of a transparent window,
	 * the depth needs to be set to 32.
	 * The following window attributes are also needed:
	 *   - background_pixel = 0;
	 *   - border_pixel = 0;
	 *   - colormap
	 */
	ui = xcalloc(1, sizeof(*ui));

	ui->dpy = XOpenDisplay(NULL);
	if (!ui->dpy)
		die("cannot open display");

	XMatchVisualInfo(ui->dpy, DefaultScreen(ui->dpy), 32, TrueColor, &(ui->vinfo));

	ui->swa.override_redirect = True;
	ui->swa.event_mask = ExposureMask | KeyPressMask | VisibilityChangeMask | ButtonPressMask;
	ui->swa.colormap = XCreateColormap(ui->dpy, DefaultRootWindow(ui->dpy), ui->vinfo.visual, AllocNone);
	ui->swa.background_pixel = 0;
	ui->swa.border_pixel = 0;
	ui->swa.event_mask = StructureNotifyMask;

	ui->screen = DefaultScreen(ui->dpy);
	ui->root = RootWindow(ui->dpy, ui->screen);
}

/*
 * The Xinerama code below was copied from dmenu's xft patch
 * (http://tools.suckless.org/dmenu/patches/xft)
 */
void ui_get_screen_res(int *x0, int *y0, int *width, int *height)
{
#ifdef XINERAMA
	int n;
	XineramaScreenInfo *info;

	info = XineramaQueryScreens(ui->dpy, &n);
	if (info) {
		int a, j, di, i = 0, area = 0;
		unsigned int du;
		Window w, pw, dw, *dws;
		XWindowAttributes wa;

		XGetInputFocus(ui->dpy, &w, &di);
		if (w != ui->root && w != PointerRoot && w != None) {
			/* find top-level window containing current input focus */
			do {
				pw = w;
				if (XQueryTree(ui->dpy, pw, &dw, &w, &dws, &du) && dws)
					XFree(dws);
			} while (w != ui->root && w != pw);
			/* find xinerama screen with which the window intersects most */
			if (XGetWindowAttributes(ui->dpy, pw, &wa))
				for (j = 0; j < n; j++) {
					a = INTERSECT(wa.x, wa.y, wa.width, wa.height, info[j]);
					if (a > area) {
						area = a;
						i = j;
					}
				}
		}

		/*
		 * No focused window is on screen, so use pointer location instead
		 */
/*
 *		if(!area && XQueryPointer(ui->dpy, root, &dw, &dw, x, y, &di, &di, &du))
 *			for(i = 0; i < n; i++)
 *				if(INTERSECT(*x, *y, 1, 1, info[i]))
 *					break;
 */

/* SET MENU DIMENSIONS */

		*x0 = info[i].x_org;
		*y0 = info[i].y_org;
		*width = info[i].width;
		*height = info[i].height;

		XFree(info);
	} else
#endif
	{
		*x0 = 0;
		*y0 = 0;
		*width = DisplayWidth(ui->dpy, ui->screen);
		*height = DisplayHeight(ui->dpy, ui->screen);
	}
}



/*
 * max_height is that associated with the longest submenu
 */
void ui_init_canvas(int max_width, int max_height)
{
	if (ui->canvas)
		XFreePixmap(ui->dpy, ui->canvas);
	ui->canvas = XCreatePixmap(ui->dpy, ui->root, max_width, max_height, 32);
}

void ui_create_window(int x, int y, int w, int h)
{
	XIM xim;

	ui->win = XCreateWindow(ui->dpy, ui->root, x, y, w, h, 1,
				ui->vinfo.depth, CopyFromParent,
				ui->vinfo.visual,
				CWOverrideRedirect | CWColormap | CWBackPixel | CWEventMask | CWBorderPixel, &ui->swa);

	xim = XOpenIM(ui->dpy, NULL, NULL, NULL);
	ui->xic = XCreateIC(xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
			    XNClientWindow, ui->win, XNFocusWindow, ui->win, NULL);
	XMapRaised(ui->dpy, ui->win);

	ui->gc = XCreateGC(ui->dpy, ui->win, 0, NULL);

	/* XDefineCursor required to prevent blindly inheriting cursor from parent
	 * (e.g. hour-glass pointer set by tint2)
	 * Check this URL for cursor styles:
	 * http://tronche.com/gui/x/xlib/appendix/b/
	 */
	XDefineCursor(ui->dpy, ui->win, XCreateFontCursor(ui->dpy, 68));
}

void ui_draw_rectangle_rounded_at_top(double x, double y, double w, double h,
				      double radius, double line_width, int fill, double *rgba)
{
	double deg = 0.017453292519943295; /* 2 x 3.1415927 / 360.0 */

	cairo_new_sub_path(ui->c);
	cairo_arc(ui->c, x + w - radius, y + radius, radius, -90 * deg, 0 * deg);  /* NE */
	cairo_arc(ui->c, x + w, y + h, 0, 0 * deg, 90 * deg);			   /* SE */
	cairo_arc(ui->c, x, y + h, 0, 90 * deg, 180 * deg);			   /* SW */
	cairo_arc(ui->c, x + radius, y + radius, radius, 180 * deg, 270 * deg);    /* NE */
	cairo_close_path(ui->c);
	cairo_set_source_rgba(ui->c, rgba[0], rgba[1], rgba[2], rgba[3]);
	if (fill) {
		cairo_set_line_width(ui->c, 0.0);
		cairo_fill_preserve(ui->c);
	} else {
		cairo_set_line_width(ui->c, line_width);
	}
	cairo_stroke(ui->c);
}

void ui_draw_rectangle(double x, double y, double w, double h, double radius, double line_width, int fill, double *rgba)
{
	x += line_width / 2;
	y += line_width / 2;
	w -= line_width;
	h -= line_width;

	cairo_set_line_width(ui->c, 0.0);
	if (radius > 0) {
		double deg = 0.017453292519943295; /* 2 x 3.1415927 / 360.0 */

		cairo_new_sub_path(ui->c);
		cairo_arc(ui->c, x + w - radius, y + radius, radius, -90 * deg, 0 * deg);
		cairo_arc(ui->c, x + w - radius, y + h - radius, radius, 0 * deg, 90 * deg);
		cairo_arc(ui->c, x + radius, y + h - radius, radius, 90 * deg, 180 * deg);
		cairo_arc(ui->c, x + radius, y + radius, radius, 180 * deg, 270 * deg);
		cairo_close_path(ui->c);
		cairo_set_source_rgba(ui->c, rgba[0], rgba[1], rgba[2], rgba[3]);
		if (fill) {
			cairo_set_line_width(ui->c, 0.0);
			cairo_fill_preserve(ui->c);
		} else {
			cairo_set_line_width(ui->c, line_width);
		}
		cairo_stroke(ui->c);
	} else {
		cairo_set_source_rgba(ui->c, rgba[0], rgba[1], rgba[2], rgba[3]);
		cairo_set_line_width(ui->c, line_width);
		cairo_rectangle(ui->c, x, y, w, h);
		if (fill)
			cairo_fill(ui->c);	/* FIXME Should line width be 0 here? */
		else
			cairo_stroke(ui->c);
	}
}

void ui_draw_line(int x0, int y0, int x1, int y1, double line_width, double *rgba)
{
	cairo_set_source_rgba(ui->c, rgba[0], rgba[1], rgba[2], rgba[3]);
	cairo_set_line_width(ui->c, line_width);
	cairo_move_to(ui->c, x0, y0);
	cairo_line_to(ui->c, x1, y1);
	cairo_stroke(ui->c);
}

void ui_insert_text(char *s, int x, int y, int h, double *rgba)
{
	/* Used to centre vertical alignment */
	int offset;

	offset = (h - ui->font_height_actual) / 2;

	/* Text seems to sit one pixel to high */
	++offset;

	pango_layout_set_text(ui->pangolayout, s, -1);
	pango_layout_set_font_description(ui->pangolayout, ui->pangofont);
	cairo_set_source_rgba(ui->c, rgba[0], rgba[1], rgba[2], rgba[3]);
	pango_cairo_update_layout(ui->c, ui->pangolayout);
	cairo_move_to(ui->c, x, y + offset);
	pango_cairo_show_layout(ui->c, ui->pangolayout);
}

int ui_get_text_height(const char *fontdesc)
{
	cairo_surface_t *cs;
	cairo_t *c;
	PangoLayout *layout;
	PangoFontDescription *font;
	PangoLayoutLine *line;
	PangoRectangle ink;
	PangoRectangle logical;

	cs = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 120, 120);
	c = cairo_create(cs);
	layout = pango_cairo_create_layout(c);
	font = pango_font_description_from_string(fontdesc);
	pango_layout_set_text(layout, "foo", -1);
	pango_layout_set_font_description(layout, font);
	cairo_set_source_rgba(c, 0, 0, 0, 1.0);
	pango_cairo_update_layout(c, layout);

	line = pango_layout_get_line_readonly(layout, 0);
	pango_layout_line_get_extents(line, &ink, &logical);
	cairo_surface_destroy(cs);
	pango_font_description_free(font);
	g_object_unref(layout);

	return (logical.height / 1000 + 1);
}

int ui_is_point_in_area(struct Point p, struct Area a)
{
	if ((p.x >= a.x) &&
	    (p.x <= a.x + a.w - 1) &&
	    (p.y >= a.y) &&
	    (p.y <= a.y + a.h - 1))
		return 1;
	else
		return 0;
}


void ui_map_window(unsigned int w, unsigned int h)
{
	XCopyArea(ui->dpy, ui->canvas, ui->win, ui->gc, 0, 0, w, h, 0, 0);
}

void ui_cleanup(void)
{
	XDestroyWindow(ui->dpy, ui->win);
	XUngrabKeyboard(ui->dpy, CurrentTime);
	XUngrabPointer(ui->dpy, CurrentTime);

	if (ui->canvas)
		XFreePixmap(ui->dpy, ui->canvas);
	if (ui->gc)
		XFreeGC(ui->dpy, ui->gc);
	if (ui->dpy)
		XCloseDisplay(ui->dpy);
	if (ui)
		free(ui);


	/* cairo_surface_destroy(ICONS); */

	/* TODO: Free cairo stuff */
	pango_font_description_free(ui->pangofont);
	g_object_unref(ui->pangolayout);
}

cairo_surface_t *ui_get_png_icon(const char *filename)
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


RsvgHandle *ui_get_svg_icon(const char *filename)
{
	GError *err = NULL;
	RsvgHandle *svg = rsvg_handle_new_from_file(filename, &err);
	if (err) {
		fprintf(stderr, "warning: problem loading svg: %s\n", err->message);
		g_error_free(err);
	}

	return svg;
}

void ui_insert_svg(RsvgHandle *svg, double x, double y, double size)
{
	RsvgDimensionData dimensions;
	rsvg_handle_get_dimensions(svg, &dimensions);

	cairo_save(ui->c);
	cairo_translate(ui->c, x, y);
	cairo_scale(ui->c, size / dimensions.width, size / dimensions.width);
	rsvg_handle_render_cairo(svg, ui->c);
	cairo_restore(ui->c);
}

void ui_insert_image(cairo_surface_t *image, double x, double y, double size)
{
	int w, h;

	w = cairo_image_surface_get_width(image);
	h = cairo_image_surface_get_height(image);
	cairo_save(ui->c);
	if (w > size || h > size)
		cairo_scale(ui->c, size / w, size / h);
	cairo_set_source_surface(ui->c, image, x, y);
	cairo_paint(ui->c);
	cairo_restore(ui->c);
}
