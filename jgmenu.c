/*
 * jgmenu.c
 *
 * Copyright (C) Johan Malm 2014
 *
 * jgmenu is a stand-alone menu which reads the menu items from stdin
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>
#include <X11/Xlocale.h>
#include <pthread.h>
#include <sys/select.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <math.h>
#include <sys/stat.h>
#include <time.h>

#include "x11-ui.h"
#include "config.h"
#include "util.h"
#include "geometry.h"
#include "isprog.h"
#include "sbuf.h"
#include "icon.h"
#include "filter.h"
#include "list.h"
#include "lockfile.h"
#include "argv-buf.h"
#include "t2conf.h"
#include "t2env.h"
#include "bl.h"
#include "xsettings-helper.h"
#include "terminal.h"
#include "restart.h"
#include "theme.h"
#include "font.h"

#define DEBUG_ICONS_LOADED_NOTIFICATION 0

static pthread_t thread;	   /* worker thread for loading icons	  */
static int pipe_fds[2];		   /* talk between threads + catch sig    */
static int die_when_loaded;	   /* Used for performance testing	  */

struct item {
	char *name;
	char *cmd;
	char *iconname;
	char *tag;
	struct area area;
	cairo_surface_t *icon;
	int selectable;
	struct list_head master;
	struct list_head filter;
	struct list_head list;	   /* submenu list under each node	  */
};

static struct item empty_item;

/* A node is marked by a ^tag() and denotes the start of a submenu */
struct node {
	char *tag;
	struct item *item;	   /* item that node points to		  */
	struct item *last_sel;	   /* used when returning to node	  */
	struct node *parent;
	struct list_head items;	   /* menu-items having off node	  */
	struct list_head node;
};

/*
 * The lists "master" and "filter" are the core building blocks of the code
 * When a submenu is checked out, *subhead and *subtail are set.
 */
struct menu {
	struct item *head;	   /* first item in linked list		  */
	struct item *subhead;	   /* first item in checked out submenu	  */
	struct item *subtail;	   /* last item in checked out submenu	  */
	struct item *first;	   /* first visible item			  */
	struct item *last;	   /* last visible item			  */
	struct item *sel;	   /* currently selected item		  */
	struct list_head master;   /* all items				  */
	struct list_head filter;   /* items to be displayed in menu	  */
	struct list_head nodes;	   /* hierarchical structure of tags	  */
	struct node *current_node;
};

struct menu menu;

static const char jgmenu_usage[] =
"Usage: jgmenu [OPTIONS]\n"
"    --version             show version\n"
"    --no-spawn            redirect command to stdout instead of executing\n"
"    --checkout=<tag>      checkout submenu <tag> on startup\n"
"    --config-file=<file>  specify config file\n"
"    --icon-size=<size>    specify icon size (22 by default); set to 0 to\n"
"                          disable icons\n"
"    --at-pointer          launch menu at mouse pointer\n"
"    --hide-on-startup     start menu is hidden state\n"
"    --simple              ignore tint2 settings and run in 'short-lived' mode\n"
"                          (i.e. exit after mouse click or enter/escape)\n"
"    --vsimple             same as --simple, but also disables icons and\n"
"                          ignores jgmenurc\n"
"    --csv-file=<file>     specify menu file (in jgmenu flavoured CSV format)\n"
"    --csv-cmd=<command>   specify command to producue menu data\n";

void init_empty_item(void)
{
	empty_item.name = strdup("<empty>");
	empty_item.cmd = strdup(":");
	empty_item.iconname = NULL;
	empty_item.tag = NULL;
	empty_item.icon = NULL;
	empty_item.selectable = 1;
	empty_item.area.h = config.item_height;
}

void usage(void)
{
	printf("%s", jgmenu_usage);
	exit(0);
}

struct item *filter_head(void)
{
	return list_first_entry_or_null(&menu.filter, struct item, filter);
}

struct item *filter_tail(void)
{
	return list_last_entry(&menu.filter, struct item, filter);
}

void step_back(struct item **ptr, int nr)
{
	int i;

	for (i = 0; i < nr; i++)
		*ptr = container_of((*ptr)->filter.prev, struct item, filter);
}

void step_fwd(struct item **ptr, int nr)
{
	int i;

	for (i = 0; i < nr; i++)
		*ptr = container_of((*ptr)->filter.next, struct item, filter);
}

struct item *fill_from_top(struct item *first)
{
	struct item *p, *last;
	int h = geo_get_itemarea_height();

	h -= config.item_margin_y;
	p = first;
	last = p;
	list_for_each_entry_from(p, &menu.filter, filter) {
		h -= p->area.h + config.item_margin_y;
		if (h < 0)
			break;
		last = p;
	}
	return last;
}

struct item *fill_from_bottom(struct item *last)
{
	struct item *p, *first;
	int h = geo_get_itemarea_height();
	int ignoring = 1;

	h -= config.item_margin_y;
	first = last;
	list_for_each_entry_reverse(p, &menu.filter, filter) {
		if (p == last)
			ignoring = 0;
		if (ignoring)
			continue;
		h -= p->area.h + config.item_margin_y;
		if (h < 0)
			break;
		first = p;
	}
	return first;
}

struct item *next_selectable(struct item *cur, int *isoutside)
{
	struct item *p = cur;

	*isoutside = 0;
	while (p != filter_tail()) {
		if (p == menu.last)
			*isoutside = 1;
		step_fwd(&p, 1);
		if (p->selectable)
			break;
	}
	if (!p->selectable)
		p = cur;
	return p;
}

struct item *prev_selectable(struct item *cur, int *isoutside)
{
	struct item *p = cur;

	*isoutside = 0;
	while (p != filter_head()) {
		if (p == menu.first)
			*isoutside = 1;
		step_back(&p, 1);
		if (p->selectable)
			break;
	}
	if (!p->selectable)
		p = cur;
	return p;
}

void add_if_unique(struct item *item)
{
	struct item *p;

	if (!item->cmd)
		return;
	list_for_each_entry(p, &menu.filter, filter) {
		if (!p->cmd)
			continue;
		if (!strcmp(item->cmd, p->cmd))
			return;
	}
	list_add_tail(&item->filter, &menu.filter);
}

int isvisible(struct item *item)
{
	struct item *p;

	p = menu.first;
	list_for_each_entry_from(p, &menu.filter, filter) {
		if (p == item)
			return 1;
		if (p == menu.last)
			break;
	}
	return 0;
}

void update_filtered_list(void)
{
	struct item *item;
	int isoutside;

	INIT_LIST_HEAD(&menu.filter);

	if (filter_needle_length()) {
		list_for_each_entry(item, &menu.master, master) {
			if (!strncmp("^checkout(", item->cmd, 10) ||
			    !strncmp("^tag(", item->cmd, 5) ||
			    !strncmp("^back(", item->cmd, 6))
				continue;
			if (filter_ismatch(item->name) ||
			    filter_ismatch(item->cmd))
				add_if_unique(item);
		}
	} else {
		list_for_each_entry(item, &menu.master, master)
			if (item == menu.subhead)
				break;
		list_for_each_entry_from(item, &menu.master, master) {
			list_add_tail(&item->filter, &menu.filter);
			if (item == menu.subtail)
				break;
		}
	}

	if (!filter_head()) {
		list_add_tail(&empty_item.filter, &menu.filter);
		menu.first = &empty_item;
		menu.last = &empty_item;
		menu.sel = &empty_item;
		return;
	}

	menu.first = filter_head();
	menu.last = fill_from_top(menu.first);

	/* select an item */
	if (!filter_needle_length() && menu.current_node->last_sel) {
		menu.sel = menu.current_node->last_sel;
		menu.current_node->last_sel = NULL;
		if (!isvisible(menu.sel))
			menu.sel = menu.first;
	} else {
		menu.sel = menu.first;
		if (!menu.sel->selectable)
			menu.sel = next_selectable(menu.first, &isoutside);
	}
}

void init_menuitem_coordinates(void)
{
	struct item *p;

	if (list_empty(&menu.filter))
		return;

	/* reset y variable */
	geo_get_item_coordinates(NULL);

	p = menu.first;
	list_for_each_entry_from(p, &menu.filter, filter) {
		geo_get_item_coordinates(&p->area);
		if (p == menu.last)
			break;
	}
}

/*
 * Returns bar from ^foo(bar)
 *   s="^foo(bar)"
 *   token="^foo("
 */
char *parse_caret_action(char *s, char *token)
{
	char *p, *q;

	p = NULL;
	q = NULL;

	if (!s)
		return NULL;

	if (!strncmp(s, token, strlen(token))) {
		p = strdup(s);
		p += strlen(token);
		q = strchr(p, ')');
		if (q)
			*q = '\0';
	}

	return p;
}

void draw_item_sep(struct item *p)
{
	double y;

	y = round(p->area.y + p->area.h / 2) + 0.5;
	ui_draw_line(p->area.x + 5, y, p->area.x + p->area.w - 5, y,
		     1.0, config.color_sep_fg);
}

void draw_item_sep_with_text(struct item *p)
{
	int text_x_coord;

	text_x_coord = p->area.x + config.item_padding_x;
	if (config.icon_size)
		text_x_coord += config.icon_size + config.item_padding_x;

	ui_insert_text(parse_caret_action(p->name, "^sep("), text_x_coord,
		       p->area.y, p->area.h, p->area.w, config.color_sep_fg,
		       config.item_halign);
}

void draw_item_bg_norm(struct item *p)
{
	ui_draw_rectangle(p->area.x, p->area.y, p->area.w,
			  p->area.h, config.item_radius, 0.0, 1,
			  config.color_norm_bg);
}

void draw_item_bg_sel(struct item *p)
{
	ui_draw_rectangle(p->area.x, p->area.y, p->area.w,
			  p->area.h, config.item_radius, 0.0, 1,
			  config.color_sel_bg);
	if (config.item_border)
		ui_draw_rectangle(p->area.x, p->area.y, p->area.w,
				  p->area.h, config.item_radius,
				  config.item_border, 0,
				  config.color_sel_border);
}

void draw_item_text(struct item *p)
{
	int text_x_coord;

	if (config.item_halign != RIGHT) {
		text_x_coord = p->area.x + config.item_padding_x;
		if (config.icon_size)
			text_x_coord += config.icon_size + config.icon_text_spacing;
	} else {
		text_x_coord = p->area.x - config.item_padding_x;
		if (config.icon_size)
			text_x_coord -= config.icon_size + config.icon_text_spacing;
	}

	if (p == menu.sel)
		ui_insert_text(p->name, text_x_coord, p->area.y,
			       p->area.h, p->area.w, config.color_sel_fg,
			       config.item_halign);
	else
		ui_insert_text(p->name, text_x_coord, p->area.y,
			       p->area.h, p->area.w, config.color_norm_fg,
			       config.item_halign);
}

void draw_submenu_arrow(struct item *p)
{
	if (config.item_halign != RIGHT)
		ui_insert_text(config.arrow_string, p->area.x + p->area.w -
			       config.item_padding_x - (config.arrow_width * 0.7), p->area.y,
			       p->area.h, p->area.w, config.color_norm_fg,
			       config.item_halign);
	else
		ui_insert_text(config.arrow_string, p->area.x + config.item_padding_x,
			       p->area.y, p->area.h, config.arrow_width * 0.7,
			       config.color_norm_fg, config.item_halign);
}

void draw_icon(struct item *p)
{
	int icon_y_coord;
	int offsety, offsetx;

	offsety = cairo_image_surface_get_height(p->icon) < config.icon_size ?
		  (config.icon_size - cairo_image_surface_get_height(p->icon)) / 2 : 0;
	offsetx = cairo_image_surface_get_width(p->icon) < config.icon_size ?
		  (config.icon_size - cairo_image_surface_get_width(p->icon)) / 2 : 0;

	icon_y_coord = p->area.y + (config.item_height - config.icon_size) / 2 +
		       offsety;
	if (config.item_halign != RIGHT)
		ui_insert_image(p->icon, p->area.x + offsetx + 1, icon_y_coord,
				config.icon_size);
	else
		ui_insert_image(p->icon, p->area.x + p->area.w - config.icon_size
				+ offsetx - 1, icon_y_coord, config.icon_size);
}

void draw_menu(void)
{
	struct item *p;
	int w;

	w = geo_get_menu_width();

	/* Draw background */
	ui_clear_canvas();
	ui_draw_rectangle(0, 0, w, geo_get_menu_height(), config.menu_radius,
			  0.0, 1, config.color_menu_bg);

	/* Draw menu border */
	if (config.menu_border)
		ui_draw_rectangle(0, 0, w, geo_get_menu_height(),
				  config.menu_radius, config.menu_border,
				  0, config.color_menu_border);

	/* Draw menu items */
	p = menu.first;
	list_for_each_entry_from(p, &menu.filter, filter) {
		/* Draw item background */
		if (p == menu.sel)
			draw_item_bg_sel(p);
		else if (p->selectable)
			draw_item_bg_norm(p);

		/* Draw submenu arrow */
		if (config.arrow_width && (!strncmp(p->cmd, "^checkout(", 10) ||
					   !strncmp(p->cmd, "^sub(", 5)))
			draw_submenu_arrow(p);

		/* Draw menu items text */
		if (p->selectable)
			draw_item_text(p);
		else if (!strncmp(p->name, "^sep()", 6))
			draw_item_sep(p);
		else if (!strncmp(p->name, "^sep(", 5))
			draw_item_sep_with_text(p);

		/* Draw Icons */
		if (config.icon_size && p->icon)
			draw_icon(p);

		if (p == menu.last)
			break;
	}

	ui_map_window(geo_get_menu_width(), geo_get_menu_height());
}

struct node *get_node_from_tag(const char *tag)
{
	struct node *n;

	if (!tag)
		return NULL;

	list_for_each_entry(n, &menu.nodes, node) {
		if (!strcmp(tag, n->tag))
			return n;
	}

	return NULL;
}

int submenu_itemarea_width(void)
{
	struct item *p;
	struct sbuf s;
	struct point point;

	sbuf_init(&s);
	p = menu.subhead;
	list_for_each_entry_from(p, &menu.master, master) {
		if (p->name) {
			sbuf_addstr(&s, p->name);
			sbuf_addch(&s, '\n');
		}
		if (p == menu.subtail)
			break;
	}
	point = ui_get_text_size(s.buf, font_get());
	/* point.x now holds the width of the widest 'line of text' */

	point.x += config.item_padding_x * 2;
	point.x += config.item_margin_x * 2;
	if (config.icon_size)
		point.x += config.icon_size + config.icon_text_spacing;
	point.x += config.arrow_width;
	free(s.buf);
	return point.x;
}

void set_submenu_width(void)
{
	int mw;
	int reqw = geo_get_menu_width_from_itemarea_width(submenu_itemarea_width());
	struct point maxarea = geo_get_max_menuarea_that_fits();

	mw = reqw < maxarea.x ? reqw : maxarea.x;
	if (mw < config.menu_width)
		mw = config.menu_width;

	if (!config.at_pointer)
		goto set_width;
	/* grow from right hand edge if too near it */
	if (config.menu_halign == LEFT && reqw > maxarea.x) {
		geo_set_menu_margin_x(geo_get_screen_width() - reqw);
		mw = reqw;
	}

set_width:
	geo_set_menu_width(mw);
}

int submenu_itemarea_height(void)
{
	struct item *p;
	int h = 0;

	p = menu.subhead;
	list_for_each_entry_from(p, &menu.master, master) {
		h += config.item_margin_y + p->area.h;
		if (p == menu.subtail)
			break;
	}
	h += config.item_margin_y;
	return h;
}

void set_submenu_height(void)
{
	int reqh = submenu_itemarea_height();
	struct point maxarea = geo_get_max_itemarea_that_fits();
	int h;

	h = reqh < maxarea.y ? reqh : maxarea.y;
	geo_set_menu_height_from_itemarea_height(h);
}

int tag_exists(const char *tag)
{
	struct node *n;

	if (!tag)
		return 0;
	list_for_each_entry(n, &menu.nodes, node) {
		if (!strcmp(tag, n->tag))
			return 1;
	}
	return 0;
}

void checkout_submenu(char *tag)
{
	struct item *item;

	/* Find head of submenu */
	if (!tag || !strncmp(tag, "__root__", 8)) {
		menu.current_node = list_first_entry_or_null(&menu.nodes, struct node, node);
		menu.subhead = list_first_entry_or_null(&menu.master, struct item, master);
	} else {
		if (!tag_exists(tag)) {
			warn("tag '%s' does not exist", tag);
			return;
		}
		menu.current_node = get_node_from_tag(tag);
		if (!menu.current_node)
			die("node '%s' does not exist", tag);
		menu.subhead = container_of((menu.current_node->item)->master.next,
					    struct item, master);
		if (!menu.subhead)
			die("no menu.subhead");
	}

	/* Find tail of submenu */
	menu.subtail = menu.subhead;
	item = menu.subhead;
	list_for_each_entry_from(item, &menu.master, master) {
		if (item->tag)
			break;
		menu.subtail = item;
	}

	set_submenu_height();
	set_submenu_width();
}

void checkout_root(void)
{
	while (menu.current_node->parent)
		menu.current_node = menu.current_node->parent;
	checkout_submenu(menu.current_node->tag);
}

void resize(void)
{
	XMoveResizeWindow(ui->dpy, ui->win, geo_get_menu_x0(),
			  geo_get_menu_y0(), geo_get_menu_width(),
			  geo_get_menu_height());
}

void update(int resize_required)
{
	update_filtered_list();
	init_menuitem_coordinates();
	draw_menu();
	if (!resize_required)
		return;
	resize();
	ui_map_window(geo_get_menu_width(), geo_get_menu_height());
}

void launch_menu_at_pointer(void)
{
	Window dw;
	int di;
	unsigned int du;
	struct point pos;

	XQueryPointer(ui->dpy, DefaultRootWindow(ui->dpy), &dw, &dw, &di, &di,
		      &pos.x, &pos.y, &du);

	if (pos.x < geo_get_screen_width() - geo_get_menu_width()) {
		geo_set_menu_halign(LEFT);
		geo_set_menu_margin_x(pos.x);
	} else {
		geo_set_menu_halign(RIGHT);
		geo_set_menu_margin_x(geo_get_screen_width() - pos.x);
	}

	if (pos.y < geo_get_screen_height() - geo_get_menu_height()) {
		geo_set_menu_valign(TOP);
		geo_set_menu_margin_y(pos.y);
	} else if (geo_get_menu_height() < pos.y) {
		geo_set_menu_valign(BOTTOM);
		geo_set_menu_margin_y(geo_get_screen_height() - pos.y);
	} else {
		geo_set_menu_valign(BOTTOM);
		geo_set_menu_margin_y(0);
	}
	set_submenu_width();
}

int tint2_getenv(int *var, const char *key)
{
	char *s;

	s = getenv(key);
	if (!s)
		return -1;
	xatoi(var, s, XATOI_NONNEG, key);
	return 0;
}

void tint2_align(void)
{
	int bx1, bx2, by1, by2, px1, px2, py1, py2;

	if (tint2_getenv(&bx1, "TINT2_BUTTON_ALIGNED_X1") == -1 ||
	    tint2_getenv(&bx2, "TINT2_BUTTON_ALIGNED_X2") == -1 ||
	    tint2_getenv(&by1, "TINT2_BUTTON_ALIGNED_Y1") == -1 ||
	    tint2_getenv(&by2, "TINT2_BUTTON_ALIGNED_Y2") == -1 ||
	    tint2_getenv(&px1, "TINT2_BUTTON_PANEL_X1") == -1 ||
	    tint2_getenv(&px2, "TINT2_BUTTON_PANEL_X2") == -1 ||
	    tint2_getenv(&py1, "TINT2_BUTTON_PANEL_Y1") == -1 ||
	    tint2_getenv(&py2, "TINT2_BUTTON_PANEL_Y2") == -1)
		return;

	if (t2conf_is_horizontal_panel() == -1) {
		warn("invalid value for tint2 panel orientation");
		return;
	}
	if (t2conf_is_horizontal_panel()) {
		printf("info: aligning to tint2 button variables in horizontal panel mode\n");
		if (bx1 < px2 - geo_get_menu_width()) {
			geo_set_menu_margin_x(bx1);
			geo_set_menu_halign(LEFT);
		} else {
			geo_set_menu_margin_x(geo_get_screen_width() - px2);
			geo_set_menu_halign(RIGHT);
		}
		if (config.menu_valign == BOTTOM)
			geo_set_menu_margin_y(geo_get_screen_height() - py1);
		else
			geo_set_menu_margin_y(py2);
	} else {
		printf("info: aligning to tint2 button variables in vertical panel mode\n");
		if (by1 < py2 - geo_get_menu_height()) {
			geo_set_menu_margin_y(by1);
			geo_set_menu_valign(TOP);
		} else {
			geo_set_menu_margin_y(geo_get_screen_height() - py2);
			geo_set_menu_valign(TOP);
		}
		if (config.menu_halign == LEFT)
			geo_set_menu_margin_x(px2);
		else
			geo_set_menu_margin_x(geo_get_screen_width() - px1);
	}
}

static void if_unity_run_hack(void)
{
	static int first_run = 1;
	static int isunity;
	char *s;

	if (first_run) {
		s = getenv("JGMENU_UNITY");
		if (s && s[0] == '1')
			isunity = 1;
		first_run = 0;
	}
	if (isunity)
		spawn("jgmenu_run unity-hack");
}

static void awake_menu(void)
{
	if_unity_run_hack();
	if (config.at_pointer) {
		launch_menu_at_pointer();
		resize();
	}
	/* for speed improvement, set tint2_button = 0 */
	if (config.tint2_button && !config.at_pointer) {
		tint2env_read_socket();
		tint2_align();
		update(1);
	}
	XMapWindow(ui->dpy, ui->win);
	ui_map_window(geo_get_menu_width(), geo_get_menu_height());
	grabkeyboard();
	grabpointer();
}

static void hide_menu(void)
{
	XUngrabKeyboard(ui->dpy, CurrentTime);
	XUngrabPointer(ui->dpy, CurrentTime);
	XUnmapWindow(ui->dpy, ui->win);
	filter_reset();
	checkout_root();
	update(1);
}

void hide_or_exit(void)
{
	if (config.stay_alive)
		hide_menu();
	else
		exit(0);
}

void checkout_parent(void)
{
	if (!menu.current_node->parent)
		return;
	checkout_submenu(menu.current_node->parent->tag);
}

void action_cmd(char *cmd)
{
	char *p = NULL;

	if (!cmd)
		return;

	if (!strncmp(cmd, "^checkout(", 10)) {
		p = parse_caret_action(cmd, "^checkout(");
		if (!p)
			return;
		menu.current_node->last_sel = menu.sel;
		checkout_submenu(p);
		update(1);
	} else if (!strncmp(cmd, "^sub(", 5)) {
		p = parse_caret_action(cmd, "^sub(");
		if (!p)
			return;
		spawn(p);
		hide_or_exit();
	} else if (!strncmp(cmd, "^back(", 6)) {
		checkout_parent();
		update(1);
	} else if (!strncmp(cmd, "^term(", 6)) {
		struct sbuf s;

		p = parse_caret_action(cmd, "^term(");
		if (!p)
			return;
		sbuf_init(&s);
		term_build_terminal_cmd(&s, strstrip(p), config.terminal_exec,
					config.terminal_args);
		spawn(s.buf);
		free(s.buf);
		hide_or_exit();
	} else {
		spawn(cmd);
		hide_or_exit();
	}
}

void key_event(XKeyEvent *ev)
{
	char buf[32];
	int len;
	KeySym ksym = NoSymbol;
	Status status;
	int isoutside;

	len = Xutf8LookupString(ui->xic, ev, buf, sizeof(buf), &ksym, &status);
	if (status == XBufferOverflow)
		return;
	switch (ksym) {
	case XK_End:
		if (filter_head() == &empty_item)
			break;
		menu.last = filter_tail();
		menu.first = fill_from_bottom(menu.last);
		menu.sel = menu.last;
		init_menuitem_coordinates();
		draw_menu();
		break;
	case XK_Escape:
		if (filter_needle_length()) {
			filter_reset();
			update(0);
		} else {
			hide_or_exit();
		}
		break;
	case XK_Home:
		if (filter_head() == &empty_item)
			break;
		menu.first = filter_head();
		menu.last = fill_from_top(menu.first);
		menu.sel = menu.first;
		init_menuitem_coordinates();
		draw_menu();
		break;
	case XK_Up:
		if (filter_head() == &empty_item ||
		    menu.sel == filter_head())
			break;
		menu.sel = prev_selectable(menu.sel, &isoutside);
		if (isoutside) {
			menu.first = menu.sel;
			menu.last = fill_from_top(menu.first);
		}
		init_menuitem_coordinates();
		draw_menu();
		break;
	case XK_Next:	/* PageDown */
		if (filter_head() == &empty_item)
			break;
		menu.first = menu.last;
		if (menu.first != filter_tail())
			step_fwd(&menu.first, 1);
		menu.last = fill_from_top(menu.first);
		if (menu.last == filter_tail())
			menu.first = fill_from_bottom(menu.last);
		init_menuitem_coordinates();
		menu.sel = menu.last;
		if (!menu.last->selectable)
			menu.sel = prev_selectable(menu.last, &isoutside);
		draw_menu();
		break;
	case XK_Prior:	/* PageUp */
		if (filter_head() == &empty_item)
			break;
		menu.last = menu.first;
		if (menu.last != filter_head())
			step_back(&menu.last, 1);
		menu.first = fill_from_bottom(menu.last);
		if (menu.first == filter_head())
			menu.last = fill_from_top(menu.first);
		init_menuitem_coordinates();
		menu.sel = menu.first;
		if (!menu.sel->selectable)
			menu.sel = next_selectable(menu.first, &isoutside);
		draw_menu();
		break;
	case XK_Return:
	case XK_KP_Enter:
		if (!menu.sel->selectable)
			break;
		if (config.spawn) {
			action_cmd(menu.sel->cmd);
		} else {
			printf("%s", menu.sel->cmd);
			hide_or_exit();
		}
		break;
	case XK_Down:
		if (filter_head() == &empty_item ||
		    menu.sel == filter_tail())
			break;
		menu.sel = next_selectable(menu.sel, &isoutside);
		if (isoutside) {
			menu.last = menu.sel;
			menu.first = fill_from_bottom(menu.last);
		}
		init_menuitem_coordinates();
		draw_menu();
		break;
	case XK_Left:
		if (!menu.current_node->parent)
			break;
		checkout_parent();
		update(1);
		break;
	case XK_Right:
		if (strncmp(menu.sel->cmd, "^checkout(", 10))
			break;
		action_cmd(menu.sel->cmd);
		break;
	case XK_F5:
		restart();
		break;
	case XK_F10:
		/* force exit even if in 'stay_alive' mode */
		exit(0);
		break;
	case XK_BackSpace:
		if (filter_needle_length()) {
			filter_backspace();
			update(0);
		} else {
			checkout_parent();
			update(1);
		}
		break;
	default:
		filter_addstr(buf, len);
		update(0);
		break;
	}
}

struct point mousexy(void)
{
	Window dw;
	int di;
	unsigned int du;
	struct point coords;

	XQueryPointer(ui->dpy, ui->win, &dw, &dw, &di, &di, &coords.x,
		      &coords.y, &du);

	return coords;
}

/* Pointer vertical offset (not sure why this is needed) */
#define MOUSE_FUDGE 3
void mouse_event(XEvent *e)
{
	struct item *item;
	XButtonReleasedEvent *ev = &e->xbutton;
	struct point mouse_coords;

	mouse_coords = mousexy();
	mouse_coords.y -= MOUSE_FUDGE;

	/* Die if mouse clicked outside window */
	if (ev->button == Button1 &&
	    (ev->x < geo_get_menu_x0() ||
	    ev->x > geo_get_menu_x0() + geo_get_menu_width() ||
	    ev->y < geo_get_menu_y0() ||
	    ev->y > geo_get_menu_y0() + geo_get_menu_height()))
		hide_or_exit();

	/* right-click */
	if (ev->button == Button3) {
		checkout_parent();
		update(1);
	}

	/* scroll up */
	if (ev->button == Button4 && menu.first != filter_head()) {
		step_back(&menu.first, 1);
		menu.last = fill_from_top(menu.first);
		step_back(&menu.sel, 1);
		init_menuitem_coordinates();
		draw_menu();
		return;
	}

	/* scroll down */
	if (ev->button == Button5 && menu.last != filter_tail()) {
		step_fwd(&menu.last, 1);
		menu.first = fill_from_bottom(menu.last);
		step_fwd(&menu.sel, 1);
		init_menuitem_coordinates();
		draw_menu();
		return;
	}

	/* left-click */
	if (ev->button == Button1) {
		item = menu.first;
		list_for_each_entry_from(item, &menu.master, master) {
			if (ui_is_point_in_area(mouse_coords, item->area)) {
				if (!item->selectable)
					break;
				if (config.spawn) {
					action_cmd(item->cmd);
					break;
				}
				puts(item->cmd);
				hide_or_exit();
			}
			if (item == menu.last)
				break;
		}
	}
}

static double timespec_to_sec(struct timespec *ts)
{
	return (double)ts->tv_sec + (double)ts->tv_nsec / 1000000000.0;
}

/*
 * This function is loaded in the background under a new pthread
 * X11 is not thread-safe, so load_icons() must not call any X functions.
 */
void *load_icons(void *arg)
{
	struct timespec ts_start;
	struct timespec ts_end;
	double duration;

	clock_gettime(CLOCK_MONOTONIC, &ts_start);
	icon_load();
	clock_gettime(CLOCK_MONOTONIC, &ts_end);
	if (DEBUG_ICONS_LOADED_NOTIFICATION) {
		duration = timespec_to_sec(&ts_end) - timespec_to_sec(&ts_start);
		fprintf(stderr, "Icons loaded in %f seconds\n", duration);
	}

	if (write(pipe_fds[1], "x", 1) == -1)
		die("error writing to icon_pipe");

	return NULL;
}

struct item *get_item_from_tag(const char *tag)
{
	struct item *item;

	if (!tag)
		die("tag=(null) in get_item_from_tag()");

	list_for_each_entry(item, &menu.master, master)
		if (item->tag && !strcmp(tag, item->tag))
			return item;

	if (tag && strncmp(tag, "root", 4))
		fprintf(stderr, "warning: could not find tag '%s'\n", tag);
	return NULL;
}

void hang_items_off_nodes(void)
{
	struct node *n;
	struct item *p;

	list_for_each_entry(n, &menu.nodes, node) {
		if (!n->tag)
			die("node has no tag");

		if (n->item)
			p = container_of((n->item)->master.next,
					 struct item, master);
		else
			p = list_first_entry_or_null(&menu.master,
						     struct item, master);

		list_for_each_entry_from(p, &menu.master, master) {
			if (p->tag)
				break;
			list_add_tail(&p->list, &n->items);
		}
	}
}

int node_exists(const char *name)
{
	struct node *n;

	if (!name)
		die("node_exists was called without name");

	list_for_each_entry(n, &menu.nodes, node)
		if (!strcmp(name, n->tag))
			return 1;

	return 0;
}

void create_node(const char *name, struct node *parent)
{
	struct node *n;

	n = xmalloc(sizeof(struct node));
	if (!name)
		die("cannot create node without name");
	n->tag = strdup(name);
	if (!strcmp(name, "__root__"))
		n->item = list_first_entry_or_null(&menu.master,
						   struct item, master);
	else
		n->item = get_item_from_tag(name);
	n->last_sel = NULL;
	n->parent = parent;
	INIT_LIST_HEAD(&n->items);
	list_add_tail(&n->node, &menu.nodes);
}

/* Create nodal tree from tagged items */
void walk_tagged_items(struct item *this, struct node *parent)
{
	struct item *child, *p;
	struct node *current_node;

	if (this) {
		create_node(this->tag, parent);
		/* move to next item, as this points to a ^tag() item */
		p = container_of((this)->master.next, struct item, master);
	} else {
		create_node("__root__", NULL);
		p = list_first_entry_or_null(&menu.master, struct item, master);
	}
	/* p now points to first menu-item under tag "this->tag" */

	if (p == list_last_entry(&menu.master, struct item, master))
		return;

	/* FIXME: Check if node(s.buf) already exists */
	current_node = list_last_entry(&menu.nodes, struct node, node);

	/* walk the items under current node and put into tree structure */
	list_for_each_entry_from(p, &menu.master, master) {
		if (!strncmp("^checkout(", p->cmd, 10)) {
			child = get_item_from_tag(parse_caret_action(p->cmd, "^checkout("));
			if (!child)
				continue;
			if (child->tag && node_exists(child->tag))
				continue;
			walk_tagged_items(child, current_node);
		} else if (!strncmp("^tag(", p->cmd, 5)) {
			break;
		}
	}
}

void build_tree(void)
{
	struct item *item;

	if (list_empty(&menu.master))
		die("cannot build tree on empty menu.master list");
	item = list_first_entry_or_null(&menu.master, struct item, master);
	if (item->tag && list_is_singular(&menu.master))
		die("cannot build a menu on a single tag item");

	/* consider case when first item is not a ^tag() item */
	if (!item->tag)
		walk_tagged_items(NULL, NULL);
	else
		walk_tagged_items(get_item_from_tag(item->tag), NULL);
}

void read_csv_file(FILE *fp)
{
	char buf[BUFSIZ], *p;
	size_t i;
	struct item *item = NULL;
	struct argv_buf argv_buf;

	if (!fp)
		die("no csv-file");
	argv_set_delim(&argv_buf, ',');
	for (i = 0; fgets(buf, sizeof(buf), fp); i++) {
		p = strchr(buf, '\n');
		if (p)
			*p = '\0';
		if ((buf[0] == '#') ||
		    (buf[0] == '\n') ||
		    (buf[0] == '\0')) {
			i--;
			continue;
		}
		argv_init(&argv_buf);
		argv_strdup(&argv_buf, buf);
		argv_parse(&argv_buf);
		item = xmalloc(sizeof(struct item));
		item->name = argv_buf.argv[0];
		item->cmd = argv_buf.argv[1];
		item->iconname = argv_buf.argv[2];
		if (!item->cmd)
			item->cmd = item->name;
		list_add_tail(&item->master, &menu.master);
	}

	if (!item || i <= 0)
		die("input file contains no menu items");

	/* Init items */
	list_for_each_entry(item, &menu.master, master) {
		item->icon = NULL;
		item->tag = NULL;
		item->selectable = 1;
		item->area.h = config.item_height;
		if (!strncmp(item->name, "^sep()", 6)) {
			item->selectable = 0;
			item->area.h = config.sep_height;
		} else if (!strncmp(item->name, "^sep(", 5)) {
			item->selectable = 0;
		}
	}

	/* Populate tag field */
	list_for_each_entry(item, &menu.master, master) {
		if (strncmp("^tag(", item->cmd, 5))
			continue;
		item->tag = parse_caret_action(item->cmd, "^tag(");
	}
}

void init_pipe_flags(void)
{
	int flags;

	flags = fcntl(pipe_fds[0], F_GETFL);
	if (flags == -1)
		die("error getting pipe flags");
	flags |= O_NONBLOCK;
	if (fcntl(pipe_fds[0], F_SETFL, flags) == -1)
		die("error setting pipe flags");

	flags = fcntl(pipe_fds[1], F_GETFL);
	if (flags == -1)
		die("error getting pipe flags");
	flags |= O_NONBLOCK;
	if (fcntl(pipe_fds[1], F_SETFL, flags) == -1)
		die("error setting pipe flags");
}

/*
 * Move highlighting with mouse
 *
 * Get mouse coordinates using XQueryPointer()
 * ev.xbutton.x and ev.xbutton.y work most of the time,
 * but occasionally throw in odd values.
 */
void process_pointer_position(void)
{
	struct item *item;
	struct point mouse_coords;
	static int oldy;
	static int oldx;

	mouse_coords = mousexy();
	mouse_coords.y -= MOUSE_FUDGE;

	if ((mouse_coords.x == oldx) && (mouse_coords.y == oldy))
		return;

	item = menu.first;
	list_for_each_entry_from(item, &menu.filter, filter) {
		if (!item->selectable)
			continue;
		if (ui_is_point_in_area(mouse_coords, item->area)) {
			if (menu.sel != item) {
				menu.sel = item;
				draw_menu();
				break;
			}
		}
		if (item == menu.last)
			break;
	}

	oldx = mouse_coords.x;
	oldy = mouse_coords.y;
}

static void signal_handler(int sig)
{
	int saved_errno;

	saved_errno = errno;
	if (write(pipe_fds[1], "1", 1) == -1 && errno != EAGAIN)
		die("write");
	errno = saved_errno;
}

void run(void)
{
	XEvent ev;
	struct item *item;

	char ch;
	int ready, nfds, x11_fd;
	fd_set readfds;
	static int all_icons_have_been_requested;
	struct sigaction sa;

	/* for performance testing */
	if (die_when_loaded && !config.icon_size)
		exit(0);

	FD_ZERO(&readfds);
	nfds = 0;

	/* Set x11 fd */
	x11_fd = ConnectionNumber(ui->dpy);
	nfds = MAX(nfds, x11_fd + 1);
	FD_SET(x11_fd, &readfds);

	/* Create icon pipe */
	if (pipe(pipe_fds) == -1)
		die("error creating pipe");

	FD_SET(pipe_fds[0], &readfds);
	nfds = MAX(nfds, pipe_fds[0] + 1);

	init_pipe_flags();

	if (config.icon_size) {
		/*
		 * Get icons in top level menu (or the one specified with
		 * --check-out=
		 */
		item = menu.subhead;
		list_for_each_entry_from(item, &menu.master, master) {
			if (item->iconname)
				icon_set_name(item->iconname);
			if (item == menu.subtail)
				break;
		}
		pthread_create(&thread, NULL, load_icons, NULL);
	}

	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	sa.sa_handler = signal_handler;
	if (sigaction(SIGUSR1, &sa, NULL) == -1)
		die("sigaction");

	for (;;) {
		FD_ZERO(&readfds);
		FD_SET(x11_fd, &readfds);
		FD_SET(pipe_fds[0], &readfds);

		/*
		 * XPending() is non-blocking whereas select() is blocking.
		 *
		 * Some X events are stored in a queue in memory, so we cannot
		 * rely on reading ConnectionNumber() alone to catch all events.
		 */
		ready = 0;
		if (!XPending(ui->dpy))
			ready = select(nfds, &readfds, NULL, NULL, NULL);

		if (ready == -1 && errno == EINTR)
			continue;

		if (ready == -1)
			die("select()");

		/*
		 * Check if there is something in the selfpipe. E.g. the icon
		 * thread has finished or we have caught a USR1 signal
		 */
		if (FD_ISSET(pipe_fds[0], &readfds) && ready) {
			for (;;) {
				if (read(pipe_fds[0], &ch, 1) == -1) {
					if (errno == EAGAIN)
						break;
					die("error reading pipe");
				}

				/* Caught SIGUSR1 */
				if (ch == '1') {
					awake_menu();
					continue;
				}

				/* 'x' means that icons have finished loading */
				if (ch != 'x')
					continue;

				/* for performance testing */
				if (die_when_loaded && all_icons_have_been_requested)
					exit(0);

				if (DEBUG_ICONS_LOADED_NOTIFICATION &&
				    all_icons_have_been_requested)
					fprintf(stderr, "All icons loaded\n");

				if (DEBUG_ICONS_LOADED_NOTIFICATION &&
				    !all_icons_have_been_requested)
					fprintf(stderr, "Root menu icons loaded\n");

				pthread_join(thread, NULL);

				list_for_each_entry(item, &menu.master, master)
					if (!item->icon)
						item->icon = icon_get_surface(item->iconname);

				draw_menu();

				if (all_icons_have_been_requested)
					continue;

				/* Get remaining icons */
				list_for_each_entry(item, &menu.master, master)
					if (item->iconname)
						icon_set_name(item->iconname);

				pthread_create(&thread, NULL, load_icons, NULL);
				all_icons_have_been_requested = 1;
			}
		}

		if (XPending(ui->dpy)) {
			XNextEvent(ui->dpy, &ev);
			if (XFilterEvent(&ev, ui->win))
				continue;

			switch (ev.type) {
			case MappingNotify:
				XRefreshKeyboardMapping(&ev.xmapping);
				break;
			case ButtonRelease:
				mouse_event(&ev);
				break;
			case KeyPress:
				key_event(&ev.xkey);
				break;
			case Expose:
				if (ev.xexpose.count == 0)
					ui_map_window(geo_get_menu_width(),
						      geo_get_menu_height());
				break;
			case VisibilityNotify:
				if (ev.xvisibility.state != VisibilityUnobscured)
					XRaiseWindow(ui->dpy, ui->win);
				break;
			}

			process_pointer_position();
		}
	}
}

void init_geo_variables_from_config(void)
{
	geo_set_menu_halign(config.menu_halign);
	geo_set_menu_valign(config.menu_valign);
	geo_set_menu_margin_x(config.menu_margin_x);
	geo_set_menu_margin_y(config.menu_margin_y);
	geo_set_menu_padding_top(config.menu_padding_top);
	geo_set_menu_padding_right(config.menu_padding_right);
	geo_set_menu_padding_bottom(config.menu_padding_bottom);
	geo_set_menu_padding_left(config.menu_padding_left);
	geo_set_menu_width(config.menu_width);
	geo_set_item_margin_x(config.item_margin_x);
	geo_set_item_margin_y(config.item_margin_y);
	geo_set_item_height(config.item_height);
}

void set_font(void)
{
	font_set();
	info("set font to '%s'", font_get());
}

void set_theme(void)
{
	struct sbuf theme;

	if (!config.icon_size)
		return;
	sbuf_init(&theme);
	theme_set(&theme);
	icon_init();
	icon_set_size(config.icon_size);
	info("set icon theme to '%s'", theme.buf);
	icon_set_theme(theme.buf);
}

static char *tag_of_first_item(void)
{
	struct item *item;

	item = list_first_entry_or_null(&menu.master, struct item, master);
	if (!item)
		die("no items in master list");
	return item->tag;
}

static void quit(int signum)
{
	fprintf(stderr, "\ninfo: caught SIGTERM or SIGINT\n");
	exit(0);
}

static void set_simple_mode(void)
{
	config.stay_alive = 0;
	config.tint2_look = 0;
	config.tint2_button = 0;
	config.tint2_rules = 0;
}

static void read_jgmenurc(const char *filename)
{
	struct stat sb;
	static struct sbuf f;
	static int initiated;

	if (initiated) {
		if (stat(f.buf, &sb) != 0)
			return;
		config_parse_file(f.buf);
		return;
	}
	sbuf_init(&f);
	if (filename)
		sbuf_cpy(&f, filename);
	if (!f.len)
		sbuf_cpy(&f, "~/.config/jgmenu/jgmenurc");
	sbuf_expand_tilde(&f);
	initiated = 1;
	if (stat(f.buf, &sb) != 0)
		return;
	config_parse_file(f.buf);
}

static void read_tint2rc(void)
{
	struct sbuf f;

	sbuf_init(&f);
	bl_tint2file(&f);
	if (f.len) {
		info("using BunsenLabs tint2 session file");
		t2conf_parse(f.buf, geo_get_screen_width(),
			     geo_get_screen_height());
	} else {
		t2conf_parse(NULL, geo_get_screen_width(),
			     geo_get_screen_height());
	}
	free(f.buf);
}

int main(int argc, char *argv[])
{
	int i;
	char *arg_checkout = NULL, *arg_config_file = NULL;
	char *csv_file = NULL, *csv_cmd = NULL;
	int arg_simple = 0, arg_vsimple = 0;
	struct sigaction term_action, int_action;
	FILE *fp = NULL;

	if (!setlocale(LC_ALL, ""))
		die("error setting locale");
	if (!XSupportsLocale())
		die("error setting locale");
	if (!XSetLocaleModifiers("@im=none"))
		die("error setting locale");

	restart_init(argv);
	memset(&term_action, 0, sizeof(struct sigaction));
	memset(&int_action, 0, sizeof(struct sigaction));
	term_action.sa_handler = quit;
	int_action.sa_handler = quit;
	sigaction(SIGTERM, &term_action, NULL);
	sigaction(SIGINT, &int_action, NULL);

	config_set_defaults();
	menu.current_node = NULL;
	INIT_LIST_HEAD(&menu.master);
	INIT_LIST_HEAD(&menu.filter);
	INIT_LIST_HEAD(&menu.nodes);

	for (i = 1; i < argc; i++) {
		if (!strncmp(argv[i], "--config-file=", 14))
			arg_config_file = argv[i] + 14;
		else if (!strncmp(argv[i], "--vsimple", 9))
			arg_vsimple = 1;
	}
	if (!arg_vsimple)
		read_jgmenurc(arg_config_file);

	for (i = 1; i < argc; i++) {
		if (!strncmp(argv[i], "--version", 9)) {
			printf("%s\n", VERSION);
			exit(0);
		} else if (!strncmp(argv[i], "--help", 6) ||
			   !strncmp(argv[i], "-h", 2)) {
			usage();
		} else if (!strncmp(argv[i], "--no-spawn", 10)) {
			config.spawn = 0;
		} else if (!strncmp(argv[i], "--checkout=", 11)) {
			arg_checkout = argv[i] + 11;
		} else if (!strncmp(argv[i], "--icon-size=", 12)) {
			xatoi(&config.icon_size, argv[i] + 12, XATOI_NONNEG,
			      "config.icon_size");
		} else if (!strncmp(argv[i], "--die-when-loaded", 17)) {
			die_when_loaded = 1;
		} else if (!strncmp(argv[i], "--at-pointer", 12)) {
			config.at_pointer = 1;
		} else if (!strncmp(argv[i], "--hide-on-startup", 17)) {
			config.hide_on_startup = 1;
		} else if (!strncmp(argv[i], "--simple", 8)) {
			arg_simple = 1;
		} else if (!strncmp(argv[i], "--csv-file=", 11)) {
			csv_file = argv[i] + 11;
		} else if (!strncmp(argv[i], "--csv-cmd=", 10)) {
			csv_cmd = argv[i] + 10;
		}
	}

	if (arg_simple || arg_vsimple)
		set_simple_mode();
	if (arg_vsimple)
		config.icon_size = 0;

	/* check lockfile after --help and --version */
	if (config.stay_alive)
		lockfile_init();

	if_unity_run_hack();

	ui_init();
	geo_init();

	if (config.tint2_look)
		read_tint2rc();

	/* Parse jgmenurc again to overrule tint2rc values */
	if (!config.tint2_rules && !arg_vsimple)
		read_jgmenurc(arg_config_file);

	set_font();
	set_theme();
	init_geo_variables_from_config();

	if (config.tint2_button) {
		tint2env_init_socket();
		tint2_align();
	}

	if (csv_file)
		fp = fopen(csv_file, "r");
	else if (csv_cmd)
		fp = popen(csv_cmd, "r");
	if (!fp)
		fp = stdin;
	read_csv_file(fp);
	build_tree();
	hang_items_off_nodes();

	if (arg_checkout)
		checkout_submenu(arg_checkout);
	else
		checkout_submenu(tag_of_first_item());

	grabkeyboard();
	grabpointer();

	if (config.at_pointer)
		launch_menu_at_pointer();

	ui_create_window(geo_get_menu_x0(), geo_get_menu_y0(),
			 geo_get_menu_width(), geo_get_menu_height());
	ui_init_canvas(geo_get_screen_width(), geo_get_screen_height());
	ui_init_cairo(geo_get_screen_width(), geo_get_screen_height(), font_get());

	init_empty_item();
	filter_init();
	update_filtered_list();
	init_menuitem_coordinates();
	if (config.hide_on_startup) {
		info("menu started in 'hidden' mode; show by `jgmenu_run`");
		hide_menu();
	} else {
		XMapRaised(ui->dpy, ui->win);
	}
	draw_menu();

	run();

	ui_cleanup();

	return 0;
}
