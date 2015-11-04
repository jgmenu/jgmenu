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
 */

#ifndef GEOMETRY_H
#define GEOMETRY_H

void geo_update(void);
void geo_init(void);

void geo_set_menu_width(int w);
void geo_set_menu_height(int h);
void geo_set_menu_margin_x(int x);
void geo_set_menu_margin_y(int y);

int geo_get_menu_x0(void);
int geo_get_menu_y0(void);
int geo_get_menu_height(void);
int geo_get_menu_width(void);

#endif  /* GEOMETRY_H */
