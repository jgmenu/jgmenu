#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "geometry.h"

int menu_margin_x;	/* s  */
int menu_margin_y;	/* s  */
int menu_height;	/* sg */
int menu_width;		/* sg */
int menu_padding_top;	/* s  */
int menu_padding_right;	/* s  */
int menu_padding_bottom;/* s  */
int menu_padding_left;	/* s  */
int menu_x0;		/*  g */
int menu_y0;		/*  g */

enum alignment menu_valign;
enum alignment menu_halign;

int item_margin_x;	/* s  */
int item_margin_y;	/* s  */

int screen_width;
int screen_height;	/*  g */
int screen_x0;
int screen_y0;

int item_height;	/* sg */

void geo_update(void)
{
	if (menu_halign == LEFT)
		menu_x0 = menu_margin_x - 1;
	else if (menu_halign == RIGHT)
		menu_x0 = screen_width - menu_width - menu_margin_x - 1;

	if (menu_valign == BOTTOM)
		menu_y0 = screen_y0 + screen_height - menu_height -
			  menu_margin_y - 1;
	else if (menu_valign == TOP)
		menu_y0 = screen_y0 + menu_margin_y - 1;
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
	menu_width = 200;
	menu_height = 500;
	menu_padding_top = 10;
	menu_padding_right = 10;
	menu_padding_bottom = 10;
	menu_padding_left = 10;
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
	a->w = menu_width - (item_margin_x * 2) - menu_padding_left -
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

void geo_set_menu_width(int w)
{
	menu_width = w;
	geo_update();
}

void geo_set_menu_width_from_itemarea_width(int w)
{
	menu_width = w + menu_padding_left + menu_padding_right;
	geo_update();
}

void geo_set_menu_height(int h)
{
	menu_height = h;
	geo_update();
}

void geo_set_menu_height_from_itemarea_height(int h)
{
	menu_height = h + menu_padding_top + menu_padding_bottom;
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
	geo_update();
}

void geo_set_item_margin_x(int margin)
{
	item_margin_x = margin;
	geo_update();
}

void geo_set_item_margin_y(int margin)
{
	item_margin_y = margin;
	geo_update();
}

void geo_set_menu_padding_top(int padding)
{
	menu_padding_top = padding;
	geo_update();
}

void geo_set_menu_padding_right(int padding)
{
	menu_padding_right = padding;
	geo_update();
}

void geo_set_menu_padding_bottom(int padding)
{
	menu_padding_bottom = padding;
	geo_update();
}

void geo_set_menu_padding_left(int padding)
{
	menu_padding_left = padding;
	geo_update();
}

/*********************************************************************/

int geo_get_menu_x0(void)
{
	return menu_x0;
}

int geo_get_menu_y0(void)
{
	return menu_y0;
}

int geo_get_menu_height(void)
{
	return menu_height;
}

int geo_get_itemarea_height(void)
{
	return menu_height - menu_padding_top - menu_padding_bottom;
}

int geo_get_menu_width(void)
{
	return menu_width;
}

int geo_get_menu_width_from_itemarea_width(int w)
{
	return w + menu_padding_right + menu_padding_left;
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
