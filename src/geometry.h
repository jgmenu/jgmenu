/*
 * geometry.h contains algorithms for calculating menu co-ordinates.
 *
 * The program window has no border or padding, so the menu_* variables
 * can be thought of as window_* variables. In fact, when the X11 window
 * is created/resized/moved the menu_margin_x, etc are used.
 *
 * Definitions:
 *			screen
 *	+---------------------------------------+
 *	|					|
 *	|	   menu				|
 *	|	+--------+			|
 *	|	|  item	 |			|
 *	|	| +----+ |			|
 *	|	| |    | |			|
 *	|	| +----+ |			|
 *	|	|	 |			|
 *	|	| +----+ |			|
 *	|	| |    | |			|
 *	|	| +----+ |			|
 *	|	|	 |			|
 *	|	+--------+			|
 *	+---------------------------------------+
 *
 * "margin" is space outside an object.
 * "padding" is space inside an object (between border and content).
 * (x0, y0) represents the top left corner of the screen/menu/item.
 * (x1, y1) represents the bottom right corner of the screen/menu/item.
 *
 * The menu_margin_* is the distance between the X11 window and the edge
 * of the screen
 *
 * "itemarea" refers to the area occupied by menu-items (incl item_margin*)
 *
 * Usage:
 * ------
 *   - Init X11 (XOpenDisplay, etc)
 *   - Call geo_init()
 *   - All the geo_get_* can now be used (default values set up geo_init)
 *   - Set variables using the geo_set_* functions.
 */

#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "x11-ui.h"
#include "align.h"

void geo_update_monitor_coords(void);
void geo_init(void);
int geo_get_item_coordinates(struct area *a);
struct point geo_get_max_itemarea_that_fits(void);
struct point geo_get_max_menuarea_that_fits(void);

int geo_cur(void);
void geo_set_cur(int c);
void geo_win_add(struct area parent_item);
void geo_win_del(void);

void geo_set_menu_width(int width);
void geo_set_menu_height(int h);
void geo_set_menu_height_from_itemarea_height(int h);
void geo_set_menu_margin_x(int x);
void geo_set_menu_margin_y(int y);
void geo_set_menu_halign(enum alignment pos);
void geo_set_menu_valign(enum alignment pos);
void geo_set_item_height(int h);
void geo_set_sub_spacing(int spacing);
void geo_set_sub_padding_top(int padding);
void geo_set_sub_padding_right(int padding);
void geo_set_sub_padding_bottom(int padding);
void geo_set_sub_padding_left(int padding);
void geo_set_item_margin_x(int margin);
void geo_set_item_margin_y(int margin);
void geo_set_menu_padding_top(int padding);
void geo_set_menu_padding_right(int padding);
void geo_set_menu_padding_bottom(int padding);
void geo_set_menu_padding_left(int padding);

int geo_get_menu_x0(void);
int geo_get_menu_y0(void);
int geo_get_menu_height(void);
int geo_get_itemarea_height(void);
int geo_get_menu_width(void);
int geo_get_menu_width_from_itemarea_width(int width);
int geo_get_screen_x0(void);
int geo_get_screen_y0(void);
int geo_get_screen_height(void);
int geo_get_screen_width(void);

#endif  /* GEOMETRY_H */
