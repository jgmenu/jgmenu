/* x11-ui.c
 *
 * x11-ui.c provides a simple X11 interface.
 * It uses cairo for shapes and pango for text.
 *
 * It has been inspired by:
 *  - dmenu 4.5 (http://tools.suckless.org/dmenu/)
 *  - tint2 (https://gitlab.com/o9000/tint2)
 *  - dzen2 (https://github.com/robm/dzen)
 *
 * We try to use X11 terminology, for example:
 *   - screen  - an area into which graphics can be rendered
 *   - display - a collection of screens
 *   - monitor - physical device such as CRT
 *   - crtc    - CRT controller
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>

#include "x11-ui.h"
#include "util.h"
#include "config.h"
#include "banned.h"

struct UI *ui;

static void *win_prop(Window win, Atom property, Atom req_type)
{
	Atom actual_type_return;
	int actual_format_return = 0;
	unsigned long nitems_return = 0;
	unsigned long bytes_after_return = 0;
	unsigned char *prop_return;

	if (!win)
		return NULL;
	if (XGetWindowProperty(ui->dpy, win, property, 0, 0x7fffffff, False,
			       req_type, &actual_type_return,
			       &actual_format_return, &nitems_return,
			       &bytes_after_return, &prop_return) != Success)
		return NULL;
	return prop_return;
}

int ui_get_workarea(struct area *a)
{
	long *wa;

	wa = win_prop(ui->root, XInternAtom(ui->dpy, "_NET_WORKAREA", False),
		      XA_CARDINAL);
	if (!wa)
		return -1;
	a->x = (int)wa[0];
	a->y = (int)wa[1];
	a->w = (int)wa[2];
	a->h = (int)wa[3];
	XFree(wa);
	return 0;
}

void ui_clear_canvas(void)
{
	cairo_save(ui->w[ui->cur].c);
	cairo_set_source_rgba(ui->w[ui->cur].c, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator(ui->w[ui->cur].c, CAIRO_OPERATOR_SOURCE);
	cairo_paint(ui->w[ui->cur].c);
	cairo_restore(ui->w[ui->cur].c);
}

void grabkeyboard(void)
{
	/* The "waiting" trick has been copied from dmenu-4.5 */
	int i;

	for (i = 0; i < 50; i++) {
		if (XGrabKeyboard(ui->dpy, ui->root, True,
				  GrabModeAsync, GrabModeAsync,
				  CurrentTime) == GrabSuccess)
			return;
		msleep(20);
	}
	die("cannot grab keyboard");
}

/*
 * grabpointer() is needed in order to detect when pointer is clicked
 * outside menu
 */
void grabpointer(void)
{
	int i;

	/*
	 * Setting the third argument of XGrabPointer() to "False"
	 * gives (x,y) wrt root window.
	 */
	for (i = 0; i < 50; i++) {
		if (XGrabPointer(ui->dpy, ui->root,
				 False,
				 ButtonPressMask | ButtonReleaseMask |
				 PointerMotionMask | FocusChangeMask |
				 EnterWindowMask | LeaveWindowMask,
				 GrabModeAsync, GrabModeAsync, None, None,
				 CurrentTime) == GrabSuccess)
			return;
		msleep(20);
	}
	die("cannot grab pointer");
}

static void get_visual_info(void)
{
	int i, depths[] = { 32, 24, 8, 4, 2, 1, 0 };

	for (i = 0; depths[i]; i++) {
		XMatchVisualInfo(ui->dpy, ui->screen, depths[i], TrueColor,
				 &ui->vinfo);
		if (ui->vinfo.visual)
			break;
	}
	info("color depth=%d", ui->vinfo.depth);
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

	ui->xim = XOpenIM(ui->dpy, NULL, NULL, NULL);
	ui->screen = DefaultScreen(ui->dpy);
	get_visual_info();
	ui->root = RootWindow(ui->dpy, ui->screen);
}

static void print_screen_info(void)
{
	int i;
	XRRScreenResources *sr;
	XRRCrtcInfo *ci = NULL;
	static int info_has_been_shown;

	if (info_has_been_shown)
		return;
	info_has_been_shown = 1;

	sr = XRRGetScreenResourcesCurrent(ui->dpy, ui->root);
	info("%d xrandr crt controller(s) found", sr->ncrtc);
	for (i = 0; i < sr->ncrtc; i++) {
		ci = XRRGetCrtcInfo(ui->dpy, sr, sr->crtcs[i]);
		if (!ci->noutput)
			continue;
		printf("    - monitor-%d: x0=%d; y0=%d; w=%d; h=%d\n",
		       i + 1, ci->x, ci->y, ci->width, ci->height);
		XRRFreeCrtcInfo(ci);
	}
	XRRFreeScreenResources(sr);
}

static int intersect(int x, int y, int w, int h, XRRCrtcInfo *ci)
{
	return MAX(0, MIN(x + w, (int)ci->x + (int)ci->width) - MAX(x, (int)ci->x)) &&
	       MAX(0, MIN(y + h, (int)ci->y + (int)ci->height) - MAX(y, (int)ci->y));
}

void ui_get_screen_res(int *x0, int *y0, int *width, int *height, int monitor)
{
	int i, n, x, y, di;
	unsigned int du;
	Window dw;
	XRRScreenResources *sr;
	XRRCrtcInfo *ci = NULL;

	if (config.verbosity >= 3)
		print_screen_info();
	sr = XRRGetScreenResourcesCurrent(ui->dpy, ui->root);
	BUG_ON(!sr);
	n = sr->ncrtc;

	/*
	 * Global variable config.monitor let's the user specify a monitor.
	 * If not set, we use the current pointer position
	 */
	if (monitor) {
		if (monitor > n)
			die("cannot connect to monitor '%d'", monitor);
		ci = XRRGetCrtcInfo(ui->dpy, sr, sr->crtcs[monitor - 1]);
		if (!ci->noutput)
			die("cannot connect to monitor '%d'", monitor);
		if (config.verbosity >= 3)
			info("using user specified monitor '%d'", monitor);
		goto monitor_selected;
	}

	XQueryPointer(ui->dpy, ui->root, &dw, &dw, &x, &y, &di, &di, &du);
	for (i = 0; i < n; i++) {
		if (ci)
			XRRFreeCrtcInfo(ci);
		ci = XRRGetCrtcInfo(ui->dpy, sr, sr->crtcs[i]);
		BUG_ON(!ci);
		if (!ci->noutput)
			continue;
		if (intersect(x, y, 1, 1, ci)) {
			if (config.verbosity >= 3)
				info("using monitor '%d'", i + 1);
			break;
		}
	}

monitor_selected:
	if (!ci)
		die("connection could be established to monitor");
	*x0 = ci->x;
	*y0 = ci->y;
	*width = ci->width;
	*height = ci->height;
	XRRFreeCrtcInfo(ci);
	XRRFreeScreenResources(sr);
}

static void set_wm_class(void)
{
	XClassHint *classhint = XAllocClassHint();

	classhint->res_name = (char *)"jgmenu";
	classhint->res_class = (char *)"jgmenu";
	XSetClassHint(ui->dpy, ui->w[ui->cur].win, classhint);
	XFree(classhint);
}

void ui_create_window(int x, int y, int w, int h)
{
	ui->w[ui->cur].swa.override_redirect = True;
	ui->w[ui->cur].swa.event_mask = ExposureMask | KeyPressMask | VisibilityChangeMask | ButtonPressMask;
	ui->w[ui->cur].swa.colormap = XCreateColormap(ui->dpy, ui->root, ui->vinfo.visual, AllocNone);
	ui->w[ui->cur].swa.background_pixel = 0;
	ui->w[ui->cur].swa.border_pixel = 0;
	ui->w[ui->cur].win = XCreateWindow(ui->dpy, ui->root, x, y, w, h, 0,
					   ui->vinfo.depth, CopyFromParent,
					   ui->vinfo.visual,
					   CWOverrideRedirect | CWColormap |
					   CWBackPixel | CWEventMask |
					   CWBorderPixel,
					   &ui->w[ui->cur].swa);
	ui->w[ui->cur].xic = XCreateIC(ui->xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
			    XNClientWindow, ui->w[ui->cur].win, XNFocusWindow, ui->w[ui->cur].win, NULL);
	ui->w[ui->cur].gc = XCreateGC(ui->dpy, ui->w[ui->cur].win, 0, NULL);
	XStoreName(ui->dpy, ui->w[ui->cur].win, "jgmenu");
	XSetIconName(ui->dpy, ui->w[ui->cur].win, "jgmenu");
	set_wm_class();

	/*
	 * XDefineCursor required to prevent blindly inheriting cursor from parent
	 * (e.g. hour-glass pointer set by tint2)
	 * Check this URL for cursor styles:
	 * http://tronche.com/gui/x/xlib/appendix/b/
	 */
	XDefineCursor(ui->dpy, ui->w[ui->cur].win, XCreateFontCursor(ui->dpy, 68));
	XSync(ui->dpy, False);
}

/*
 * max_height is that associated with the longest submenu
 */
void ui_init_canvas(int max_width, int max_height)
{
	ui->w[ui->cur].canvas = XCreatePixmap(ui->dpy, ui->root, max_width,
					      max_height, ui->vinfo.depth);
}

void ui_init_cairo(int canvas_width, int canvas_height, const char *font)
{
	struct point p;

	ui->w[ui->cur].cs = cairo_xlib_surface_create(ui->dpy,
						      ui->w[ui->cur].canvas,
						      ui->vinfo.visual,
						      canvas_width,
						      canvas_height);
	ui->w[ui->cur].c = cairo_create(ui->w[ui->cur].cs);

	/*
	 * pango-font-description-from-string() interprets the size without
	 * a suffix as "points". If "px" is added, it will be read as pixels.
	 */
	ui->w[ui->cur].pangolayout = pango_cairo_create_layout(ui->w[ui->cur].c);
	ui->w[ui->cur].pangofont = pango_font_description_from_string(font);

	p = ui_get_text_size("abcfghjklABC", font);
	ui->font_height_actual = p.y;
}

void ui_win_init(int x, int y, int w, int h, int max_w, int max_h, const char *font)
{
	ui->cur = 0;
	ui_create_window(x, y, w, h);
	ui_init_canvas(max_w, max_h);
	ui_init_cairo(max_w, max_h, font);
}

void ui_win_add(int x, int y, int w, int h, int max_w, int max_h, const char *font)
{
	ui->cur++;
	ui_create_window(x, y, w, h);
	ui_init_canvas(max_w, max_h);
	ui_init_cairo(max_w, max_h, font);
	XMapWindow(ui->dpy, ui->w[ui->cur].win);
}

void ui_win_activate(Window w)
{
	int i;

	for (i = 0; ui->w[i].c; i++)
		if (w == ui->w[i].win)
			goto out;
	die("badness: %s", __func__);
out:
	ui->cur = i;
}

int ui_has_child_window_open(Window w)
{
	int i;

	for (i = 0; ui->w[i].c; i++)
		if (w == ui->w[i].win)
			break;
	if (ui->w[i + 1].c)
		return 1;
	return 0;
}

static void del_win(int win_index)
{
	if (!ui->w[win_index].c)
		die("there is not a window to delete");
	XMapWindow(ui->dpy, ui->w[win_index].win);
	XDestroyWindow(ui->dpy, ui->w[win_index].win);
	XDestroyIC(ui->w[win_index].xic);
	XFreePixmap(ui->dpy, ui->w[win_index].canvas);
	XFreeGC(ui->dpy, ui->w[win_index].gc);
	cairo_destroy(ui->w[win_index].c);
	ui->w[win_index].c = NULL;
	cairo_surface_destroy(ui->w[win_index].cs);
	pango_font_description_free(ui->w[win_index].pangofont);
	g_object_unref(ui->w[win_index].pangolayout);
}

void ui_win_del(void)
{
	del_win(ui->cur);
	ui->cur--;
}

void ui_win_del_beyond(int w)
{
	int i;

	if (w < 0)
		die("%s: 'w' cannot be less than zero", __func__);
	for (i = 0; ui->w[i].c; i++)
		;
	i--;
	while (i > w)
		del_win(i--);
	ui->cur = w;
}

static void set_rgba(double *rgba)
{
	cairo_set_source_rgba(ui->w[ui->cur].c, rgba[0], rgba[1], rgba[2],
			      rgba[3]);
}

void ui_draw_rectangle_rounded_at_top(double x, double y, double w, double h,
				      double radius, double line_width, int fill, double *rgba)
{
	double deg = 0.017453292519943295; /* 2 x 3.1415927 / 360.0 */

	cairo_new_sub_path(ui->w[ui->cur].c);
	cairo_arc(ui->w[ui->cur].c, x + w - radius, y + radius, radius, -90 * deg, 0 * deg);  /* NE */
	cairo_arc(ui->w[ui->cur].c, x + w, y + h, 0, 0 * deg, 90 * deg);			   /* SE */
	cairo_arc(ui->w[ui->cur].c, x, y + h, 0, 90 * deg, 180 * deg);			   /* SW */
	cairo_arc(ui->w[ui->cur].c, x + radius, y + radius, radius, 180 * deg, 270 * deg);    /* NE */
	cairo_close_path(ui->w[ui->cur].c);
	set_rgba(rgba);
	if (fill) {
		cairo_set_line_width(ui->w[ui->cur].c, 0.0);
		cairo_fill_preserve(ui->w[ui->cur].c);
	} else {
		cairo_set_line_width(ui->w[ui->cur].c, line_width);
	}
	cairo_stroke(ui->w[ui->cur].c);
}

void ui_draw_rectangle(double x, double y, double w, double h, double radius, double line_width, int fill, double *rgba)
{
	x += line_width / 2;
	y += line_width / 2;
	w -= line_width;
	h -= line_width;

	cairo_set_line_width(ui->w[ui->cur].c, 0.0);
	if (radius > 0) {
		double deg = 0.017453292519943295; /* 2 x 3.1415927 / 360.0 */

		cairo_new_sub_path(ui->w[ui->cur].c);
		cairo_arc(ui->w[ui->cur].c, x + w - radius, y + radius, radius, -90 * deg, 0 * deg);
		cairo_arc(ui->w[ui->cur].c, x + w - radius, y + h - radius, radius, 0 * deg, 90 * deg);
		cairo_arc(ui->w[ui->cur].c, x + radius, y + h - radius, radius, 90 * deg, 180 * deg);
		cairo_arc(ui->w[ui->cur].c, x + radius, y + radius, radius, 180 * deg, 270 * deg);
		cairo_close_path(ui->w[ui->cur].c);
		set_rgba(rgba);
		if (fill) {
			cairo_set_line_width(ui->w[ui->cur].c, 0.0);
			cairo_fill_preserve(ui->w[ui->cur].c);
		} else {
			cairo_set_line_width(ui->w[ui->cur].c, line_width);
		}
		cairo_stroke(ui->w[ui->cur].c);
	} else {
		set_rgba(rgba);
		cairo_set_line_width(ui->w[ui->cur].c, line_width);
		cairo_rectangle(ui->w[ui->cur].c, x, y, w, h);
		if (fill)
			cairo_fill(ui->w[ui->cur].c);	/* FIXME Should line width be 0 here? */
		else
			cairo_stroke(ui->w[ui->cur].c);
	}
}

void ui_draw_line(double x0, double y0, double x1, double y1, double line_width, double *rgba)
{
	set_rgba(rgba);
	cairo_set_line_width(ui->w[ui->cur].c, line_width);
	cairo_move_to(ui->w[ui->cur].c, x0, y0);
	cairo_line_to(ui->w[ui->cur].c, x1, y1);
	cairo_stroke(ui->w[ui->cur].c);
}

void ui_insert_text(char *s, int x, int y, int h, int w, double *rgba,
		    enum alignment align)
{
	PangoTabArray *tabs;
	int height;

	pango_layout_set_width(ui->w[ui->cur].pangolayout, w * PANGO_SCALE);
	switch (align) {
	case RIGHT:
		pango_layout_set_alignment(ui->w[ui->cur].pangolayout, PANGO_ALIGN_RIGHT);
		break;
	case CENTER:
		pango_layout_set_alignment(ui->w[ui->cur].pangolayout, PANGO_ALIGN_CENTER);
		break;
	default:
		pango_layout_set_alignment(ui->w[ui->cur].pangolayout, PANGO_ALIGN_LEFT);
	}
	tabs = pango_tab_array_new_with_positions(1, TRUE, PANGO_TAB_LEFT, config.tabs);
	pango_layout_set_wrap(ui->w[ui->cur].pangolayout, PANGO_WRAP_WORD_CHAR);
	pango_layout_set_ellipsize(ui->w[ui->cur].pangolayout, PANGO_ELLIPSIZE_END);
	pango_layout_set_font_description(ui->w[ui->cur].pangolayout, ui->w[ui->cur].pangofont);
	pango_layout_set_tabs(ui->w[ui->cur].pangolayout, tabs);
	pango_layout_set_markup(ui->w[ui->cur].pangolayout, s, -1);
	set_rgba(rgba);
	pango_cairo_update_layout(ui->w[ui->cur].c, ui->w[ui->cur].pangolayout);
	pango_layout_get_pixel_size(ui->w[ui->cur].pangolayout, NULL, &height);
	/* use (h - height) / 2 to center-align vertically */
	cairo_move_to(ui->w[ui->cur].c, x, y + (h - height) / 2);
	pango_cairo_show_layout(ui->w[ui->cur].c, ui->w[ui->cur].pangolayout);
	pango_tab_array_free(tabs);
}

struct point ui_get_text_size(const char *str, const char *fontdesc)
{
	cairo_surface_t *cs;
	cairo_t *c;
	PangoLayout *layout;
	PangoFontDescription *font;
	struct point point;

	cs = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 120, 120);
	c = cairo_create(cs);
	layout = pango_cairo_create_layout(c);
	font = pango_font_description_from_string(fontdesc);
	pango_layout_set_font_description(layout, font);
	pango_layout_set_markup(layout, str, -1);
	pango_cairo_update_layout(c, layout);
	pango_layout_get_pixel_size(layout, &point.x, &point.y);
	cairo_surface_destroy(cs);
	cairo_destroy(c);
	pango_font_description_free(font);
	g_object_unref(layout);

	return point;
}

int ui_is_point_in_area(struct point p, struct area a)
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
	XCopyArea(ui->dpy, ui->w[ui->cur].canvas, ui->w[ui->cur].win, ui->w[ui->cur].gc, 0, 0, w, h, 0, 0);
}

void ui_cleanup(void)
{
	XDestroyWindow(ui->dpy, ui->w[ui->cur].win);
	XUngrabKeyboard(ui->dpy, CurrentTime);
	XUngrabPointer(ui->dpy, CurrentTime);
	XDestroyIC(ui->w[0].xic);
	XCloseIM(ui->xim);

	if (ui->w[ui->cur].canvas)
		XFreePixmap(ui->dpy, ui->w[ui->cur].canvas);
	if (ui->w[ui->cur].gc)
		XFreeGC(ui->dpy, ui->w[ui->cur].gc);
	if (ui->dpy)
		XCloseDisplay(ui->dpy);

	cairo_destroy(ui->w[ui->cur].c);
	cairo_surface_destroy(ui->w[ui->cur].cs);
	pango_font_description_free(ui->w[ui->cur].pangofont);
	g_object_unref(ui->w[ui->cur].pangolayout);
	xfree(ui);
}

void ui_insert_image(cairo_surface_t *image, double x, double y, double size, double alpha)
{
	double w, h, max;

	cairo_save(ui->w[ui->cur].c);
	cairo_translate(ui->w[ui->cur].c, x, y);

	/* scale */
	w = cairo_image_surface_get_width(image);
	h = cairo_image_surface_get_height(image);
	max = h > w ? h : w;
	if (max != size)
		cairo_scale(ui->w[ui->cur].c, size / max, size / max);

	cairo_set_source_surface(ui->w[ui->cur].c, image, 0, 0);
	cairo_paint_with_alpha(ui->w[ui->cur].c, alpha);
	cairo_restore(ui->w[ui->cur].c);
}
