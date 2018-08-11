/*
 * widgets.c
 *
 * Copyright (C) Johan Malm 2017
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

enum widget_type { WIDGET_ERROR, ICON, RECT, TEXT, SEARCH };

static LIST_HEAD(widgets);
static struct point mouse;

/* indicates if mouse is over any of the widgets */
static int mouseover;

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
	return TEXT;
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
			  1.0, 0, (*w)->fgcol);
	ui_draw_rectangle((*w)->x, (*w)->y, (*w)->w, (*w)->h, (*w)->r,
			  0.0, 1, (*w)->bgcol);
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

static int ismouseover(struct widget **w)
{
	struct area a;

	a.x = (*w)->x;
	a.y = (*w)->y;
	a.w = (*w)->w;
	a.h = (*w)->h;
	return ui_is_point_in_area(mouse, a);
}

static void draw_selection(struct widget **w)
{
	if (!ismouseover(w))
		return;
	if (!(*w)->action || (*w)->action[0] == '\0')
		return;
	ui_draw_rectangle((*w)->x, (*w)->y, (*w)->w, (*w)->h, (*w)->r,
			  0.0, 1, config.color_sel_bg);
	ui_draw_rectangle((*w)->x, (*w)->y, (*w)->w, (*w)->h, (*w)->r,
			  1.0, 0, config.color_sel_fg);
}

int widgets_mouseover(void)
{
	return mouseover;
}

void widgets_set_pointer_position(int x, int y)
{
	struct widget *w;

	mouseover = 0;
	mouse.x = x;
	mouse.y = y;

	list_for_each_entry(w, &widgets, list) {
		if (ismouseover(&w)) {
			mouseover = 1;
			break;
		}
	}
}

/*
 * widgets_set_point_position() should be run just before calling
 * this function
 */
char *widgets_get_mouseover_action(void)
{
	struct widget *w;

	list_for_each_entry(w, &widgets, list) {
		if (ismouseover(&w)) {
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
		draw_selection(&w);
		if (w->type == ICON)
			draw_icon(&w);
		else if (w->type == RECT)
			draw_rect(&w);
		else if (w->type == SEARCH)
			draw_search(&w);
		else
			warn("widget type not recognised");
	}
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
//	enum alignment halign;
//	enum alignment valign;
	parse_hexstr(argv_buf.argv[9], w->fgcol);
	parse_hexstr(argv_buf.argv[10], w->bgcol);
	w->content = argv_buf.argv[11];
	w->surface = NULL;
	list_add_tail(&w->list, &widgets);
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
