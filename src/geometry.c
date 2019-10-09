#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "geometry.h"
#include "util.h"
#include "config.h"

struct win {
	int menu_x0;			/*  g */
	int menu_y0;			/*  g */
	int menu_height;		/* sg */
	int menu_width;			/* sg */
	enum alignment menu_valign;	/* s  */
	enum alignment menu_halign;	/* s  */
	struct area parent_item;
};

static struct win win[MAX_NR_WINDOWS];
static int cur;

static int menu_margin_x;		/* s  */
static int menu_margin_y;		/* s  */
static int menu_padding_top;		/* s  */
static int menu_padding_right;		/* s  */
static int menu_padding_bottom;		/* s  */
static int menu_padding_left;		/* s  */
static int sub_spacing;			/* s  */
static int sub_padding_top;		/* s  */
static int sub_padding_right;		/* s  */
static int sub_padding_bottom;		/* s  */
static int sub_padding_left;		/* s  */
static int item_margin_x;		/* s  */
static int item_margin_y;		/* s  */
static int item_height;			/* sg */

static int screen_width;
static int screen_height;		/*  g */
static int screen_x0;
static int screen_y0;

static void update_root(void)
{
	if (win[cur].menu_halign == LEFT)
		win[cur].menu_x0 = menu_margin_x;
	else if (win[cur].menu_halign == RIGHT)
		win[cur].menu_x0 = screen_width - win[cur].menu_width -
				   menu_margin_x;
	else if (win[cur].menu_halign == CENTER)
		win[cur].menu_x0 = (screen_width - win[cur].menu_width) / 2;
	if (config.position_mode != POSITION_MODE_PTR &&
	    config.position_mode != POSITION_MODE_IPC)
		win[cur].menu_x0 += screen_x0;

	if (win[cur].menu_valign == BOTTOM)
		win[cur].menu_y0 = screen_height - win[cur].menu_height -
				   menu_margin_y;
	else if (win[cur].menu_valign == TOP)
		win[cur].menu_y0 = menu_margin_y;
	else if (win[cur].menu_valign == CENTER)
		win[cur].menu_y0 = (screen_height - win[cur].menu_height) / 2;
	if (config.position_mode != POSITION_MODE_PTR &&
	    config.position_mode != POSITION_MODE_IPC)
		win[cur].menu_y0 += screen_y0;
}

static void left_align(void)
{
	win[cur].menu_halign = LEFT;
	win[cur].menu_x0 = win[cur - 1].menu_x0 + win[cur - 1].menu_width +
			   sub_spacing;
}

static void right_align(void)
{
	win[cur].menu_halign = RIGHT;
	win[cur].menu_x0 = win[cur - 1].menu_x0 - win[cur].menu_width -
			   sub_spacing;
}

static void left_align_and_adjust(void)
{
	int max_x0 = screen_x0 + screen_width - win[cur].menu_width;

	left_align();
	if (win[cur].menu_x0 > max_x0)
		win[cur].menu_x0 = max_x0;
}

static void right_align_and_adjust(void)
{
	right_align();
	if (win[cur].menu_x0 < screen_x0)
		win[cur].menu_x0 = screen_x0;
}

static void update_sub_window(void)
{
	BUG_ON(!cur);
	if (win[cur].menu_valign == TOP ||
	    win[cur].menu_valign == CENTER) {
		int max_y0 = screen_y0 + screen_height - win[cur].menu_height;

		win[cur].menu_y0 = win[cur - 1].menu_y0 +
				   win[cur - 1].parent_item.y -
				   item_margin_y -
				   sub_padding_top;
		if (win[cur].menu_y0 > max_y0)
			win[cur].menu_y0 = max_y0;
	} else if (win[cur].menu_valign == BOTTOM) {
		win[cur].menu_y0 = win[cur - 1].menu_y0 +
				   win[cur - 1].parent_item.y +
				   win[cur - 1].parent_item.h +
				   item_margin_y +
				   sub_padding_bottom -
				   win[cur].menu_height;
		if (win[cur].menu_y0 < screen_y0)
			win[cur].menu_y0 = screen_y0;
	}

	if (win[cur].menu_halign == LEFT ||
	    win[cur].menu_halign == CENTER) {
		left_align();
		if (screen_x0 + screen_width - win[cur].menu_width <
		    win[cur].menu_x0)
			right_align_and_adjust();
	} else if (win[cur].menu_halign == RIGHT) {
		right_align();
		if (screen_x0 > win[cur].menu_x0)
			left_align_and_adjust();
	}
}

/* Update window's (x,y) co-ordinates */
static void geo_update(void)
{
	if (!cur)
		update_root();
	else
		update_sub_window();
}

void geo_update_monitor_coords(void)
{
	ui_get_screen_res(&screen_x0, &screen_y0, &screen_width,
			  &screen_height, config.monitor);
	geo_update();
}

void geo_init(void)
{
	/*
	 * With the exception of the ui_get_screen_res() variables, the
	 * variables below will be changed in init_geo_variables_from_config()
	 * in jgmenu.c
	 */
	menu_margin_x = 0;
	menu_margin_y = 0;
	win[cur].menu_width = 200;
	win[cur].menu_height = 500;
	menu_padding_top = 5;
	menu_padding_right = 5;
	menu_padding_bottom = 5;
	menu_padding_left = 5;
	win[cur].menu_valign = BOTTOM;
	win[cur].menu_halign = LEFT;

	item_height = 20;
	item_margin_x = 4;
	item_margin_y = 4;

	geo_update_monitor_coords();
}

static int get_available_width_of_one_column(void)
{
	int w;

	/* calculate available width for all columns */
	w = !cur ? win[cur].menu_width - (item_margin_x * 2) -
	       menu_padding_left - menu_padding_right :
	       win[cur].menu_width - (item_margin_x * 2) -
	       sub_padding_left - sub_padding_right;

	/* We do not support columns in sub-windows */
	if (cur)
		return w;

	/* calculate available width for each column */
	if (config.columns > 1)
		w = w / config.columns - (config.columns - 1) * item_margin_x;
	return w;
}

int geo_get_item_coordinates(struct area *a)
{
	static int h;
	static int item_width, col;
	int pad_top, pad_bottom, pad_left;

	/* This is how we reset it */
	if (!a) {
		h = 0;
		col = 1;
		item_width = 0;
		goto out;
	}
	if (!item_width)
		item_width = get_available_width_of_one_column();
	pad_top = !cur ? menu_padding_top : sub_padding_top;
	pad_left = !cur ? menu_padding_left : sub_padding_left;
	pad_bottom = !cur ? menu_padding_bottom : sub_padding_bottom;
	/* move to next column */
	if (h > win[cur].menu_height - a->h - item_margin_y - pad_bottom &&
	    !cur) {
		h = 0;
		col++;
	}
	if (!h)
		h = item_margin_y + pad_top;
	a->y = h;
	a->w = item_width;
	a->x = item_margin_x + pad_left;
	if (!cur && col > 1)
		a->x += (col - 1) * (item_width + item_margin_x);
	h += a->h + item_margin_y;
out:
	return 0;
}

struct point geo_get_max_itemarea_that_fits(void)
{
	struct point p;

	p.x = !cur ? screen_x0 + screen_width - menu_margin_x -
	      menu_padding_left - menu_padding_right :
	      screen_x0 + screen_width - menu_margin_x - sub_padding_left -
	      sub_padding_right;
	p.y = !cur ? screen_y0 + screen_height - menu_margin_y -
	      menu_padding_top - menu_padding_bottom :
	      screen_y0 + screen_height - sub_padding_top - sub_padding_bottom;
	return p;
}

struct point geo_get_max_menuarea_that_fits(void)
{
	struct point p;

	p.x = screen_x0 + screen_width - menu_margin_x;
	p.y = screen_y0 + screen_height - menu_margin_y;
	return p;
}

/*********************************************************************/

int geo_cur(void)
{
	return cur;
}

void geo_set_cur(int c)
{
	cur = c;
}

void geo_win_add(struct area parent_item)
{
	win[cur].parent_item.x = parent_item.x;
	win[cur].parent_item.y = parent_item.y;
	win[cur].parent_item.w = parent_item.w;
	win[cur].parent_item.h = parent_item.h;
	cur++;
	win[cur].menu_valign = win[cur - 1].menu_valign;
	win[cur].menu_halign = win[cur - 1].menu_halign;
	geo_update();
}

void geo_win_del(void)
{
	if (cur < 1)
		fprintf(stderr, "%s:%d - %s:  cannot delete root window\n",
			__FILE__, __LINE__, __func__);
	cur--;
}

/*********************************************************************/

void geo_set_menu_width(int width)
{
	win[cur].menu_width = width;
	geo_update();
}

void geo_set_menu_height(int h)
{
	win[cur].menu_height = h;
	geo_update();
}

void geo_set_menu_height_from_itemarea_height(int h)
{
	win[cur].menu_height = !cur ? h + menu_padding_top + menu_padding_bottom :
			       h + sub_padding_top + sub_padding_bottom;
	geo_update();
}

void geo_set_menu_margin_x(int x)
{
	menu_margin_x = x;
	geo_update();
}

void geo_set_menu_margin_y(int y)
{
	menu_margin_y = y;
	geo_update();
}

void geo_set_menu_halign(enum alignment pos)
{
	win[cur].menu_halign = pos;
	geo_update();
}

void geo_set_menu_valign(enum alignment pos)
{
	win[cur].menu_valign = pos;
	geo_update();
}

void geo_set_item_height(int h)
{
	item_height = h;
}

void geo_set_sub_spacing(int spacing)
{
	sub_spacing = spacing;
}

void geo_set_sub_padding_top(int padding)
{
	sub_padding_top = padding;
}

void geo_set_sub_padding_right(int padding)
{
	sub_padding_right = padding;
}

void geo_set_sub_padding_bottom(int padding)
{
	sub_padding_bottom = padding;
}

void geo_set_sub_padding_left(int padding)
{
	sub_padding_left = padding;
}

void geo_set_item_margin_x(int margin)
{
	item_margin_x = margin;
}

void geo_set_item_margin_y(int margin)
{
	item_margin_y = margin;
}

void geo_set_menu_padding_top(int padding)
{
	menu_padding_top = padding;
}

void geo_set_menu_padding_right(int padding)
{
	menu_padding_right = padding;
}

void geo_set_menu_padding_bottom(int padding)
{
	menu_padding_bottom = padding;
}

void geo_set_menu_padding_left(int padding)
{
	menu_padding_left = padding;
}

/*********************************************************************/

int geo_get_menu_x0(void)
{
	return win[cur].menu_x0;
}

int geo_get_menu_y0(void)
{
	return win[cur].menu_y0;
}

int geo_get_menu_height(void)
{
	return win[cur].menu_height;
}

int geo_get_itemarea_height(void)
{
	return !cur ? win[cur].menu_height - menu_padding_top -
	       menu_padding_bottom :
	       win[cur].menu_height - sub_padding_top - sub_padding_bottom;
}

int geo_get_menu_width(void)
{
	return win[cur].menu_width;
}

int geo_get_menu_width_from_itemarea_width(int width)
{
	return !cur ? width + menu_padding_right + menu_padding_left :
	       width + sub_padding_right + sub_padding_left;
}

int geo_get_screen_x0(void)
{
	return screen_x0;
}

int geo_get_screen_y0(void)
{
	return screen_y0;
}

int geo_get_screen_height(void)
{
	return screen_height;
}

int geo_get_screen_width(void)
{
	return screen_width;
}
