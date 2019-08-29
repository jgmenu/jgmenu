#include <stdio.h>
#include <stdlib.h>

#include "workarea.h"
#include "x11-ui.h"
#include "util.h"
#include "geometry.h"
#include "config.h"
#include "align.h"

static enum alignment panel_pos = UNKNOWN;
static int margin_x;
static int margin_y;

/*
 * WMs that support _NET_WORKAREA include (but are not limited to):
 *   - openbox, xfwm4
 *
 * These WMs do not:
 *   - awesome, i3, bspwm
 */
static void workarea_init(void)
{
	static int done;
	int ret;
	struct area wa = { 0, 0, 0, 0 };
	struct area sc = { 0, 0, 0, 0 };

	if (done)
		return;
	done = 1;

	sc.x = geo_get_screen_x0();
	sc.y = geo_get_screen_y0();
	sc.w = geo_get_screen_width();
	sc.h = geo_get_screen_height();

	ret = ui_get_workarea(&wa);
	if (ret < 0)
		return;
	if (config.verbosity >= 2) {
		info("screen:        (%d,%d,%d,%d)", sc.x, sc.y, sc.w, sc.h);
		info("_NET_WORKAREA: (%d,%d,%d,%d)", wa.x, wa.y, wa.w, wa.h);
	}

	if (wa.y && sc.h != wa.y + wa.h) {
		if (config.verbosity < 2)
			return;
		info("_NET_WORKAREA: panel @ 'top' and 'bottom' - do nothing");
	} else if (sc.y != wa.y) {
		panel_pos = TOP;
		margin_y = wa.y;
		if (config.verbosity < 2)
			return;
		info("_NET_WORKAREA: panel @ 'top'; margin=%d", margin_y);
	} else if (!wa.y && sc.h != wa.h) {
		panel_pos = BOTTOM;
		margin_y = sc.h - wa.h;
		if (config.verbosity < 2)
			return;
		info("_NET_WORKAREA: panel @ 'bottom'; margin=%d", margin_y);
	} else if (wa.x && sc.w != wa.x + wa.w) {
		if (config.verbosity < 2)
			return;
		info("_NET_WORKAREA: panel @ 'right' and 'left' - do nothing");
	} else if (sc.x != wa.x) {
		panel_pos = LEFT;
		margin_x = wa.x;
		if (config.verbosity < 2)
			return;
		info("_NET_WORKAREA: panel @ 'left'; margin=%d", margin_x);
	} else if (!wa.x && sc.w != wa.w) {
		panel_pos = RIGHT;
		margin_x = sc.w - wa.w;
		if (config.verbosity < 2)
			return;
		info("_NET_WORKAREA: panel @ 'right'; margin=%d", margin_x);
	}
}

void workarea_set_margin(void)
{
	workarea_init();
	if (margin_y) {
		config.menu_margin_y = margin_y;
		if (config.verbosity >= 2)
			info("margin_y = %d", margin_y);
	}
	if (margin_x) {
		config.menu_margin_x = margin_x;
		if (config.verbosity >= 2)
			info("margin_x = %d", margin_x);
	}
}

void workarea_set_panel_pos(void)
{
	workarea_init();
	if (panel_pos == UNKNOWN)
		return;
	if (panel_pos == TOP || panel_pos == BOTTOM) {
		config.menu_valign = panel_pos;
		if (config.verbosity >= 2)
			info("valign has been set");
	}
	if (panel_pos == LEFT || panel_pos == RIGHT) {
		config.menu_halign = panel_pos;
		if (config.verbosity >= 2)
			info("halign has been set");
	}
}
