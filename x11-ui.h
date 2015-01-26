#ifndef X11_UI_H
#define X11_UI_H

#include <unistd.h>		/* for usleep */
#include <cairo.h>
#include <cairo-xlib.h>
#include <pango/pangocairo.h>

struct UI {
	Display *dpy;
	GC gc;
	Window win;
	XIC xic;
	Pixmap canvas;
	cairo_surface_t *cs;
	cairo_t *c;
	PangoLayout *pangolayout;
	PangoFontDescription *pangofont;
};

struct UI *ui;

void ui_init();
void ui_get_screen_res();
void ui_create_window(void);
void ui_draw_rectangle(int x, int y, int w, int h, int fill,
		       float r, float g, float b, float a);
void ui_draw_line(int x0, int y0, int x1, int y1,
		       float r, float g, float b, float a);
void ui_insert_text();
int ui_get_text_height(char *fontdesc);
void ui_map_window(unsigned int w, unsigned int h);
void ui_cleanup(void);

#endif /* X11_UI_H */
