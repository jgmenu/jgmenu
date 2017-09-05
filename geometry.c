#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "geometry.h"

struct win {
	int menu_x0;			/*  g */
	int menu_y0;			/*  g */
	int menu_height;		/* sg */
	int menu_width;			/* sg */
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
static enum alignment menu_valign;
static enum alignment menu_halign;
static int sub_spacing;			/* s  */
static int item_margin_x;		/* s  */
static int item_margin_y;		/* s  */
static int item_height;			/* sg */

static int screen_width;
static int screen_height;		/*  g */
static int screen_x0;
static int screen_y0;

static void update_root(void)
{
	if (menu_halign == LEFT)
		win[cur].menu_x0 = menu_margin_x;
	else if (menu_halign == RIGHT)
		win[cur].menu_x0 = screen_width - win[cur].menu_width -
				   menu_margin_x;

	if (menu_valign == BOTTOM)
		win[cur].menu_y0 = screen_y0 + screen_height -
				   win[cur].menu_height - menu_margin_y;
	else if (menu_valign == TOP)
		win[cur].menu_y0 = screen_y0 + menu_margin_y;
}

static void update_sub_window(void)
{
	if (menu_halign == LEFT)
		win[cur].menu_x0 = win[cur - 1].menu_x0 +
				   win[cur - 1].menu_width +
				   sub_spacing;
	else if (menu_halign == RIGHT)
		win[cur].menu_x0 = win[cur - 1].menu_x0 -
				   win[cur].menu_width -
				   sub_spacing;

	/* We're assuming here that all submenu windows will be TOP aligned */
	if (cur) {
		int max_y0 = screen_height - menu_margin_y - win[cur].menu_height;

		win[cur].menu_y0 = win[cur - 1].menu_y0 +
				   win[cur - 1].parent_item.y -
				   item_margin_y -
				   menu_padding_top;
		/* FIXME: Needs to be sm_padding_top later */

		/* Do not go off the screen */
		if (win[cur].menu_y0 > max_y0) {
			if (menu_valign == TOP)
				win[cur].menu_y0 = screen_y0 + menu_margin_y;
			else
				win[cur].menu_y0 = max_y0;
		}
	} else {
		if (menu_valign == BOTTOM)
			win[cur].menu_y0 = screen_y0 + screen_height -
					   win[cur].menu_height -
					   menu_margin_y;
		else if (menu_valign == TOP)
			win[cur].menu_y0 = screen_y0 + menu_margin_y;
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

void geo_init(void)
{
	/*
	 * With the exception of the ui_get_screen_res() variables, the
	 * variables below will be changed in init_geo_variables_from_config()
	 * in jgmenu.c
	 */
	menu_margin_x = 1;
	menu_margin_y = 30;
	win[cur].menu_width = 200;
	win[cur].menu_height = 500;
	menu_padding_top = 5;
	menu_padding_right = 5;
	menu_padding_bottom = 5;
	menu_padding_left = 5;
	menu_valign = BOTTOM;
	menu_halign = LEFT;

	item_height = 20;
	item_margin_x = 4;
	item_margin_y = 4;

	ui_get_screen_res(&screen_x0, &screen_y0, &screen_width,
			  &screen_height);

	geo_update();
}

int geo_get_item_coordinates(struct area *a)
{
	static int h;

	/* This is how we reset it */
	if (!a) {
		h = 0;
		goto out;
	}
	if (!h)
		h = item_margin_y + menu_padding_top;
	a->y = h;
	a->x = menu_padding_left + item_margin_x;
	a->w = win[cur].menu_width - (item_margin_x * 2) - menu_padding_left -
	       menu_padding_right;
	h += a->h + item_margin_y;
out:
	return 0;
}

struct point geo_get_max_itemarea_that_fits(void)
{
	struct point p;

	p.x = screen_width - menu_margin_x - menu_padding_left -
	      menu_padding_right;
	p.y = screen_height - menu_margin_y - menu_padding_top -
	      menu_padding_bottom;
	return p;
}

struct point geo_get_max_menuarea_that_fits(void)
{
	struct point p;

	p.x = screen_width - menu_margin_x;
	p.y = screen_height - menu_margin_y;
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

void geo_set_menu_width_from_itemarea_width(int width)
{
	win[cur].menu_width = width + menu_padding_left + menu_padding_right;
	geo_update();
}

void geo_set_menu_height(int h)
{
	win[cur].menu_height = h;
	geo_update();
}

void geo_set_menu_height_from_itemarea_height(int h)
{
	win[cur].menu_height = h + menu_padding_top + menu_padding_bottom;
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
	menu_halign = pos;
	geo_update();
}

void geo_set_menu_valign(enum alignment pos)
{
	menu_valign = pos;
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
	return win[cur].menu_height - menu_padding_top - menu_padding_bottom;
}

int geo_get_menu_width(void)
{
	return win[cur].menu_width;
}

int geo_get_menu_width_from_itemarea_width(int width)
{
	return width + menu_padding_right + menu_padding_left;
}

int geo_get_item_height(void)
{
	return item_height;
}

int geo_get_screen_height(void)
{
	return screen_height;
}

int geo_get_screen_width(void)
{
	return screen_width;
}
