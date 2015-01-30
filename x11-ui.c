/* x11-ui.c
 *
 * x11-ui.c provides a simple X11 interface.
 * It uses cairo for shapes and pango for text.
 * 
 * It has been inspired by:
 *  - dmenu 4.5 (http://tools.suckless.org/dmenu/)
 *  - tint2 (https://code.google.com/p/tint2/source/browse/)
 *  - dzen2 (https://github.com/robm/dzen)
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#ifdef XINERAMA
#include <X11/extensions/Xinerama.h>
#endif
#include "x11-ui.h"
#include "config.h"
#include "util.h"

/* INTERSECT is required by Xinerama.  MAX and MIN are defined in glib */
#define INTERSECT(x,y,w,h,r)  (MAX(0, MIN((x)+(w),(r).x_org+(r).width)  - \
			       MAX((x),(r).x_org)) * \
                               MAX(0, MIN((y)+(h),(r).y_org+(r).height) - \
			       MAX((y),(r).y_org)))


void grabkeyboard(void)
{
	/* The "waiting" trick has been copied from dmenu-4.5 */
	int i;

	for(i = 0; i < 1000; i++) {
		if(XGrabKeyboard(ui->dpy, DefaultRootWindow(ui->dpy), True,
		                 GrabModeAsync, GrabModeAsync, 
				 CurrentTime) == GrabSuccess)
			return;
		usleep(1000);
	}
	die("cannot grab keyboard");
}


/* grabpointer() is needed in order to detect when pointer is clicked
 * outside menu
 */
void grabpointer(void)
{
	XGrabPointer(ui->dpy, DefaultRootWindow(ui->dpy),
		     True,	/* False gives (x,y) wrt root window. */
 	             ButtonPressMask | ButtonReleaseMask | 
		     PointerMotionMask | FocusChangeMask |
		     EnterWindowMask | LeaveWindowMask,
	             GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
}


void init_cairo(void)
{
        Visual *vis;
        vis = DefaultVisual(ui->dpy, 0);

	/* Would be tidier to calc height to largest submenu rather than 
	 * allocating memory for (menu.end * menu.item_h) */
        ui->cs = cairo_xlib_surface_create(ui->dpy, ui->canvas, vis, menu.menu_w,
					   menu.end * menu.item_h);	

        ui->c = cairo_create (ui->cs);

	ui->pangolayout = pango_cairo_create_layout(ui->c);
	ui->pangofont = pango_font_description_from_string(menu.font);
}

void ui_init()
{
	ui = xcalloc(1, sizeof *ui);

	if(!(ui->dpy = XOpenDisplay(NULL)))
		die("cannot open display");

	ui->gc = XCreateGC(ui->dpy, DefaultRootWindow(ui->dpy), 0, NULL);

	grabkeyboard();
	grabpointer();
}

/* The Xinerama code below was copied from dmenu's xft patch
 * (http://tools.suckless.org/dmenu/patches/xft)
 */
void ui_get_screen_res(int *x0, int *y0, int *width, int *height)
{
	int screen = DefaultScreen(ui->dpy);
#ifdef XINERAMA
	Window root = RootWindow(ui->dpy, screen);
	int n;
	XineramaScreenInfo *info;
	if((info = XineramaQueryScreens(ui->dpy, &n))) {
		int a, j, di, i = 0, area = 0;
		unsigned int du;
		Window w, pw, dw, *dws;
		XWindowAttributes wa;

		XGetInputFocus(ui->dpy, &w, &di);
		if(w != root && w != PointerRoot && w != None) {
			/* find top-level window containing current input focus */
			do {
				if(XQueryTree(ui->dpy, (pw = w), &dw, &w, &dws, &du) && dws)
					XFree(dws);
			} while(w != root && w != pw);
			/* find xinerama screen with which the window intersects most */
			if(XGetWindowAttributes(ui->dpy, pw, &wa))
				for(j = 0; j < n; j++)
					if((a = INTERSECT(wa.x, wa.y, wa.width, wa.height, info[j])) > area) {
						area = a;
						i = j;
					}
		}

		/* no focused window is on screen, so use pointer location instead */
/*		if(!area && XQueryPointer(ui->dpy, root, &dw, &dw, x, y, &di, &di, &du))
			for(i = 0; i < n; i++)
				if(INTERSECT(*x, *y, 1, 1, info[i]))
					break;
*/

/* SET MENU DIMENSIONS */

		*x0 = info[i].x_org;
		*y0 = info[i].y_org;
		*width = info[i].width;
		*height = info[i].height;

		XFree(info);
	}
	else
#endif
	{
		*x0 = 0;
		*y0 = 0;
		*width = DisplayWidth(ui->dpy, screen); 
		*height = DisplayHeight(ui->dpy, screen);
	}
}


void ui_create_window(void)
{
	/* The following is heavily based dmenu's draw.c */
	int screen = DefaultScreen(ui->dpy);
	Window root = RootWindow(ui->dpy, screen);
	XSetWindowAttributes swa;
	XIM xim;

	swa.override_redirect = True;
	swa.event_mask = ExposureMask | KeyPressMask | VisibilityChangeMask | ButtonPressMask;
	ui->win = XCreateWindow(ui->dpy, root, menu.win_x0, menu.win_y0, menu.menu_w, menu.menu_h, 0,
	                    DefaultDepth(ui->dpy, screen), CopyFromParent,
	                    DefaultVisual(ui->dpy, screen),
	                    CWOverrideRedirect | CWBackPixel | CWEventMask, &swa);
	xim = XOpenIM(ui->dpy, NULL, NULL, NULL);
	ui->xic = XCreateIC(xim, XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
	                XNClientWindow, ui->win, XNFocusWindow, ui->win, NULL);
	XMapRaised(ui->dpy, ui->win);
	if(ui->canvas)
		XFreePixmap(ui->dpy, ui->canvas);

	/* Would be tidier to calc height to largest submenu rather than 
	 * allocating memory for (menu.end * menu.item_h) */
	ui->canvas = XCreatePixmap(ui->dpy, DefaultRootWindow(ui->dpy), menu.menu_w,
				   menu.end * menu.item_h,
	                           DefaultDepth(ui->dpy, screen));
	/* END OF COPY */

	init_cairo();

	/* XDefineCursor required to prevent blindly inheriting cursor from parent
	 * (e.g. hour-glass pointer set by tint2)
	 * Check this URL for cursor styles:
	 * http://tronche.com/gui/x/xlib/appendix/b/	
	 */
	XDefineCursor(ui->dpy, ui->win, XCreateFontCursor(ui->dpy, 68));
}


void ui_draw_rectangle(int x, int y, int w, int h, int fill,
		       float r, float g, float b, float a)
{
	cairo_set_source_rgba (ui->c, r, g, b, a);
	cairo_set_line_width(ui->c, 0.5);
	cairo_rectangle(ui->c, x, y, w, h);
	if(fill)
		cairo_fill(ui->c);
	else
		cairo_stroke(ui->c);
}

void ui_draw_line(int x0, int y0, int x1, int y1,
		       float r, float g, float b, float a)
{
	cairo_set_source_rgba (ui->c, r, g, b, a);
	cairo_set_line_width(ui->c, 0.5);
	cairo_move_to(ui->c, x0, y0);
	cairo_line_to(ui->c, x1, y1);
	cairo_stroke(ui->c);
}

void ui_insert_text(char *s, int x, int y, int h)
{
	pango_layout_set_text(ui->pangolayout, s, -1);
	pango_layout_set_font_description(ui->pangolayout, ui->pangofont);
	cairo_set_source_rgba(ui->c, 0, 0, 0, 1.0);
	pango_cairo_update_layout(ui->c, ui->pangolayout);
	cairo_move_to(ui->c, x + 4, y);
	pango_cairo_show_layout(ui->c, ui->pangolayout);
}

int ui_get_text_height(char *fontdesc)
{
	cairo_surface_t *cs;
	cairo_t *c;
	PangoLayout *layout;
	PangoFontDescription *font;
	PangoLayoutLine *line;
	PangoRectangle ink;
	PangoRectangle logical;

	cs = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 120, 120);
	c = cairo_create (cs);
	layout = pango_cairo_create_layout(c);
	font = pango_font_description_from_string(fontdesc);
	pango_layout_set_text(layout, "foo", -1);
	pango_layout_set_font_description(layout, font);
	cairo_set_source_rgba(c, 0, 0, 0, 1.0);
	pango_cairo_update_layout(c, layout);

	line = pango_layout_get_line_readonly(layout, 0);
	pango_layout_line_get_extents (line, &ink, &logical);
	/* TODO: FREE CAIRO MEMORY */
	pango_font_description_free(font);
	g_object_unref(layout);

	return (logical.height / 1000 + 1);
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

	if(ui->canvas)
		XFreePixmap(ui->dpy, ui->canvas);
	if(ui->gc)
        	XFreeGC(ui->dpy, ui->gc);
	if(ui->dpy)
        	XCloseDisplay(ui->dpy);
	if(ui)
		free(ui);

	/* TODO: Free cairo stuff */
	pango_font_description_free(ui->pangofont);
	g_object_unref(ui->pangolayout);
}
