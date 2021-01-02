/*
 * jgmenu.c
 *
 * Copyright (C) Johan Malm 2014
 *
 * jgmenu is a stand-alone menu which reads the menu items from a CSV file
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
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
#include <sys/time.h>

#include "x11-ui.h"
#include "config.h"
#include "util.h"
#include "geometry.h"
#include "sbuf.h"
#include "icon.h"
#include "filter.h"
#include "list.h"
#include "lockfile.h"
#include "argv-buf.h"
#include "t2conf.h"
#include "ipc.h"
#include "bl.h"
#include "xsettings-helper.h"
#include "terminal.h"
#include "restart.h"
#include "theme.h"
#include "font.h"
#include "args.h"
#include "widgets.h"
#include "pm.h"
#include "workarea.h"
#include "charset.h"
#include "hooks.h"
#include "spawn.h"
#include "banned.h"

#define DEBUG_ICONS_LOADED_NOTIFICATION 0

static pthread_t thread;	   /* worker thread for loading icons	  */
static int pipe_fds[2];		   /* talk between threads + catch sig    */
static int sw_close_pending;
static int menu_is_hidden;
static int super_key_pressed;

struct item {
	char *buf;
	char *name;
	char *cmd;
	char *iconname;
	char *working_dir;
	char *metadata;
/*	int strict_xdg_exec	*/
/*	int start_notify;	*/
	char *tag;
	struct area area;
	cairo_surface_t *icon;
	int selectable;
	struct list_head master;
	struct list_head filter;
};

static struct item empty_item;

/* A node is marked by a ^tag() and denotes the start of a submenu */
struct node {
	struct item *item;	   /* item that node points to		  */
	struct item *last_sel;	   /* used when returning to node	  */
	struct item *last_first;   /* used when returning to node	  */
	struct item *expanded;	   /* tracks item with sub window open    */
	struct node *parent;
	Window wid;
	struct list_head node;
};

/*
 * The lists "master" and "filter" are the core building blocks of the code
 * When a submenu is checked out, *subhead and *subtail are set.
 */
struct menu {
	struct item *subhead;	   /* first item in checked out submenu	  */
	struct item *subtail;	   /* last item in checked out submenu	  */
	struct item *first;	   /* first visible item		  */
	struct item *last;	   /* last visible item			  */
	struct item *sel;	   /* currently selected item		  */
	struct list_head master;   /* all items				  */
	struct list_head filter;   /* items to be displayed in menu	  */
	struct list_head nodes;	   /* hierarchical structure of tags	  */
	struct node *current_node;
};

static struct menu menu;

static const char jgmenu_usage[] =
"Usage: jgmenu [command] [options]\n\n"
"Commands include init\n"
"Options:\n"
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
"    --csv-cmd=<command>   specify command to produce menu data\n";

static void checkout_rootnode(void);
static void pipemenu_del_all(void);
static void pipemenu_del_beyond(struct node *keep_me);
static void tmr_mouseover_stop(void);
static void del_beyond_current(void);
static void del_beyond_root(void);

static void init_empty_item(void)
{
	empty_item.name = xstrdup("&lt;empty&gt;");
	empty_item.cmd = xstrdup(":");
	empty_item.iconname = NULL;
	empty_item.working_dir = NULL;
	empty_item.metadata = NULL;
	empty_item.tag = NULL;
	empty_item.icon = NULL;
	empty_item.selectable = 1;
	empty_item.area.h = config.item_height;
}

static void delete_empty_item(void)
{
	xfree(empty_item.name);
	xfree(empty_item.cmd);
}

static void usage(void)
{
	printf("%s", jgmenu_usage);
	exit(0);
}

static void version(void)
{
	printf("%s\n", (char *)VERSION);
	exit(0);
}

static void indent(int number_of_spaces)
{
	int i;

	if (number_of_spaces <= 0)
		return;
	for (i = 0; i < number_of_spaces; i++)
		fprintf(stderr, " ");
}

static int level(struct node *n)
{
	int i = 0;

	while (n->parent) {
		n = n->parent;
		++i;
	}
	return i;
}

static void print_nodes(void)
{
	struct node *n;

	fprintf(stderr, "nodes:\n");
	list_for_each_entry(n, &menu.nodes, node) {
		indent(level(n) * 2);
		fprintf(stderr, "%.8s ", n->item->tag);
		if (pm_is_pipe_node(n))
			fprintf(stderr, "[pipe] ");
		if (n->expanded)
			fprintf(stderr, "[expanded] ");
		fprintf(stderr, "\n");
	}
}

static struct item *filter_head(void)
{
	return list_first_entry_or_null(&menu.filter, struct item, filter);
}

static struct item *filter_tail(void)
{
	return list_last_entry(&menu.filter, struct item, filter);
}

static void step_back(struct item **ptr, int nr)
{
	int i;

	for (i = 0; i < nr; i++)
		*ptr = container_of((*ptr)->filter.prev, struct item, filter);
}

static void step_fwd(struct item **ptr, int nr)
{
	int i;

	for (i = 0; i < nr; i++)
		*ptr = container_of((*ptr)->filter.next, struct item, filter);
}

static struct item *fill_from_top(struct item *first)
{
	struct item *p, *last;
	int col, h;

	h = geo_get_itemarea_height() - config.item_margin_y;
	col = 1;
	p = first;
	last = p;
	list_for_each_entry_from(p, &menu.filter, filter) {
		h -= p->area.h + config.item_margin_y;
		if (h < 0) {
			col++;
			h = geo_get_itemarea_height() - config.item_margin_y;
			h -= p->area.h + config.item_margin_y;
		}
		if (col > config.columns)
			break;
		last = p;
	}
	return last;
}

static struct item *fill_from_bottom(struct item *last)
{
	struct item *p, *first;
	int col, h;
	int ignoring = 1;

	h = geo_get_itemarea_height() - config.item_margin_y;
	col = 1;
	first = last;
	list_for_each_entry_reverse(p, &menu.filter, filter) {
		if (p == last)
			ignoring = 0;
		if (ignoring)
			continue;
		h -= p->area.h + config.item_margin_y;
		if (h < 0) {
			col++;
			h = geo_get_itemarea_height() - config.item_margin_y;
			h -= p->area.h + config.item_margin_y;
		}
		if (col > config.columns)
			break;
		first = p;
	}
	return first;
}

static struct item *next_selectable(struct item *cur, int *isoutside)
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

static struct item *prev_selectable(struct item *cur, int *isoutside)
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

static struct item *first_selectable(void)
{
	int isoutside;
	struct item *item;

	item = filter_head();
	if (!item->selectable)
		item = next_selectable(item, &isoutside);
	return item;
}

static struct item *last_selectable(void)
{
	int isoutside;
	struct item *item;

	item = filter_tail();
	if (!item->selectable)
		item = prev_selectable(item, &isoutside);
	return item;
}

static void add_if_unique(struct item *item)
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

static int isvisible(struct item *item)
{
	struct item *p;

	if (!item)
		return 0;
	p = menu.first;
	list_for_each_entry_from(p, &menu.filter, filter) {
		if (p == item)
			return 1;
		if (p == menu.last)
			break;
	}
	return 0;
}

static void update_filtered_list(void)
{
	struct item *item;
	int isoutside;

	INIT_LIST_HEAD(&menu.filter);

	if (filter_needle_length()) {
		del_beyond_root();
		list_for_each_entry(item, &menu.master, master) {
			if (!strncmp("^checkout(", item->cmd, 10) ||
			    !strncmp("^tag(", item->cmd, 5) ||
			    !strncmp("^pipe(", item->cmd, 6) ||
			    !strncmp("^back(", item->cmd, 6) ||
			    !strncmp("^sep(", item->cmd, 5))
				continue;
			if (filter_ismatch(item->name) ||
			    filter_ismatch(item->cmd) ||
			    filter_ismatch(item->metadata))
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

	if (!filter_needle_length() && menu.current_node->last_first)
		menu.first = menu.current_node->last_first;
	else
		menu.first = filter_head();
	menu.last = fill_from_top(menu.first);

	/* select an item */
	if (!filter_needle_length() && menu.current_node->last_sel) {
		menu.sel = menu.current_node->last_sel;
		if (!isvisible(menu.sel))
			menu.sel = first_selectable();
	} else {
		menu.sel = menu.first;
		if (!menu.sel->selectable)
			menu.sel = next_selectable(menu.first, &isoutside);
		menu.current_node->last_sel = menu.sel;
	}
}

static void init_menuitem_coordinates(void)
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
static char *parse_caret_action(char *s, char *token)
{
	char *p;

	p = NULL;
	if (!s)
		return NULL;
	if (!strncmp(s, token, strlen(token))) {
		p = s;
		p += strlen(token);
	}
	return p;
}

static void draw_item_sep_without_text(struct item *p)
{
	double y;

	y = round(p->area.y + p->area.h / 2) + 0.5;
	ui_draw_line(p->area.x + 5, y, p->area.x + p->area.w - 5, y,
		     1.0, config.color_sep_fg);
}

static void draw_item_sep_with_text(struct item *p)
{
	struct sbuf s;
	int text_x_coord;

	sbuf_init(&s);
	if (config.sep_markup && config.sep_markup[0] != '\0') {
		sbuf_addstr(&s, "<span ");
		sbuf_addstr(&s, config.sep_markup);
		sbuf_addstr(&s, ">");
	}
	sbuf_addstr(&s, parse_caret_action(p->name, "^sep("));
	if (config.sep_markup && config.sep_markup[0] != '\0')
		sbuf_addstr(&s, "</span>");

	text_x_coord = p->area.x;
	if (config.sep_halign == LEFT)
		text_x_coord += config.item_padding_x;
	else if (config.sep_halign == RIGHT)
		text_x_coord -= config.item_padding_x;
	ui_draw_rectangle(p->area.x, p->area.y, p->area.w,
			  p->area.h, config.item_radius, 1.0, 1,
			  config.color_title_bg);
	ui_draw_rectangle(p->area.x, p->area.y, p->area.w,
			  p->area.h, config.item_radius, 1.0, 0,
			  config.color_title_border);
	ui_insert_text(s.buf, text_x_coord, p->area.y, p->area.h, p->area.w,
		       config.color_title_fg, config.sep_halign);
	xfree(s.buf);
}

static void draw_item_sep(struct item *p)
{
	if (p->name[strlen("^sep(")] == '\0')
		draw_item_sep_without_text(p);
	else
		draw_item_sep_with_text(p);
}

static void draw_item_bg_norm(struct item *p)
{
	ui_draw_rectangle(p->area.x, p->area.y, p->area.w, p->area.h,
			  config.item_radius, config.item_border, 1,
			  config.color_norm_bg);
}

static void draw_item_bg_sel(struct item *p)
{
	ui_draw_rectangle(p->area.x, p->area.y, p->area.w, p->area.h,
			  config.item_radius, config.item_border, 1,
			  config.color_sel_bg);
	if (!config.item_border)
		return;
	ui_draw_rectangle(p->area.x, p->area.y, p->area.w, p->area.h,
			  config.item_radius, config.item_border, 0,
			  config.color_sel_border);
}

static void draw_item_text(struct item *p)
{
	int text_x_coord, width;

	if (config.item_halign != RIGHT) {
		text_x_coord = p->area.x + config.item_padding_x;
		if (config.icon_size)
			text_x_coord += config.icon_size +
					config.icon_text_spacing;
	} else {
		text_x_coord = p->area.x - config.item_padding_x;
	}
	width = p->area.w - config.item_padding_x;
	if (config.icon_size)
		width -= config.icon_size + config.icon_text_spacing;

	if (p == menu.sel)
		ui_insert_text(p->name, text_x_coord, p->area.y,
			       p->area.h, width, config.color_sel_fg,
			       config.item_halign);
	else
		ui_insert_text(p->name, text_x_coord, p->area.y,
			       p->area.h, width, config.color_norm_fg,
			       config.item_halign);
}

static void draw_submenu_arrow(struct item *p)
{
	double *color;

	color = (p == menu.sel) ? config.color_sel_fg : config.color_norm_fg;
	if (config.item_halign != RIGHT)
		ui_insert_text(config.arrow_string, p->area.x + p->area.w -
			       config.item_padding_x - (config.arrow_width * 0.7), p->area.y,
			       p->area.h, p->area.w, color,
			       config.item_halign);
	else
		ui_insert_text(config.arrow_string, p->area.x + config.item_padding_x,
			       p->area.y, p->area.h, config.arrow_width * 0.7,
			       color, config.item_halign);
}

static void draw_icon(struct item *p, double alpha)
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
		ui_insert_image(p->icon, p->area.x + config.item_padding_x +
				offsetx, icon_y_coord, config.icon_size, alpha);
	else
		ui_insert_image(p->icon, p->area.x + p->area.w - config.icon_size -
				config.item_padding_x + offsetx - 1, icon_y_coord, config.icon_size, alpha);
}

static void draw_icon_norm(struct item *p)
{
	draw_icon(p, config.icon_norm_alpha / 100.0);
}

static void draw_icon_sel(struct item *p)
{
	draw_icon(p, config.icon_sel_alpha / 100.0);
}

static void draw_items_below_indicator(void)
{
	int b, r, l;

	if (!menu.current_node->parent) {
		b = config.menu_padding_bottom;
		r = config.menu_padding_right;
		l = config.menu_padding_left;
	} else {
		b = config.sub_padding_bottom;
		r = config.sub_padding_right;
		l = config.sub_padding_left;
	}
	if (b < 1)
		return;
	ui_draw_line(l, geo_get_menu_height() - b - 0.5, geo_get_menu_width() - r,
		     geo_get_menu_height() - b - 0.5, 1.0, config.color_scroll_ind);
}

static void draw_items_above_indicator(void)
{
	int t, r, l;

	if (!menu.current_node->parent) {
		t = config.menu_padding_top;
		r = config.menu_padding_right;
		l = config.menu_padding_left;
	} else {
		t = config.sub_padding_top;
		r = config.sub_padding_right;
		l = config.sub_padding_left;
	}
	if (t < 1)
		return;
	ui_draw_line(l, t + 0.5, geo_get_menu_width() - r, t + 0.5, 1.0,
		     config.color_scroll_ind);
}

static void draw_menu(void)
{
	struct item *p;
	int w;

	w = geo_get_menu_width();

	/* Draw background */
	ui_clear_canvas();
	if (config.menu_gradient_pos != ALIGNMENT_NONE) {
		ui_draw_rectangle_gradient(0, 0, w, geo_get_menu_height(), config.menu_radius,
				config.menu_border, 1, config.color_menu_bg, config.color_menu_bg_to, config.menu_gradient_pos);
	} else {
		ui_draw_rectangle(0, 0, w, geo_get_menu_height(), config.menu_radius,
				config.menu_border, 1, config.color_menu_bg);
	}


	/* Draw menu border */
	if (config.menu_border)
		ui_draw_rectangle(0, 0, w, geo_get_menu_height(),
				  config.menu_radius, config.menu_border,
				  0, config.color_menu_border);

	if (!ui->cur)
		widgets_draw();

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
					   !strncmp(p->cmd, "^pipe(", 6) ||
					   !strncmp(p->cmd, "^root(", 6) ||
					   !strncmp(p->cmd, "^sub(", 5)))
			draw_submenu_arrow(p);

		/* Draw menu items text */
		if (p->selectable)
			draw_item_text(p);
		else if (!strncmp(p->name, "^sep(", 5))
			draw_item_sep(p);

		/* Draw Icons */
		if (config.icon_size && p->icon) {
			if (p == menu.sel)
				draw_icon_sel(p);
			else
				draw_icon_norm(p);
		}

		if (p == menu.last)
			break;
	}
	if (filter_tail() != menu.last)
		draw_items_below_indicator();
	if (filter_head() != menu.first)
		draw_items_above_indicator();
	ui_map_window(geo_get_menu_width(), geo_get_menu_height());
}

static struct node *get_node_from_tag(const char *tag)
{
	struct node *n;

	if (!tag)
		return NULL;
	list_for_each_entry(n, &menu.nodes, node) {
		if (!strcmp(tag, n->item->tag))
			return n;
	}
	return NULL;
}

static int submenu_itemarea_width(void)
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

static void set_submenu_width(void)
{
	int mw;
	int reqw = geo_get_menu_width_from_itemarea_width(submenu_itemarea_width());
	struct point maxarea = geo_get_max_menuarea_that_fits();

	mw = reqw < maxarea.x ? reqw : maxarea.x;
	if (mw < config.menu_width)
		mw = config.menu_width;

	if (config.position_mode != POSITION_MODE_PTR)
		goto set_width;
	/* grow from right hand edge if too near it */
	if (config.menu_halign == LEFT && reqw > maxarea.x) {
		geo_set_menu_margin_x(geo_get_screen_x0() +
				      geo_get_screen_width() -
				      reqw);
		mw = reqw;
	}

set_width:
	geo_set_menu_width(mw);
}

static int submenu_itemarea_height(void)
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

static void set_submenu_height(void)
{
	int reqh = submenu_itemarea_height();
	struct point maxarea = geo_get_max_itemarea_that_fits();
	int h;

	h = reqh < maxarea.y ? reqh : maxarea.y;
	geo_set_menu_height_from_itemarea_height(h);
}

static int tag_exists(const char *tag)
{
	struct item *item;

	if (!tag)
		return 0;
	list_for_each_entry(item, &menu.master, master)
		if (item->tag && !strcmp(tag, item->tag))
			return 1;
	return 0;
}

static int tag_count(const char *tag)
{
	struct item *item;
	int n = 0;

	if (!tag)
		return 0;
	list_for_each_entry(item, &menu.master, master)
		if (item->tag && !strcmp(tag, item->tag))
			++n;
	return n;
}

static struct item *get_item_from_tag(const char *tag)
{
	struct item *item;

	BUG_ON(!tag);
	list_for_each_entry(item, &menu.master, master)
		if (item->tag && !strcmp(tag, item->tag))
			return item;
	if (tag && strncmp(tag, "root", 4))
		fprintf(stderr, "warning: could not find tag '%s'\n", tag);
	return NULL;
}

static char *tag_of_first_item(void)
{
	struct item *item;

	item = list_first_entry_or_null(&menu.master, struct item, master);
	if (!item)
		die("no items in master list");
	return item->tag;
}

static void find_subhead(const char *tag)
{
	BUG_ON(!tag);
	if (tag_exists(tag)) {
		menu.current_node = get_node_from_tag(tag);
	} else {
		warn("tag '%s' does not exist", tag);
		menu.current_node = get_node_from_tag(tag_of_first_item());
	}
	if (!menu.current_node)
		die("node '%s' does not exist", tag);
	menu.subhead = container_of((menu.current_node->item)->master.next,
				    struct item, master);
	if (!menu.subhead)
		die("no menu.subhead");
}

static void find_subtail(void)
{
	struct item *item;

	menu.subtail = menu.subhead;
	item = menu.subhead;
	list_for_each_entry_from(item, &menu.master, master) {
		if (item->tag)
			break;
		menu.subtail = item;
	}
}

static void checkout_tag(const char *tag)
{
	find_subhead(tag);
	find_subtail();
}

static void checkout_submenu(char *tag)
{
	if (!menu.sel) {
		warn("checkout_submenu(): no menu.sel");
		return;
	}
	if (geo_cur() >= MAX_NR_WINDOWS - 1) {
		warn("Maximum number of windows reached ('%d')", MAX_NR_WINDOWS);
		return;
	}
	checkout_tag(tag);
	geo_win_add(menu.sel->area);
	set_submenu_height();
	set_submenu_width();
	ui_win_add(geo_get_menu_x0(), geo_get_menu_y0(),
		   geo_get_menu_width(), geo_get_menu_height(),
		   geo_get_screen_width(), geo_get_screen_height(),
		   font_get());
	menu.current_node->wid = ui->w[ui->cur].win;
}

static void checkout_parentmenu(char *tag)
{
	checkout_tag(tag);
	/* If we've used ^root(tag), we're alreday at top window */
	if (!ui->cur)
		return;
	geo_win_del();
	ui_win_del();
}

static void checkout_rootmenu(char *tag)
{
	checkout_tag(tag);
	geo_set_cur(0);
}

/* This checks out the original root menu, regardless of any ^root() action */
static void checkout_rootnode(void)
{
	BUG_ON(!menu.current_node);
	if (!menu.current_node->parent)
		return;
	while (menu.current_node->parent)
		menu.current_node = menu.current_node->parent;
	checkout_rootmenu(menu.current_node->item->tag);
}

static void resize(void)
{
	XMoveResizeWindow(ui->dpy, ui->w[ui->cur].win, geo_get_menu_x0(),
			  geo_get_menu_y0(), geo_get_menu_width(),
			  geo_get_menu_height());
}

static void update(int resize_required)
{
	update_filtered_list();
	init_menuitem_coordinates();
	draw_menu();
	if (!resize_required)
		return;
	resize();
	ui_map_window(geo_get_menu_width(), geo_get_menu_height());
}

static void launch_menu_at_pointer(void)
{
	Window dw;
	int di;
	unsigned int du;
	struct point pos;
	struct area wa;

	geo_update_monitor_coords();
	XQueryPointer(ui->dpy, DefaultRootWindow(ui->dpy), &dw, &dw, &di, &di,
		      &pos.x, &pos.y, &du);
	if (pos.x < config.edge_snap_x)
		pos.x = 0;

	/* We use config.menu_{v,h}align to tell us where the panel is */
	if (config.menu_valign == TOP)
		wa.y = geo_get_screen_y0() + config.menu_margin_y;
	else
		wa.y = geo_get_screen_y0();
	wa.h = geo_get_screen_height() - config.menu_margin_y;

	if (config.menu_halign == LEFT)
		wa.x = geo_get_screen_x0() + config.menu_margin_x;
	else
		wa.x = geo_get_screen_x0();
	wa.w = geo_get_screen_width() - config.menu_margin_x;

	if (pos.x < wa.w + wa.x - geo_get_menu_width()) {
		geo_set_menu_halign(LEFT);
		if (config.menu_halign == LEFT && pos.x < config.menu_margin_x)
			pos.x = config.menu_margin_x;
		geo_set_menu_margin_x(pos.x);
	} else {
		geo_set_menu_halign(RIGHT);
		if (config.menu_halign == RIGHT && pos.x > wa.w)
			pos.x = wa.w;
		geo_set_menu_margin_x(geo_get_screen_width() - pos.x);
	}

	if (pos.y < wa.h + wa.y - geo_get_menu_height()) {
		geo_set_menu_valign(TOP);
		if (config.menu_valign == TOP && pos.y < config.menu_margin_y)
			pos.y = config.menu_margin_y;
		geo_set_menu_margin_y(pos.y);
	} else if (geo_get_menu_height() < pos.y) {
		geo_set_menu_valign(BOTTOM);
		if (config.menu_valign == BOTTOM && pos.y > wa.h)
			pos.y = wa.h;
		geo_set_menu_margin_y(geo_get_screen_height() - pos.y);
	} else {
		geo_set_menu_valign(BOTTOM);
		if (config.menu_valign == BOTTOM)
			geo_set_menu_margin_y(config.menu_margin_y);
		else
			geo_set_menu_margin_y(0);
	}
	set_submenu_width();
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
		spawn_async("jgmenu_run unity-hack", NULL);
}

static void awake_menu(void)
{
	menu_is_hidden = 0;
	if_unity_run_hack();
	hooks_check();
	if (config.position_mode == POSITION_MODE_PTR) {
		launch_menu_at_pointer();
		resize();
	}
	ipc_read_socket();
	if (config.position_mode == POSITION_MODE_IPC) {
		ipc_align_based_on_env_vars();
		update(1);
	}
	XMapWindow(ui->dpy, ui->w[ui->cur].win);
	ui_map_window(geo_get_menu_width(), geo_get_menu_height());
	XRaiseWindow(ui->dpy, ui->w[ui->cur].win);
	grabkeyboard();
	grabpointer();
}

static int node_exists(const char *name)
{
	struct node *n;

	BUG_ON(!name);
	list_for_each_entry(n, &menu.nodes, node)
		if (!strcmp(name, n->item->tag))
			return 1;
	return 0;
}

static void create_node(const char *name, struct node *parent)
{
	struct node *n;

	n = xmalloc(sizeof(struct node));
	BUG_ON(!name);
	n->item = get_item_from_tag(name);
	n->last_sel = NULL;
	n->last_first = NULL;
	n->expanded = NULL;
	n->parent = parent;
	n->wid = 0;
	list_add_tail(&n->node, &menu.nodes);
}

/* Create nodal tree from tagged items */
static struct node *node_add_new(struct item *this, struct node *parent)
{
	struct item *child, *p;
	struct node *current_node;

	BUG_ON(!this);
	BUG_ON(!this->tag);
	if (tag_count(this->tag) > 1)
		die("duplicate tag (%s)", this->tag);
	create_node(this->tag, parent);
	/* move to next item, as this points to a ^tag() item */
	p = container_of((this)->master.next, struct item, master);
	/* p now points to first menu-item under tag "this->tag" */

	current_node = list_last_entry(&menu.nodes, struct node, node);

	/* walk the items under current node and put into tree structure */
	list_for_each_entry_from(p, &menu.master, master) {
		if (!strncmp("^checkout(", p->cmd, 10)) {
			child = get_item_from_tag(parse_caret_action(p->cmd, "^checkout("));
			if (!child)
				continue;
			if (child->tag && node_exists(child->tag))
				continue;
			node_add_new(child, current_node);
		} else if (!strncmp("^tag(", p->cmd, 5)) {
			break;
		}
	}
	return current_node;
}

static void destroy_node_tree(void)
{
	struct node *n, *tmp_n;

	list_for_each_entry_safe(n, tmp_n, &menu.nodes, node) {
		list_del(&n->node);
		xfree(n);
	}
}

static void build_tree(void)
{
	struct item *item;
	struct node *n, *root_node;

	BUG_ON(list_empty(&menu.master));
	item = list_first_entry_or_null(&menu.master, struct item, master);
	BUG_ON(!item->tag);
	BUG_ON(list_is_singular(&menu.master));

	root_node = node_add_new(get_item_from_tag(item->tag), NULL);

	/*
	 * Add any remaining ^tag()s - i.e. those without a corresponding
	 * ^checkout(). These are added to the top-level node.
	 */
	list_for_each_entry(item, &menu.master, master) {
		BUG_ON(!item);
		if (!item->tag)
			continue;
		list_for_each_entry(n, &menu.nodes, node)
			if (n->item == item)
				goto already_exists;
		create_node(item->tag, root_node);
already_exists:
		;
	}
}

static void remove_checkouts_without_matching_tags(void)
{
	struct item *i, *tmp;

	list_for_each_entry_safe(i, tmp, &menu.master, master) {
		if (!strncmp(i->cmd, "^checkout(", 10) &&
		    !tag_exists(i->cmd + 10)) {
			info("remove (%s) as it has no matching tag", i->cmd);
			xfree(i->buf);
			list_del(&i->master);
			xfree(i);
		}
	}
}

/*
 * The first item of a rootmenu or pipe must be a ^tag() item. If the user has not
 * provided it, we tag care of it here.
 */
#define UTAG_BIG_NR (99999)
#define UTAG_BUFSIZ (18)
static void get_unique_tag_item(char *utag)
{
	int i;

	for (i = 0; i < UTAG_BIG_NR; i++) {
		snprintf(utag, UTAG_BUFSIZ, "%d", i);
		if (!node_exists(utag))
			break;
	}
	snprintf(utag, UTAG_BUFSIZ, "%d,^tag(%d", i, i);
}

static void insert_tag_item(void)
{
	struct item *item = NULL;
	struct argv_buf argv_buf;
	char utag[UTAG_BUFSIZ];

	get_unique_tag_item(utag);
	argv_set_delim(&argv_buf, ',');
	argv_init(&argv_buf);
	argv_strdup(&argv_buf, utag);
	argv_parse(&argv_buf);
	item = xmalloc(sizeof(struct item));
	item->buf = argv_buf.buf;
	item->name = argv_buf.argv[0];
	item->cmd = argv_buf.argv[1];
	item->iconname = NULL;
	item->working_dir = NULL;
	item->metadata = NULL;
	item->icon = NULL;
	item->tag = item->cmd + 5;
	item->selectable = 1;
	item->area.h = config.item_height;
	list_add_tail(&item->master, &menu.master);
}

static void resolve_newline(char *s)
{
	char *p;

	if (!s)
		return;
	p = strstr(s, "\\n");
	if (!p)
		return;
	*p = ' ';
	*(p + 1) = '\n';
}

/**
 * read_csv_file - read lines from FILE to "master" list
 * @fp: file to be read
 * @ispipemenu: ensure first item is ^tag(...) for pipemenus
 *
 * Return number of lines read
 */
static int read_csv_file(FILE *fp, bool ispipemenu)
{
	char buf[BUFSIZ], *p;
	size_t i;
	struct item *item = NULL;
	struct argv_buf argv_buf;
	static bool first_item = true;

	if (!fp)
		die("no csv-file");
	if (ispipemenu)
		first_item = true;
	argv_set_delim(&argv_buf, ',');
	for (i = 0; fgets(buf, sizeof(buf), fp); i++) {
		buf[BUFSIZ - 1] = '\0';
		if (strlen(buf) == BUFSIZ - 1)
			die("item %d is too long", i);
		p = strrchr(buf, '\n');
		if (p)
			*p = '\0';
		else
			die("item %d was not correctly terminated with a '\\n'", i);
		if (!utf8_validate(buf, p - &buf[0])) {
			warn("line not utf-8 compatible: '%s'", buf);
			i--;
			continue;
		}
		if ((buf[0] == '#') ||
		    (buf[0] == '\n') ||
		    (buf[0] == '\0')) {
			i--;
			continue;
		} else if (buf[0] == '@') {
			widgets_add(buf);
			continue;
		} else if (buf[0] == '.' && buf[1] == ' ') {
			FILE *include_file;
			struct sbuf filename;

			sbuf_init(&filename);
			sbuf_cpy(&filename, buf + 1);
			sbuf_trim(&filename);
			sbuf_expand_tilde(&filename);
			include_file = fopen(filename.buf, "r");
			if (include_file) {
				read_csv_file(include_file, false);
				fclose(include_file);
			}
			xfree(filename.buf);
			continue;
		}
		argv_init(&argv_buf);
		argv_strdup(&argv_buf, buf);
		argv_parse(&argv_buf);
		item = xmalloc(sizeof(struct item));
		item->buf = argv_buf.buf;
		item->name = argv_buf.argv[0];
		resolve_newline(item->name);
		item->cmd = argv_buf.argv[1];
		item->iconname = argv_buf.argv[2];
		item->working_dir = argv_buf.argv[3];
		item->metadata = argv_buf.argv[4];
		remove_caret_markup_closing_bracket(item->name);
		remove_caret_markup_closing_bracket(item->cmd);
		if (!item->cmd)
			item->cmd = item->name;
		if (first_item) {
			if (item->cmd && strncmp(item->cmd, "^tag(", 5))
				insert_tag_item();
			first_item = false;
		}
		item->icon = NULL;
		if (!strncmp("^tag(", item->cmd, 5))
			item->tag = parse_caret_action(item->cmd, "^tag(");
		else
			item->tag = NULL;
		item->selectable = 1;
		item->area.h = config.item_height;
		if (!strncmp(item->name, "^sep(", 5)) {
			item->selectable = 0;
			if (item->name[5] == '\0')
				item->area.h = config.sep_height;
		}
		list_add_tail(&item->master, &menu.master);
	}

	return i;
}

static void rm_back_items(void)
{
	struct item *i, *tmp;

	list_for_each_entry_safe(i, tmp, &menu.master, master) {
		if (!strncmp(i->cmd, "^back(", 6)) {
			xfree(i->buf);
			list_del(&i->master);
			xfree(i);
		}
	}
}

static int is_ancestor_to_current_node(struct node *node)
{
	struct node *n;

	if (!node)
		return 0;
	n = menu.current_node;
	while (n->parent) {
		n = n->parent;
		if (n == node)
			return 1;
	}
	return 0;
}

static void recalc_expanded_nodes(void)
{
	struct node *n;

	menu.current_node->expanded = NULL;
	list_for_each_entry(n, &menu.nodes, node)
		if (!is_ancestor_to_current_node(n))
			n->expanded = NULL;
}

/* Delete windows and pipemenus beyond current node */
static void del_beyond_current(void)
{
	ui_win_del_beyond(ui->cur);
	pipemenu_del_beyond(menu.current_node);
	recalc_expanded_nodes();
}

static void del_beyond_root(void)
{
	ui_win_del_beyond(0);
	geo_set_cur(0);
	checkout_rootnode();	/* original root node */
	pipemenu_del_all();
}

static int check_pipe_tags_unique(struct item *from)
{
	struct item *p;

	p = from;
	list_for_each_entry_from(p, &menu.master, master) {
		BUG_ON(!p);
		if (!p->tag)
			continue;
		if (node_exists(p->tag)) {
			warn("tag (%s) already exists", p->tag);
			return -1;
		}
	}
	return 0;
}

static void destroy_master_list_from(struct item *from)
{
	struct item *i, *i_tmp;

	i = from;
	list_for_each_entry_safe_from(i, i_tmp, &menu.master, master) {
		xfree(i->buf);
		list_del(&i->master);
		xfree(i);
	}
}

static void pipemenu_add(const char *s)
{
	FILE *fp = NULL;
	struct item *pipe_head;
	struct node *parent_node;
	int nr_lines;

	BUG_ON(!s);
	fp = popen(s, "r");
	if (!fp) {
		warn("could not open pipe '%s'", s);
		return;
	}

	pipe_head = list_last_entry(&menu.master, struct item, master);
	nr_lines = read_csv_file(fp, true);
	if (fp && fp != stdin)
		pclose(fp);
	if (!nr_lines) {
		warn("empty pipemenu");
		return;
	}
	pipe_head = container_of(pipe_head->master.next, struct item, master);
	/* pipe_head now points to first item of pipe */

	if (check_pipe_tags_unique(pipe_head) < 0) {
		destroy_master_list_from(pipe_head);
		info("pipe menu removed");
		return;
	}

	if (config.hide_back_items)
		rm_back_items();
	parent_node = menu.current_node;
	remove_checkouts_without_matching_tags();
	node_add_new(pipe_head, parent_node);
	checkout_submenu(pipe_head->tag);
	pm_push(menu.current_node, parent_node);
}

/**
 * pipemenu_del_from - remove nodes and menu items associated with pipemenu
 * @node: delete pipemenu from this point onwards
 * Note: 'node' needs to be within a pipemenu
 */
static void pipemenu_del_from(struct node *node)
{
	struct node *n_tmp;

	pm_pop();
	destroy_master_list_from(node->item);
	list_for_each_entry_safe_from(node, n_tmp, &menu.nodes, node) {
		list_del(&node->node);
		xfree(node);
	}
}

static void pipemenu_del_beyond(struct node *keep_me)
{
	struct node *n, *n_tmp;

	list_for_each_entry_safe_reverse(n, n_tmp, &menu.nodes, node) {
		if (n == keep_me)
			break;
		/* we can't remove nodes within a pipemenu */
		if (pm_is_pipe_node(n))
			pipemenu_del_from(n);
	}
}

static void pipemenu_del_all(void)
{
	struct node *n;

	n = (struct node *)pm_first_pipemenu_node();
	if (!n)
		return;
	pipemenu_del_from(n);
	pm_cleanup();
}

static void checkout_parent(void)
{
	struct node *parent;

	if (!menu.current_node->parent)
		return;
	parent = menu.current_node->parent;
	if (pm_is_pipe_node(menu.current_node))
		pipemenu_del_from(menu.current_node);
	/* reverting to parent following ^root() */
	if (!parent->wid) {
		parent->wid = menu.current_node->wid;
		menu.current_node->wid = 0;
	}
	checkout_parentmenu(parent->item->tag);
	if (config.menu_height_mode == CONFIG_DYNAMIC)
		set_submenu_height();
}

static void clear_self_pipe(void)
{
	fd_set readfds;

	if (!FD_ISSET(pipe_fds[0], &readfds))
		return;
	for (;;) {
		char ch;

		if (read(pipe_fds[0], &ch, 1) == -1) {
			if (errno == EAGAIN)
				break;
			warn("error reading pipe");
		}
	}
}

static void hide_menu(void)
{
	tmr_mouseover_stop();
	del_beyond_root();
	XUnmapWindow(ui->dpy, ui->w[ui->cur].win);
	filter_reset();
	XUngrabKeyboard(ui->dpy, CurrentTime);
	XUngrabPointer(ui->dpy, CurrentTime);
	menu.current_node->expanded = NULL;
	menu.sel = NULL;
	update(1);
	clear_self_pipe();
	menu_is_hidden = 1;
}

static void hide_or_exit(void)
{
	if (config.persistent > 0)
		return;
	if (config.stay_alive)
		hide_menu();
	else
		exit(0);
}

static void action_cmd(char *cmd, const char *working_dir)
{
	char *p = NULL;

	if (!cmd)
		return;
	if (!config.spawn && strncmp("^checkout(", cmd, 10) &&
	    strncmp("^sub(", cmd, 5) && strncmp("^back(", cmd, 6) &&
	    strncmp("^pipe(", cmd, 6)) {
		printf("%s\n", cmd);
		exit(0);
	}
	if (!strncmp(cmd, "^checkout(", 10)) {
		p = parse_caret_action(cmd, "^checkout(");
		if (!p)
			return;
		if (!tag_exists(p))
			return;
		menu.current_node->last_sel = menu.sel;
		menu.current_node->last_first = menu.first;
		checkout_submenu(p);
		update(1);
	} else if (!strncmp(cmd, "^sub(", 5)) {
		p = parse_caret_action(cmd, "^sub(");
		if (!p)
			return;
		spawn_async(p, working_dir);
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
		spawn_async(s.buf, working_dir);
		free(s.buf);
		hide_or_exit();
	} else if (!strncmp(cmd, "^pipe(", 6)) {
		p = parse_caret_action(cmd, "^pipe(");
		if (!p)
			return;
		menu.current_node->last_sel = menu.sel;
		menu.current_node->last_first = menu.first;
		pipemenu_add(p);
		update(1);
	} else if (!strncmp(cmd, "^root(", 6)) {
		/* Two nodes with the same wid breaks get_node_from_wid() */
		if (!tag_exists(cmd + 6))
			return;
		menu.current_node->last_sel = menu.sel;
		menu.current_node->last_first = menu.first;
		menu.current_node->wid = 0;
		del_beyond_root();
		filter_reset();
		checkout_tag(cmd + 6);
		menu.current_node->wid = ui->w[ui->cur].win;
		if (config.menu_height_mode == CONFIG_DYNAMIC) {
			set_submenu_height();
			update(1);
		} else {
			update(0);
		}
	} else if (!strncmp(cmd, "^filter(", 8)) {
		p = parse_caret_action(cmd, "^filter(");
		if (!p)
			return;
		filter_reset();
		filter_set_clear_on_keyboard_input(1);
		filter_addstr(p, strlen(p));
		update(1);
	} else if (!strncmp(cmd, "^quit(", 6)) {
		exit(0);
	} else {
		spawn_async(cmd, working_dir);
		hide_or_exit();
	}
}

static struct point mousexy(void)
{
	Window dw;
	int di;
	unsigned int du;
	struct point coords;

	XQueryPointer(ui->dpy, ui->w[ui->cur].win, &dw, &dw, &di, &di, &coords.x,
		      &coords.y, &du);

	return coords;
}

static void key_event(XKeyEvent *ev)
{
	char buf[32];
	int len;
	KeySym ksym = NoSymbol;
	Status status;
	int isoutside;

	len = Xutf8LookupString(ui->w[ui->cur].xic, ev, buf, sizeof(buf), &ksym, &status);
	if (status == XBufferOverflow)
		return;
	if (ui_has_child_window_open(menu.current_node->wid))
		del_beyond_current();

	/* menu.sel == NULL could be caused by pointer movement */
	if (!menu.sel)
		menu.sel = menu.current_node->last_sel;

	switch (ksym) {
	case XK_Tab:
		widgets_toggle_kb_grabbed();
		break;
	case XK_End:
		if (filter_head() == &empty_item)
			break;
		menu.last = filter_tail();
		menu.first = fill_from_bottom(menu.last);
		menu.sel = last_selectable();
		init_menuitem_coordinates();
		menu.current_node->last_sel = menu.sel;
		draw_menu();
		break;
	case XK_Super_L:
	case XK_Super_R:
		super_key_pressed = 1;
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
		menu.sel = first_selectable();
		init_menuitem_coordinates();
		menu.current_node->last_sel = menu.sel;
		draw_menu();
		break;
	case XK_Up:
		if (widgets_get_kb_grabbed()) {
			widgets_select("XK_Up");
			draw_menu();
			action_cmd(widgets_get_selection_action(), NULL);
			break;
		}
		if (filter_head() == &empty_item)
			break;
		if (menu.sel == first_selectable()) {
			/* bounce to bottom */
			menu.last = filter_tail();
			menu.first = fill_from_bottom(menu.last);
			menu.sel = last_selectable();
		} else {
			menu.sel = prev_selectable(menu.sel, &isoutside);
			if (isoutside) {
				menu.first = menu.sel;
				menu.last = fill_from_top(menu.first);
			}
		}
		init_menuitem_coordinates();
		menu.current_node->last_sel = menu.sel;
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
		menu.current_node->last_sel = menu.sel;
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
		menu.current_node->last_sel = menu.sel;
		draw_menu();
		break;
	case XK_Return:
	case XK_KP_Enter:
		if (menu.sel->selectable)
			action_cmd(menu.sel->cmd, menu.sel->working_dir);
		break;
	case XK_Down:
		if (widgets_get_kb_grabbed()) {
			widgets_select("XK_Down");
			draw_menu();
			action_cmd(widgets_get_selection_action(), NULL);
			break;
		}
		if (filter_head() == &empty_item)
			break;
		if (menu.sel == last_selectable()) {
			menu.first = filter_head();
			menu.last = fill_from_top(menu.first);
			menu.sel = first_selectable();
		} else {
			menu.sel = next_selectable(menu.sel, &isoutside);
			if (isoutside) {
				menu.last = menu.sel;
				menu.first = fill_from_bottom(menu.last);
			}
		}
		init_menuitem_coordinates();
		menu.current_node->last_sel = menu.sel;
		draw_menu();
		break;
	case XK_Left:
		if (!menu.current_node->parent)
			break;
		checkout_parent();
		update(1);
		break;
	case XK_Right:
		if (!strncmp(menu.sel->cmd, "^checkout(", 10) ||
		    !strncmp(menu.sel->cmd, "^root(", 6) ||
		    !strncmp(menu.sel->cmd, "^pipe(", 6))
			action_cmd(menu.sel->cmd, NULL);
		break;
	case XK_F5:
		restart();
		break;
	case XK_F7:
		break;
	case XK_F8:
		print_nodes();
		break;
	case XK_F9:
		/* Useful for stopping Makefile during tests/ or examples/ */
		exit(1);
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
		if (filter_get_clear_on_keyboard_input())
			filter_reset();
		filter_addstr(buf, len);
		update(0);
		break;
	}
}

static int is_outside_menu_windows(XMotionEvent **e)
{
	struct node *n;

	if (!(*e)->subwindow)
		return 1;
	list_for_each_entry(n, &menu.nodes, node)
		if ((*e)->subwindow == n->wid)
			return 0;
	return 1;
}

/* Pointer vertical offset (not sure why this is needed) */
#define MOUSE_FUDGE 3

static void mouse_release(XEvent *e)
{
	XButtonReleasedEvent *ev;
	struct point mouse_coords;

	mouse_coords = mousexy();
	mouse_coords.y -= MOUSE_FUDGE;
	ev = &e->xbutton;

	/* scroll up */
	if (ev->button == Button4 && menu.first != filter_head()) {
		if (!menu.sel)
			menu.sel = menu.current_node->last_sel;
		if (ui_has_child_window_open(menu.current_node->wid))
			del_beyond_current();
		step_back(&menu.first, 1);
		menu.last = fill_from_top(menu.first);
		step_back(&menu.sel, 1);
		init_menuitem_coordinates();
		menu.current_node->last_sel = menu.sel;
		draw_menu();
		return;
	}

	/* scroll down */
	if (ev->button == Button5 && menu.last != filter_tail()) {
		if (!menu.sel)
			menu.sel = menu.current_node->last_sel;
		if (ui_has_child_window_open(menu.current_node->wid))
			del_beyond_current();
		step_fwd(&menu.last, 1);
		menu.first = fill_from_bottom(menu.last);
		step_fwd(&menu.sel, 1);
		init_menuitem_coordinates();
		menu.current_node->last_sel = menu.sel;
		draw_menu();
		return;
	}

	/* left-click */
	if (ev->button == Button1) {
		char *ret;

		/* widgets */
		widgets_set_pointer_position(mouse_coords.x, mouse_coords.y);
		ret = widgets_get_selection_action();
		if (ret && ret[0] != '\0') {
			action_cmd(ret, NULL);
			return;
		}

		/* normal menu items */
		if (!menu.sel)
			return;
		if (!menu.sel->selectable)
			return;
		if (config.sub_hover_action &&
		    (!strncmp(menu.sel->cmd, "^checkout(", 10) ||
		     !strncmp(menu.sel->cmd, "^pipe(", 6)))
			return;
		if (ui_has_child_window_open(menu.current_node->wid))
			del_beyond_current();
		action_cmd(menu.sel->cmd, menu.sel->working_dir);
	}
}

static int mouse_outside(XEvent *e)
{
	XMotionEvent *xme;
	XButtonReleasedEvent *ev;
	struct point mouse_coords;
	int outside_menu_windows = 0;

	mouse_coords = mousexy();
	mouse_coords.y -= MOUSE_FUDGE;

	xme = (XMotionEvent *)e;
	if (is_outside_menu_windows(&xme))
		outside_menu_windows = 1;

	ev = &e->xbutton;

	/* left/right-click outside menu windows */
	if ((ev->button == Button1 || ev->button == Button3) &&
	    outside_menu_windows)
		return 1;
	return 0;
}

static double timespec_to_sec(struct timespec *ts)
{
	return (double)ts->tv_sec + (double)ts->tv_nsec / 1000000000.0;
}

/*
 * This function is loaded in the background under a new pthread
 * X11 is not thread-safe, so load_icons() must not call any X functions.
 */
static void *load_icons(void *arg)
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

static void destroy_master_list(void)
{
	struct item *item, *tmp_item;

	list_for_each_entry_safe(item, tmp_item, &menu.master, master) {
		xfree(item->buf);
		list_del(&item->master);
		xfree(item);
	}
}

static void init_pipe_flags(void)
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

static void move_selection_with_mouse(struct point *mouse_coord)
{
	struct item *item;

	item = menu.first;
	list_for_each_entry_from(item, &menu.filter, filter) {
		if (!item->selectable)
			continue;
		if (ui_is_point_in_area(*mouse_coord, item->area)) {
			if (menu.sel != item) {
				menu.sel = item;
				menu.current_node->last_sel = item;
				draw_menu();
			}
			return;
		}
		if (item == menu.last)
			break;
	}
	/* if we got this far, the mouse is not over a menu item */

	if (!menu.current_node->expanded && menu.sel) {
		menu.sel = NULL;
		draw_menu();
	}
	if (widgets_mouseover())
		draw_menu();
}

static void mouseover_handler(int sig)
{
	int saved_errno;

	saved_errno = errno;
	if (write(pipe_fds[1], "t", 1) == -1)
		die("error writing to pipe for onmouseover timer");
	errno = saved_errno;
}

static void tmr_mouseover_init(void)
{
	if (signal(SIGALRM, (void *)mouseover_handler) == SIG_ERR)
		die("SIGALRM action");
}

static void tmr_mouseover_set(int msec)
{
	struct itimerval it;
	static int run_once;

	if (msec >= 1000)
		msec = 999;
	if (!run_once) {
		tmr_mouseover_init();
		run_once = 1;
	}
	it.it_value.tv_sec = 0;
	it.it_value.tv_usec = msec * 1000;
	it.it_interval.tv_sec = 0;
	it.it_interval.tv_usec = 0;
	/* As timer_*() are not supported on OpenBSD, we use setitimer() */
	if (setitimer(ITIMER_REAL, &it, NULL) == -1)
		die("setitimer()");
}

static void tmr_mouseover_start(void)
{
	tmr_mouseover_set(config.hover_delay);
}

static void tmr_mouseover_stop(void)
{
	tmr_mouseover_set(0);
}

static struct node *get_node_from_wid(Window w)
{
	struct node *n;

	if (!w)
		return NULL;
	list_for_each_entry(n, &menu.nodes, node)
		if (w == n->wid)
			return n;
	return NULL;
}

static void close_sub_window(void)
{
	if (!ui_has_child_window_open(menu.current_node->wid))
		return;
	sw_close_pending = 1;
	tmr_mouseover_start();
}

static void hover(void)
{
	/*
	 * Mouse is over an already "expanded" item (i.e. one that caused a
	 * sub window to open
	 */
	if (menu.sel == menu.current_node->expanded) {
		tmr_mouseover_stop();
		return;
	}
	if (menu.sel && (!strncmp(menu.sel->cmd, "^checkout(", 10) ||
			 !strncmp(menu.sel->cmd, "^pipe(", 6))) {
		tmr_mouseover_start();
		sw_close_pending = 0;
		return;
	}

	/* non-submenu item */
	if (!sw_close_pending) {
		tmr_mouseover_stop();
		close_sub_window();
	}
	if (widgets_mouseover()) {
		tmr_mouseover_stop();
		close_sub_window();
		return;
	}
}

static void set_focus(Window w)
{
	struct node *n;
	int ret = 0;

	n = get_node_from_wid(w);
	menu.current_node->last_sel = menu.sel;
	ret = ui_win_activate(w);
	if (ret < 0)
		return;
	geo_set_cur(ui->cur);
	checkout_tag(n->item->tag);
	menu.sel = n->last_sel;
	menu.current_node = n;
}

static void adjust_selection_and_redraw(void)
{
	if (menu.current_node->expanded) {
		/*
		 * When moving the pointer diagonally from a parent to
		 * a child window, the correct menu.sel needs to be set
		 * and redrawn in the parent window.
		 */
		menu.sel = menu.current_node->expanded;
		menu.current_node->last_sel = menu.sel;
	} else {
		/*
		 * Don't show selection in submenu if we are not
		 * actually over the window
		 */
		menu.sel = NULL;
	}
	draw_menu();
}

static void process_pointer_position(XEvent *ev, int force)
{
	struct point pw;
	static int oldy;
	static int oldx;
	XMotionEvent *e = (XMotionEvent *)ev;

	/*
	 * We get the mouse coordinates using XQueryPointer() as
	 * ev.xbutton.{x,y} sometimes returns peculiar values.
	 * If we ever need the coordinates relative to the root window, use
	 * e->x_root and e->y_root
	 */
	pw = mousexy();
	pw.y -= MOUSE_FUDGE;
	if (!force && (pw.x == oldx) && (pw.y == oldy))
		return;

	widgets_set_pointer_position(pw.x, pw.y);

	/* e->subwindow refers to the window under the pointer */
	if (e->subwindow == ui->w[ui->cur].win) {
		move_selection_with_mouse(&pw);
		if (config.sub_hover_action)
			hover();
	} else if (is_outside_menu_windows(&e)) {
		adjust_selection_and_redraw();
		tmr_mouseover_stop();
	} else {
		/*
		 * We end up here whenever we move from one window to another.
		 *
		 * When a sub window has just opened, we end up here on
		 * the next event (once) and re-set the focus on the 'parent'
		 * window.
		 */
		adjust_selection_and_redraw();
		set_focus(e->subwindow);
		update(1);
	}
	oldx = pw.x;
	oldy = pw.y;
}

static void signal_handler(int sig)
{
	int saved_errno;

	saved_errno = errno;
	if (write(pipe_fds[1], "1", 1) == -1 && errno != EAGAIN)
		die("write");
	errno = saved_errno;
}

static void run(void)
{
	XEvent ev;
	struct item *item;

	int ready, nfds, x11_fd;
	fd_set readfds;
	static int all_icons_have_been_requested;
	struct sigaction sa;

	/* for performance testing */
	if (args_die_when_loaded() && !config.icon_size)
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

	if (config.hide_on_startup)
		hide_menu();

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
				char ch;

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

				/* mouse over signal */
				if (ch == 't') {
					if (!menu.sel)
						continue;
					del_beyond_current();
					/* open new sub window */
					if (!sw_close_pending && !menu_is_hidden) {
						menu.current_node->expanded = menu.sel;
						action_cmd(menu.sel->cmd,
							   menu.sel->working_dir);
					}
					sw_close_pending = 0;
					process_pointer_position(&ev, 1);
					continue;
				}

				/* 'x' means that icons have finished loading */
				if (ch != 'x')
					continue;

				/* for performance testing */
				if (args_die_when_loaded() && all_icons_have_been_requested)
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
			static int close_pending;

			XNextEvent(ui->dpy, &ev);

			/* UTF-8 support */
			if (XFilterEvent(&ev, ui->w[ui->cur].win))
				continue;

			switch (ev.type) {
			case MappingNotify:
				XRefreshKeyboardMapping(&ev.xmapping);
				break;
			case ButtonRelease:
				if (close_pending) {
					close_pending = 0;
					hide_or_exit();
					break;
				}
				mouse_release(&ev);
				break;
			case ButtonPress:
				/*
				 * tint2 buttons/execps take action on
				 * "ButtonRelease". We want to be able to use
				 * these to both open and close the menu.
				 * When passing mouse events through tint2 to
				 * the WM, we want menu to be able to respond
				 * to "ButtonPress" without immediately dying
				 * on "ButtonRelease".
				 */
				if (mouse_outside(&ev))
					close_pending = 1;
				break;
			case KeyRelease:
				if (super_key_pressed) {
					super_key_pressed = 0;
					/* avoid passing super key to WM */
					msleep(30);
					hide_or_exit();
				}
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
					XRaiseWindow(ui->dpy, ui->w[ui->cur].win);
				break;
			case MotionNotify:
				process_pointer_position(&ev, 0);
				break;
			}
		}
	}
}

static void init_geo_variables_from_config(void)
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
	geo_set_sub_spacing(config.sub_spacing);
	geo_set_sub_padding_top(config.sub_padding_top);
	geo_set_sub_padding_right(config.sub_padding_right);
	geo_set_sub_padding_bottom(config.sub_padding_bottom);
	geo_set_sub_padding_left(config.sub_padding_left);
	geo_set_item_margin_x(config.item_margin_x);
	geo_set_item_margin_y(config.item_margin_y);
	geo_set_item_height(config.item_height);
}

static void set_font(void)
{
	font_set();
	info("set font to '%s'", font_get());
}

static void set_theme(void)
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
	xfree(theme.buf);
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
}

static void read_tint2rc(void)
{
	char *t;
	struct sbuf bl;

	sbuf_init(&bl);
	t = getenv("TINT2_CONFIG");
	if (t) {
		t2conf_parse(t, geo_get_screen_width(),
			     geo_get_screen_height());
		goto out;
	}

	bl_tint2file(&bl);
	if (bl.len) {
		info("using BunsenLabs tint2 session file");
		t2conf_parse(bl.buf, geo_get_screen_width(),
			     geo_get_screen_height());
	} else {
		t2conf_parse(NULL, geo_get_screen_width(),
			     geo_get_screen_height());
	}
out:
	free(bl.buf);
}

static void init_locale(void)
{
	static bool have_alread_tried_lang_c;
	char *lang = getenv("LANG");

	if (!setlocale(LC_ALL, ""))
		warn("setlocale(): locale not supported by C library; using 'C' locale");
	if (!XSupportsLocale()) {
		warn("XSupportsLocale(): error setting locale %s", lang);
		goto fallback;
	}
	if (!XSetLocaleModifiers("@im=none")) {
		warn("XSetLocaleModifiers(): error setting locale %s", lang);
		goto fallback;
	}
	return;

fallback:
	if (have_alread_tried_lang_c)
		return;
	info("fallback to LANG=C");
	setenv("LANG", "C", 1);
	have_alread_tried_lang_c = true;
	init_locale();
}

static void init_sigactions(void)
{
	struct sigaction term_action, int_action;

	memset(&term_action, 0, sizeof(struct sigaction));
	memset(&int_action, 0, sizeof(struct sigaction));
	term_action.sa_handler = quit;
	int_action.sa_handler = quit;
	sigaction(SIGTERM, &term_action, NULL);
	sigaction(SIGINT, &int_action, NULL);
}

static void cleanup(void)
{
	ui_cleanup();
	config_cleanup();
	filter_cleanup();
	font_cleanup();
	if (config.icon_size)
		icon_cleanup();
	widgets_cleanup();
	hooks_cleanup();
	t2conf_atexit();
	delete_empty_item();
	destroy_node_tree();
	destroy_master_list();
}

static void keep_menu_height_between_min_and_max(void)
{
	if (config.menu_height_min &&
	    geo_get_menu_height() < config.menu_height_min)
		geo_set_menu_height(config.menu_height_min);
	if (config.menu_height_max &&
	    geo_get_menu_height() > config.menu_height_max)
		geo_set_menu_height(config.menu_height_max);
}

static FILE *get_csv_source(bool arg_vsimple)
{
	if (args_csv_file()) {
		struct sbuf s;
		FILE *fp;

		sbuf_init(&s);
		sbuf_cpy(&s, args_csv_file());
		sbuf_expand_tilde(&s);
		fp = fopen(s.buf, "r");
		xfree(s.buf);
		return fp;
	}
	if (args_csv_cmd())
		return popen(args_csv_cmd(), "r");
	if (args_simple() || arg_vsimple)
		return stdin;
	if (config.csv_cmd && config.csv_cmd[0] != '\0')
		return popen(config.csv_cmd, "r");
	return stdin;
}

static void exec_startup_script(const char *filename)
{
	struct sbuf s;
	struct stat sb;

	if (!filename)
		return;
	sbuf_init(&s);
	sbuf_addstr(&s, filename);
	sbuf_expand_tilde(&s);
	if (stat(s.buf, &sb))
		goto clean;
	spawn_command_line_sync(s.buf);
clean:
	xfree(s.buf);
}

int main(int argc, char *argv[])
{
	int i;
	char *arg_config_file = NULL;
	int arg_vsimple = 0;
	FILE *fp = NULL;

	args_exec_commands(argc, argv);
	init_locale();
	restart_init(argc, argv);
	init_sigactions();
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
		else if (!strncmp(argv[i], "--version", 9))
			version();
		else if (!strncmp(argv[i], "--help", 6) ||
			 !strncmp(argv[i], "-h", 2))
			usage();
	}

	if (!arg_vsimple)
		exec_startup_script("~/.config/jgmenu/startup");

	if (!arg_vsimple)
		config_read_jgmenurc(arg_config_file);
	if (!config.verbosity)
		mute_info();
	args_parse(argc, argv);
	if (args_simple() || arg_vsimple)
		set_simple_mode();
	if (arg_vsimple)
		config.icon_size = 0;

	/* check lockfile after --help and --version */
	if (config.stay_alive)
		lockfile_init();

	if_unity_run_hack();

	hooks_init();
	ui_init();
	geo_init();
	filter_init();

	if (config.tint2_look)
		read_tint2rc();

	/*
	 * If _NET_WORKAREA is supported by the Window Manager, set
	 * config.menu_margin_{x|y} and config.{v|h}align if possible
	 */
	if (config.respect_workarea) {
		workarea_set_margin();
		workarea_set_panel_pos();
	}

	/* Parse jgmenurc again to overrule tint2rc and workarea values */
	if (!arg_vsimple)
		config_read_jgmenurc(arg_config_file);

	args_parse(argc, argv);
	config_post_process();
	/* config variables will not be changed after this point */

	set_font();
	set_theme();
	init_geo_variables_from_config();

	if (config.position_mode == POSITION_MODE_IPC) {
		ipc_init_socket();
		ipc_align_based_on_env_vars();
	}

	fp = get_csv_source(arg_vsimple);
	read_csv_file(fp, false);
	if (fp && fp != stdin)
		fclose(fp);
	if (config.hide_back_items)
		rm_back_items();

	/*
	 * read_csv_file() ensures that the first item is a ^tag() one.
	 * In addition to this, we need at least one menu item to enable us
	 * to generate a menu.
	 */
	if (list_empty(&menu.master) || list_is_singular(&menu.master))
		die("file did not contain any menu items");

	remove_checkouts_without_matching_tags();
	build_tree();

	if (args_checkout())
		checkout_rootmenu(args_checkout());
	else
		checkout_rootmenu(tag_of_first_item());
	set_submenu_height();
	set_submenu_width();
	keep_menu_height_between_min_and_max();

	grabkeyboard();
	grabpointer();

	if (config.position_mode == POSITION_MODE_PTR)
		launch_menu_at_pointer();

	if (config.menu_height_mode == CONFIG_DYNAMIC)
		/*
		 * Make canvases the full size of the monitor to allow for
		 * future changes in menu size on ^root().
		 */
		ui_win_init(geo_get_menu_x0(), geo_get_menu_y0(),
			    geo_get_menu_width(), geo_get_menu_height(),
			    geo_get_screen_width(), geo_get_screen_height(),
			    font_get());
	else
		/* Make canvas just big enough */
		ui_win_init(geo_get_menu_x0(), geo_get_menu_y0(),
			    geo_get_menu_width(), geo_get_menu_height(),
			    geo_get_menu_width(), geo_get_menu_height(),
			    font_get());

	init_empty_item();
	update_filtered_list();
	init_menuitem_coordinates();
	if (config.hide_on_startup)
		info("menu started in 'hidden' mode; show by `jgmenu_run`");
	else
		XMapRaised(ui->dpy, ui->w[ui->cur].win);
	draw_menu();

	atexit(cleanup);
	menu.current_node->wid = ui->w[ui->cur].win;
	run();

	return 0;
}
