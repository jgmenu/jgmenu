#include <stdio.h>
#include <stdlib.h>

#include "workarea.h"
#include "x11-ui.h"
#include "util.h"
#include "geometry.h"

static void print_work_area(void)
{
	struct area a = { 0, 0, 0, 0 };

	/*
	 * Support _NET_WORKAREA: openbox, xfwm4
	 * Do not support:        awesome, i3, bspwm
	 */
	if (ui_get_workarea(&a) < 0) {
		info("your WM does not support _NET_WORKAREA");
		return;
	}
	info("_NET_WORKAREA: (%d,%d,%d,%d)", a.x, a.y, a.w, a.h);
}

void workarea_set(void)
{
	info("screen:        (%d,%d,%d,%d)", geo_get_screen_x0(),
	     geo_get_screen_y0(), geo_get_screen_width(),
	     geo_get_screen_height());
	print_work_area();
}
