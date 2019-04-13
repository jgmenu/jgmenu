/*
 * widgets.c
 *
 * Copyright (C) Johan Malm 2017-2019
 *
 * A very simple widget implementation
 *
 * We read lines beginning with '@' from jgmenu flavoured CSV file and parses in
 * accordance with the following syntax:
 *
 * @type,action,x,y,w,h,r,halign,valign,fgcol,bgcol,content
 *
 * where
 *	- action = what to do when clicked
 *	- (x, y) = margin
 *	- (w, h) = size
 *	- r = corner radius
 *	- content = icon_path or text
 * note
 *	- For RECT, a 1px thick border will be drawn using fgcol
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "list.h"
#include "argv-buf.h"
#include "align.h"
#include "x11-ui.h"
#include "config.h"
#include "geometry.h"
#include "sbuf.h"
#include "filter.h"
#include "icon.h"
#include "x11-ui.h"
#include "widgets.h"

enum widget_type { WIDGET_ERROR, ICON, RECT, TEXT, SEARCH };

static LIST_HEAD(widgets);

/* indicates if mouse is over any of the widgets */
static int mouseover;

/* "widgets" has control of keyboard movement keys */
static int keyboard_grabbed;

struct widget {
	char *buf;
	enum widget_type type;
	char *action;
	int x;
	int y;
	int w;
	int h;
	int r;
	enum alignment halign;
	enum alignment valign;
	double fgcol[4];
	double bgcol[4];
	char *content;
	cairo_surface_t *surface;
	struct list_head list;
};

static struct widget *selection;

static enum widget_type parse_type(const char *field)
{
	if (!field || !field[0])
		return WIDGET_ERROR;
	if (!strcmp(field, "icon"))
		return ICON;
	if (!strcmp(field, "rect"))
		return RECT;
	if (!strcmp(field, "search"))
		return SEARCH;
	if (!strcmp(field, "text"))
		return TEXT;
	return WIDGET_ERROR;
}

static void draw_icon(struct widget **w)
{
	if (!(*w)->surface)
		(*w)->surface = load_cairo_icon((*w)->content, (*w)->w);
	if (!(*w)->surface)
		return;
	ui_insert_image((*w)->surface, (*w)->x, (*w)->y, (*w)->w);
}

static void draw_rect(struct widget **w)
{
	ui_draw_rectangle((*w)->x, (*w)->y, (*w)->w, (*w)->h, (*w)->r,
			  0.0, 1, (*w)->bgcol);
	ui_draw_rectangle((*w)->x, (*w)->y, (*w)->w, (*w)->h, (*w)->r,
			  1.0, 0, (*w)->fgcol);
}

static void draw_search(struct widget **w)
{
	char *t;
	int padding_left = 4;

	if (filter_needle_length())
		t = filter_strdup_needle();
	else
		t = xstrdup((*w)->content);
	ui_insert_text(t, (*w)->x + padding_left, (*w)->y, (*w)->h, (*w)->w,
		       (*w)->fgcol, LEFT);
	xfree(t);
}

static void draw_text(struct widget **w)
{
	ui_insert_text((*w)->content, (*w)->x, (*w)->y, (*w)->h, (*w)->w,
		       (*w)->fgcol, LEFT);
}

static void draw_selection(struct widget **w)
{
	if (!(*w)->action || (*w)->action[0] == '\0')
		return;
	ui_draw_rectangle((*w)->x, (*w)->y, (*w)->w, (*w)->h, (*w)->r,
			  0.0, 1, config.color_sel_bg);
	ui_draw_rectangle((*w)->x, (*w)->y, (*w)->w, (*w)->h, (*w)->r,
			  1.0, 0, config.color_sel_border);
}

void widgets_select(const char *ksym)
{
	struct widget *w;
	enum direction {UNKNOWN, NEXT, PREV};
	enum direction direction = UNKNOWN;

	/*
	 * 'selection' is set when widgets are initiates
	 * if selection == 0, there are no selectable widgets
	 */
	if (!selection)
		return;
	w = selection;
	if (!strcmp(ksym, "XK_Down"))
		direction = NEXT;
	else if (!strcmp(ksym, "XK_Up"))
		direction = PREV;
	if (direction == UNKNOWN)
		warn("unknown string passed to %s", __func__);
	for (;;) {
		switch (direction) {
		case NEXT:
			w = container_of(w->list.next, struct widget, list);
			break;
		case PREV:
			w = container_of(w->list.prev, struct widget, list);
			break;
		case UNKNOWN:
			return;
		}
		if (!w->action || w->action[0] == '\0')
			continue;
		selection = w;
		break;
	}
}

int widgets_get_kb_grabbed(void)
{
	return keyboard_grabbed;
}

void widgets_toggle_kb_grabbed(void)
{
	keyboard_grabbed = keyboard_grabbed ? 0 : 1;
}

int widgets_mouseover(void)
{
	return mouseover;
}

void widgets_set_pointer_position(int x, int y)
{
	struct widget *w;
	struct area widget_area;
	struct point pointer;

	mouseover = 0;
	pointer.x = x;
	pointer.y = y;
	list_for_each_entry(w, &widgets, list) {
		widget_area.x = w->x;
		widget_area.y = w->y;
		widget_area.w = w->w;
		widget_area.h = w->h;
		if (ui_is_point_in_area(pointer, widget_area)) {
			selection = w;
			mouseover = 1;
			break;
		}
	}
}

/*
 * widgets_set_point_position() should be run just before calling
 * this function
 */
char *widgets_get_selection_action(void)
{
	struct widget *w;

	list_for_each_entry(w, &widgets, list) {
		if (selection == w) {
			if (!w->action || w->action[0] == '\0')
				continue;
			return w->action;
		}
	}
	return NULL;
}

void widgets_draw(void)
{
	struct widget *w;

	if (list_empty(&widgets))
		return;
	list_for_each_entry(w, &widgets, list) {
		if (selection == w)
			draw_selection(&w);
		if (w->type == ICON)
			draw_icon(&w);
		else if (w->type == RECT)
			draw_rect(&w);
		else if (w->type == SEARCH)
			draw_search(&w);
		else if (w->type == TEXT)
			draw_text(&w);
		else
			warn("widget type not recognised");
	}
}

static void color_copy(double to[4], double from[4])
{
	int i;

	for (i = 0; i < 4; i++)
		to[i] = from[i];
}

void widgets_add(const char *s)
{
	struct argv_buf argv_buf;
	struct widget *w;

	w = xmalloc(sizeof(struct widget));
	argv_init(&argv_buf);
	argv_set_delim(&argv_buf, ',');
	argv_strdup(&argv_buf, s);
	argv_parse(&argv_buf);
	if (argv_buf.argc != 12)
		warn("widget did not contain 12 fields");
	w->buf = argv_buf.buf;
	w->type = parse_type(argv_buf.argv[0] + 1);
	w->action = argv_buf.argv[1];
	remove_caret_markup_closing_bracket(w->action);
	xatoi(&w->x, argv_buf.argv[2], XATOI_NONNEG, "w->x");
	xatoi(&w->y, argv_buf.argv[3], XATOI_NONNEG, "w->y");
	xatoi(&w->w, argv_buf.argv[4], XATOI_NONNEG, "w->w");
	xatoi(&w->h, argv_buf.argv[5], XATOI_NONNEG, "w->h");
	xatoi(&w->r, argv_buf.argv[6], XATOI_NONNEG, "w->r");
/*	enum alignment halign; */
/*	enum alignment valign; */
	if (!strcasecmp(argv_buf.argv[9], "auto"))
		color_copy(w->fgcol, config.color_norm_fg);
	else
		parse_hexstr(argv_buf.argv[9], w->fgcol);
	parse_hexstr(argv_buf.argv[10], w->bgcol);
	w->content = argv_buf.argv[11];
	w->surface = NULL;
	list_add_tail(&w->list, &widgets);
	if (selection)
		return;
	if (w->action && w->action[0] != '\0')
		selection = w;
}

void widgets_cleanup(void)
{
	struct widget *w, *tmp_w;

	list_for_each_entry_safe(w, tmp_w, &widgets, list) {
		xfree(w->buf);
		list_del(&w->list);
		xfree(w);
	}
}
