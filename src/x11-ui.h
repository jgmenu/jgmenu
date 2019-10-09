#ifndef X11_UI_H
#define X11_UI_H

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <cairo.h>
#include <cairo-xlib.h>
#include <pango/pangocairo.h>
#include <librsvg/rsvg.h>

#include "align.h"

struct area {
	int x, y, w, h;
};

struct point {
	int x, y;
};

struct window_data {
	Window win;
	XIC xic;
	XSetWindowAttributes swa;
	GC gc;
	Pixmap canvas;
	cairo_surface_t *cs;
	cairo_t *c;
	PangoLayout *pangolayout;
	PangoFontDescription *pangofont;
};

struct UI {
	int cur;
	struct window_data w[MAX_NR_WINDOWS];
	Display *dpy;
	XIM xim;
	int screen;
	Window root;
	XVisualInfo vinfo;
	int font_height_actual;		/* used to centre text vertically */
};

extern struct UI *ui;

int ui_get_workarea(struct area *a);
void ui_clear_canvas(void);
void grabkeyboard(void);
void grabpointer(void);
void ui_init(void);
void ui_get_screen_res(int *x0, int *y0, int *width, int *height, int monitor);
void ui_create_window(int x, int y, int w, int h);
void ui_init_canvas(int max_width, int max_height);
void ui_init_cairo(int canvas_width, int canvas_height, const char *font);
void ui_win_init(int x, int y, int w, int h, int max_w, int max_h, const char *font);
void ui_win_add(int x, int y, int w, int h, int max_w, int max_h, const char *font);
void ui_win_activate(Window w);
int ui_has_child_window_open(Window w);
void ui_win_del(void);
void ui_win_del_beyond(int w);
void ui_draw_rectangle_rounded_at_top(double x, double y, double w, double h, double radius,
				      double line_width, int fill, double *rgba);
void ui_draw_rectangle(double x, double y, double w, double h, double radius, double line_width,
		       int fill, double *rgba);
void ui_draw_line(double x0, double y0, double x1, double y1, double line_width, double *rgba);
void ui_insert_text(char *s, int x, int y, int h, int w, double *rgba,
		    enum alignment align);
struct point ui_get_text_size(const char *str, const char *fontdesc);
int ui_is_point_in_area(struct point p, struct area a);
void ui_map_window(unsigned int w, unsigned int h);
void ui_cleanup(void);
void ui_insert_image(cairo_surface_t *image, double x, double y, double size);

#endif /* X11_UI_H */
