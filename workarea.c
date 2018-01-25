#include <stdio.h>
#include <stdlib.h>

#include "workarea.h"
#include "x11-ui.h"
#include "util.h"
#include "geometry.h"

/*
 * Support _NET_WORKAREA: openbox, xfwm4
 * Do not support:        awesome, i3, bspwm
 */
void workarea_set(void)
{
	struct area a = { 0, 0, 0, 0 };

	if (ui_get_workarea(&a) < 0) {
		info("your WM does not support _NET_WORKAREA");
		return;
	}
	info("screen:        (%d,%d,%d,%d)", geo_get_screen_x0(),
	     geo_get_screen_y0(), geo_get_screen_width(),
	     geo_get_screen_height());
	geo_set_screen_area(a);
	info("_NET_WORKAREA: (%d,%d,%d,%d)", a.x, a.y, a.w, a.h);
}
