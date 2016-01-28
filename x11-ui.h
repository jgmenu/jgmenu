#ifndef X11_UI_H
#define X11_UI_H

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <cairo.h>
#include <cairo-xlib.h>
#include <pango/pangocairo.h>

struct UI {
	Display *dpy;
	int screen;
	GC gc;
	Window win;
	Window root;
	XSetWindowAttributes swa;
	XIC xic;
	Pixmap canvas;
	XVisualInfo vinfo;
	cairo_surface_t *cs;
	cairo_t *c;
	PangoLayout *pangolayout;
	PangoFontDescription *pangofont;
};

struct UI *ui;

void ui_clear_canvas();
void ui_clear_canvas2(int x, int y, int w, int h);
void ui_init_cairo(int canvas_width, int canvas_height, const char *font);
void ui_init(void);
void ui_get_screen_res(int *x0, int *y0, int *width, int *height);
void ui_init_canvas(int max_width, int max_height);
void ui_create_window(int x, int y, int w, int h);
void ui_draw_rectangle_rounded_at_top(double x, double y, double w, double h, double radius, int fill, double *rgba);
void ui_draw_rectangle(double x, double y, double w, double h, double radius, int fill, double *rgba);
void ui_draw_line(int x0, int y0, int x1, int y1, double *rgba);
void ui_insert_text(char *s, int x, int y, int h, double *rgba);
int ui_get_text_height(char *fontdesc);
void ui_map_window(unsigned int w, unsigned int h);
void ui_cleanup(void);

#endif /* X11_UI_H */
