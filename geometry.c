#include <stdio.h>
#include <stdlib.h>

#include "x11-ui.h"
#include "geometry.h"

enum Alignment {TOP, CENTER, BOTTOM, LEFT, RIGHT};

enum Alignment menu_valign;
enum Alignment menu_halign;

int menu_margin_x;
int menu_margin_y;
int menu_height;	/* need to be able to change this in real time */
int menu_width;
int menu_x0;		/* menu_x0 and menu_y0 - commonly called */
int menu_y0;

int screen_width;
int screen_height;
int screen_x0;
int screen_y0;


/*
 * geo_update() contains all the algorithms.
 *
 * all the others functions are getters and setters
 */
void geo_update(void)
{
	menu_x0 = menu_margin_x;

	menu_y0 = screen_y0 + screen_height - menu_height - menu_margin_y;
}

void geo_init(void)
{
	menu_margin_x = 1;
	menu_margin_y = 30;
	menu_width = 200;
	menu_height = 500;

	ui_get_screen_res(&screen_x0, &screen_y0, &screen_width,
			  &screen_height);

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

