#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "geometry.h"

enum Alignment {TOP, CENTER, BOTTOM, LEFT, RIGHT};

enum Alignment menu_valign;
enum Alignment menu_halign;

int menu_margin_x;	/* s  */
int menu_margin_y;	/* s  */
int menu_height;	/* sg */
int menu_width;		/* sg */
int menu_x0;		/*  g */
int menu_y0;		/*  g */

int item_margin_x;	/* s  */
int item_margin_y;	/* s  */

int screen_width;
int screen_height;	/*  g */
int screen_x0;
int screen_y0;

int nr_visible_items;	/* sg */
int show_title;		/* s  */

int item_height;	/* sg */

/*
 * geo_update() contains algorithms for geometric variables
 */
void geo_update(void)
{
	menu_x0 = menu_margin_x;

	menu_height = nr_visible_items * (item_height + item_margin_y) + item_margin_y;

	if (show_title)
		menu_height = menu_height + item_height;

	menu_y0 = screen_y0 + screen_height - menu_height - menu_margin_y;
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

	item_height = 20;
	item_margin_x = 4;
	item_margin_y = 4;

	show_title = 0;
	nr_visible_items = 1;

	ui_get_screen_res(&screen_x0, &screen_y0, &screen_width,
			  &screen_height);

	geo_update();
}

/*
 * item_number is the sequential number of the item starting with zero
 */
struct Area geo_get_item_coordinates(int item_number)
{
	struct Area a;

	a.x = 0 + item_margin_x;
	a.y = item_number * (item_height + item_margin_y) + item_margin_y;
	if (show_title)
		a.y -= item_margin_y;

	a.h = item_height;
	a.w = menu_width - (item_margin_x * 2);

	return a;
}

int geo_get_nr_items_that_fit_on_screen(void)
{
	int nr_items, h;

	h = screen_height - menu_margin_y - item_margin_y;

	if (show_title)
		h = h - item_height;

	nr_items = h / (item_height + item_margin_y);

	return nr_items;
}

/*********************************************************************/

void geo_set_menu_width(int w)
{
	menu_width = w;
	geo_update();
}

void geo_set_menu_height(int h)
{
	menu_height = h;
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

void geo_set_item_height(int h)
{
	item_height = h;
	geo_update();
}

void geo_set_nr_visible_items(int nr)
{
	if (nr > geo_get_nr_items_that_fit_on_screen())
		nr_visible_items = geo_get_nr_items_that_fit_on_screen();
	else
		nr_visible_items = nr;
	geo_update();
}

void geo_set_show_title(char *s)
{
	if (s)
		show_title = 1;
	else
		show_title = 0;
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

int geo_get_menu_width(void)
{
	return menu_width;
}

int geo_get_item_height(void)
{
	return item_height;
}

int geo_get_screen_height(void)
{
	return screen_height;
}

int geo_get_nr_visible_items(void)
{
	return nr_visible_items;
}
