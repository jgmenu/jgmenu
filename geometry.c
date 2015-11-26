#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "x11-ui.h"
#include "geometry.h"

enum Alignment {TOP, CENTER, BOTTOM, LEFT, RIGHT};

enum Alignment menu_valign;
enum Alignment menu_halign;

int menu_margin_x;	/* s  */
int menu_margin_y;	/* s  */
int menu_height;	/* sg */
int menu_width;		/* sg */
int menu_x0;		/* g  */
int menu_y0;		/* g  */

int screen_width;
int screen_height;
int screen_x0;
int screen_y0;

int nr_items;
int show_title;

int item_height;
char item_font[1024];

/*
 * geo_update() contains algorithms for geometric variables
 *
 * item_height is not recalculated as it's too slow
 */
void geo_update(void)
{
	menu_x0 = menu_margin_x;

	menu_height = nr_items * item_height;
	if (show_title)
		menu_height = menu_height + item_height;

	menu_y0 = screen_y0 + screen_height - menu_height - menu_margin_y;

}

void geo_init(void)
{
	menu_margin_x = 1;
	menu_margin_y = 30;
	menu_width = 200;
	menu_height = 500;

	item_height = 20;
	strncpy(item_font, "Sans 18px", 9);
	show_title = 0;

	nr_items = 1;

	ui_get_screen_res(&screen_x0, &screen_y0, &screen_width,
			  &screen_height);

	geo_set_item_height(item_height);
	geo_update();
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
	int font_height;

	font_height =  ui_get_text_height(item_font);
	item_height = h > font_height ? h : font_height;
	geo_update();
}


void geo_set_font(char *font)
{
	strncpy(item_font, font, strlen(font));
	geo_set_item_height(item_height);
	geo_update();
}

void geo_set_nr_items(int nr)
{
	nr_items = nr;
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

int geo_get_font_height(void)
{
	return ui_get_text_height(item_font);
}
