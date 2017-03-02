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
#include "config-xs.h"
#include "filter.h"
#include "list.h"
#include "lockfile.h"

#define DEBUG_ICONS_LOADED_NOTIFICATION 0

#define MAX_FIELDS 3		/* nr fields to parse for each stdin line */

static pthread_t thread;	/* worker thread for loading icons	  */
static int pipe_fds[2];		/* talk between threads + catch signals   */
static int die_when_loaded;	/* Used for performance testing		  */

struct item {
	char *t[MAX_FIELDS];
	char *tag;
	struct area area;
	cairo_surface_t *icon;
	int selectable;
	struct item *next, *prev;	/* DELETE? */
	struct list_head master;
	struct list_head filter;
	struct list_head list;		/* submenu list under each node */
};

static struct item empty_item;

/* A node is marked by a ^tag() and denotes the start of a submenu */
struct node {
	char *tag;
	struct item *item;		/* item that node points to */
	struct node *parent;
	struct list_head items;		/* menu-items having off node */
	struct list_head node;
};

/*
 * All menu items in the input file are stored in a dynamic array (=vector).
 * These items are also joined in a doubly linked list.
 *
 * When a submenu is checked out, *subhead and *subtail are set.
 *
 * *first and *last point to the first/last visible menu items (i.e. what can
 * pysically be seen on the screen.)
 */
struct menu {
	struct item *head;	 /* first item in linked list		  */
	struct item *tail;	 /* last item in linked list		  */

	struct item *subhead;	 /* first item in checked out submenu	  */
	struct item *subtail;	 /* last item in checked out submenu	  */
	struct item *first;	 /* first visible item			  */
	struct item *last;	 /* last visible item			  */
	struct item *sel;	 /* currently selected item		  */

	char *title;

	/* These two lists are the core building blocks of the code	  */
	struct list_head master; /* all items				  */
	struct list_head filter; /* items to be displayed in menu	  */

	struct list_head nodes;	 /* hierarchical structure of tags	  */
	struct node *current_node;
};

struct menu menu;

static const char jgmenu_usage[] =
"Usage: jgmenu [OPTIONS]\n"
"    --version             show version\n"
"    --no-spawn            redirect command to stdout rather than executing it\n"
"    --checkout=<tag>      checkout submenu <tag> on startup\n"
"    --config-file=<file>  read config file\n"
"    --at-pointer          launch menu at mouse pointer\n";

void init_empty_item(void)
{
	empty_item.t[0] = strdup("<empty>");
	empty_item.t[1] = strdup(":");
	empty_item.t[2] = NULL;
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

void update_filtered_list(void)
{
	struct item *item;
	int isoutside;

	INIT_LIST_HEAD(&menu.filter);

	if (config.search_all_items && filter_needle_length()) {
		list_for_each_entry(item, &menu.master, master) {
			if (!strncmp("^checkout(", item->t[1], 10) ||
			    !strncmp("^tag(", item->t[1], 5))
				continue;
			if (filter_ismatch(item->t[0]) ||
			    filter_ismatch(item->t[1]))
				list_add_tail(&item->filter, &menu.filter);
		}
	} else {
		list_for_each_entry(item, &menu.master, master)
			if (item == menu.subhead)
				break;
		list_for_each_entry_from(item, &menu.master, master) {
			if (filter_ismatch(item->t[0]) ||
			    filter_ismatch(item->t[0]))
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

	/* FIXME: change this to something more sophisticated */
	menu.sel = menu.first;
	if (!menu.sel->selectable)
		menu.sel = next_selectable(menu.first, &isoutside);
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
 * s="^foo(bar)"
 * token="^foo("
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

	ui_insert_text(parse_caret_action(p->t[0], "^sep("), text_x_coord,
		       p->area.y, p->area.h, config.color_sep_fg);
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

	text_x_coord = p->area.x + config.item_padding_x;
	if (config.icon_size)
		text_x_coord += config.icon_size + config.item_padding_x;

	if (p == menu.sel)
		ui_insert_text(p->t[0], text_x_coord, p->area.y,
			       p->area.h, config.color_sel_fg);
	else
		ui_insert_text(p->t[0], text_x_coord, p->area.y,
			       p->area.h, config.color_norm_fg);
}

void draw_menu(void)
{
	struct item *p;
	int w, h;
	int icon_y_coord;

	h = geo_get_item_height();
	w = geo_get_menu_width();

	/* Draw background */
	ui_clear_canvas();
	ui_draw_rectangle(0, 0, w, geo_get_menu_height(), config.menu_radius,
			  0.0, 1, config.color_menu_bg);

	/* Draw title */
	if (menu.title && config.show_title) {
		ui_draw_rectangle_rounded_at_top(0, 0, w, h, config.menu_radius,
						 0.0, 1, config.color_title_bg);
		ui_insert_text(menu.title, config.item_padding_x, 0, h,
			       config.color_norm_fg);
	}

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
		else
			draw_item_bg_norm(p);

		/* Draw submenu arrow */
		if (config.arrow_show && ((!strncmp(p->t[1], "^checkout(", 10) &&
					   strncmp(p->t[0], "..", 2)) ||
					   !strncmp(p->t[1], "^sub(", 5)))
			ui_insert_text(config.arrow_string, p->area.x + p->area.w -
				       config.item_padding_x - (p->area.h / 3), p->area.y,
				       p->area.h, config.color_norm_fg);

		/* Draw menu items text */
		if (p->selectable)
			draw_item_text(p);
		else if (!strncmp(p->t[0], "^sep()", 6))
			draw_item_sep(p);
		else if (!strncmp(p->t[0], "^sep(", 5))
			draw_item_sep_with_text(p);

		/* Draw Icons */
		if (config.icon_size && p->icon) {
			icon_y_coord = p->area.y + (config.item_height - config.icon_size) / 2;
			ui_insert_image(p->icon, p->area.x + 1, icon_y_coord, config.icon_size);
		}

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

void set_submenu_width(void)
{
	struct item *p;
	struct sbuf s;
	struct point point;

	sbuf_init(&s);
	p = menu.subhead;
	list_for_each_entry_from(p, &menu.master, master) {
		if (p->t[0]) {
			sbuf_addstr(&s, p->t[0]);
			sbuf_addch(&s, '\n');
		}
		if (p == menu.subtail)
			break;
	}
	point = ui_get_text_size(s.buf, config.font);
	if (config.icon_size)
		point.x += config.icon_size + config.item_padding_x;
	if (point.x > config.menu_width)
		geo_set_menu_width_from_itemarea_width(point.x);
	else
		geo_set_menu_width_from_itemarea_width(config.menu_width);
	free(s.buf);
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
	int maxh = geo_get_max_itemarea_that_fits_on_screen();
	int h;

	h = reqh < maxh ? reqh : maxh;
	geo_set_menu_height_from_itemarea_height(h);
}

void checkout_submenu(char *tag)
{
	struct item *item;
	char *tagtok = "^tag(";

	menu.subhead = NULL;
	menu.title = NULL;

	/* Find head of submenu */
	if (!tag) {
		menu.current_node = list_first_entry_or_null(&menu.nodes, struct node, node);
		menu.subhead = menu.head;
	} else {
		menu.current_node = get_node_from_tag(tag);
		list_for_each_entry(item, &menu.master, master) {
			if (item->tag && !strncmp(item->tag, tag, strlen(tag))) {
				menu.title = item->t[0];
				if (item->next)
					menu.subhead = item + 1;
				else
					menu.subhead = NULL;
				break;
			}
		}

		/* If ^checkout() called without associated ^tag() */
		if (!menu.title)
			die("cannot find ^tag(%s)", tag);
		if (!menu.subhead)
			die("menu.subhead not set. Menu has no content");
	}

	/* Find tail of submenu */
	menu.subtail = NULL;

	if (!menu.subhead->next) {
		menu.subtail = menu.subhead;
	} else {
		item = menu.subhead;

		while (!menu.subtail && item && item->t[0]) {
			if (!item->next || !strncmp(item->next->t[1], tagtok, strlen(tagtok)))
				menu.subtail = item;
			else
				item++;
		}

		if (!menu.subtail)
			die("menu.subtail pointer not set");
	}

	/* DELETE SOON */
	menu.first = menu.subhead;
	menu.last = menu.subtail;

	menu.sel = NULL;
	/* menu.sel gets set in updated_filtered_list() */

	if (config.show_title)
		geo_set_show_title(menu.title);
	else
		geo_set_show_title(0);

	set_submenu_height();
	set_submenu_width();
}

void checkout_root(void)
{
	while (menu.current_node->parent)
		menu.current_node = menu.current_node->parent;
	checkout_submenu(menu.current_node->tag);
}

static void awake_menu(void)
{
	filter_reset();
	checkout_root();
	XMapWindow(ui->dpy, ui->win);
	grabkeyboard();
	grabpointer();
	XMoveResizeWindow(ui->dpy, ui->win, geo_get_menu_x0(), geo_get_menu_y0(),
			  geo_get_menu_width(), geo_get_menu_height());
	update_filtered_list();
	init_menuitem_coordinates();
	draw_menu();
}

static void hide_menu(void)
{
	XUngrabKeyboard(ui->dpy, CurrentTime);
	XUngrabPointer(ui->dpy, CurrentTime);
	XUnmapWindow(ui->dpy, ui->win);
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
	XMoveResizeWindow(ui->dpy, ui->win, geo_get_menu_x0(), geo_get_menu_y0(),
			  geo_get_menu_width(), geo_get_menu_height());
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
		checkout_submenu(p);
		/* menu height has changed - need to redraw window */
		XMoveResizeWindow(ui->dpy, ui->win, geo_get_menu_x0(), geo_get_menu_y0(),
				  geo_get_menu_width(), geo_get_menu_height());
		update_filtered_list();
		init_menuitem_coordinates();
		draw_menu();
	} else if (!strncmp(cmd, "^sub(", 5)) {
		p = parse_caret_action(cmd, "^sub(");
		if (!p)
			return;
		spawn(p);
		hide_or_exit();
	} else if (!strncmp(cmd, "^back(", 6)) {
		checkout_parent();
		update_filtered_list();
		init_menuitem_coordinates();
		draw_menu();
	} else {
		spawn(cmd);
		hide_or_exit();
	}
}

void move_window(void)
{
	XMoveResizeWindow(ui->dpy, ui->win, geo_get_menu_x0(),
			  geo_get_menu_y0(), geo_get_menu_width(),
			  geo_get_menu_height());
	draw_menu();
}

void key_event(XKeyEvent *ev)
{
	char buf[32];
	int len;
	KeySym ksym = NoSymbol;
	Status status;
	int isoutside;

	len = XmbLookupString(ui->xic, ev, buf, sizeof(buf), &ksym, &status);
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
		break;
	case XK_Escape:
		if (filter_needle_length()) {
			filter_reset();
			update_filtered_list();
			init_menuitem_coordinates();
			draw_menu();
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
		break;
	case XK_Return:
	case XK_KP_Enter:
		if (!menu.sel->selectable)
			break;
		if (config.spawn) {
			action_cmd(menu.sel->t[1]);
		} else {
			printf("%s", menu.sel->t[1]);
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
		break;
	case XK_F3:
		config.color_menu_bg[3] -= 0.1;
		if (config.color_menu_bg[3] < 0.0)
			config.color_menu_bg[3] = 0.0;
		init_menuitem_coordinates();
		draw_menu();
		break;
	case XK_F4:
		config.color_menu_bg[3] += 0.1;
		if (config.color_menu_bg[3] > 1.0)
			config.color_menu_bg[3] = 1.0;
		init_menuitem_coordinates();
		draw_menu();
		break;
	case XK_F5:
		geo_set_menu_halign("left");
		move_window();
		break;
	case XK_F6:
		geo_set_menu_valign("bottom");
		move_window();
		break;
	case XK_F7:
		geo_set_menu_valign("top");
		move_window();
		break;
	case XK_F8:
		geo_set_menu_halign("right");
		move_window();
		break;
	case XK_F10:
		/* force exit even if in 'stay_alive' mode */
		exit(0);
		break;
	case XK_BackSpace:
		if (filter_needle_length())
			filter_backspace();
		else
			checkout_parent();
		update_filtered_list();
		init_menuitem_coordinates();
		draw_menu();
		break;
	default:
		filter_addstr(buf, len);
		update_filtered_list();
		init_menuitem_coordinates();
		draw_menu();
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
	XButtonPressedEvent *ev = &e->xbutton;
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
		update_filtered_list();
		init_menuitem_coordinates();
		draw_menu();
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
		for (item = menu.first; item && item->t[0] && item->prev != menu.last ; item++) {
			if (ui_is_point_in_area(mouse_coords, item->area)) {
				if (!item->selectable)
					break;
				if (config.spawn) {
					action_cmd(item->t[1]);
					break;
				}
				puts(item->t[1]);
				hide_or_exit();
			}
		}
	}
}

char *next_field(char *str)
{
	char *tmp;

	tmp = strchr(str, ',');
	if (tmp)
		tmp++;
	return tmp;
}

void parse_csv(struct item *p)
{
	char *q;
	int j;

	p->t[1] = NULL;
	p->t[2] = NULL;

	for (j = 0; j < MAX_FIELDS - 1; j++)
		if (p->t[j])
			p->t[j + 1] = next_field(p->t[j]);

	while ((q = strrchr(p->t[0], ',')))
		*q = '\0';

	/* Prevents seg fault when t[1] == NULL */
	if (!p->t[1])
		p->t[1] = p->t[0];
}

void dlist_append(struct item *item, struct item **list, struct item **last)
{
	if (*last)
		(*last)->next = item;
	else
		*list = item;

	item->prev = *last;
	item->next = NULL;
	*last = item;
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

void create_master_list(void)
{
	struct item *item;

	for (item = menu.head; item && item->t[0]; item++)
		list_add_tail(&item->master, &menu.master);
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
			p = container_of((n->item)->master.next, struct item, master);
		else
			p = list_first_entry_or_null(&menu.master, struct item, master);

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
	n->tag = strdup(name);
	n->item = get_item_from_tag(name);
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
		create_node("root", parent);
		p = list_first_entry_or_null(&menu.master, struct item, master);
	}
	/* p now points to first menu-item under tag "this->tag" */

	if (p == list_last_entry(&menu.master, struct item, master))
		return;

	/* FIXME: Check if node(s.buf) already exists */
	current_node = list_last_entry(&menu.nodes, struct node, node);

	/* walk the items under current node and put into tree structure */
	list_for_each_entry_from(p, &menu.master, master) {
		if (!strncmp("^checkout(", p->t[1], 10)) {
			child = get_item_from_tag(parse_caret_action(p->t[1], "^checkout("));
			if (!child)
				continue;
			if (child->tag && node_exists(child->tag))
				continue;
			walk_tagged_items(child, current_node);
		} else if (!strncmp("^tag(", p->t[1], 5)) {
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

void read_stdin(void)
{
	char buf[BUFSIZ], *p;
	size_t i, size = 0;
	struct item *item;

	menu.head = NULL;

	for (i = 0; fgets(buf, sizeof(buf), stdin); i++) {
		if (size <= (i + 1) * sizeof(struct item)) {
			size += BUFSIZ;
			menu.head = xrealloc(menu.head, size);
		}

		p = strchr(buf, '\n');
		if (p)
			*p = '\0';

		if ((buf[0] == '#') ||
		    (buf[0] == '\n') ||
		    (buf[0] == '\0')) {
			i--;
			continue;
		}

		menu.head[i].t[0] = strdup(buf);
		if (!menu.head[i].t[0])
			die("cannot strdup");

		parse_csv(&menu.head[i]);
	}

	if (!menu.head || i <= 0)
		die("input file contains no menu items");

	/*
	 * Using "menu.head[i].t[0] = NULL" as a dynamic array end-marker
	 */
	menu.head[i].t[0] = NULL;

	/* Create doubly-linked list */
	menu.tail = NULL;
	for (item = menu.head; item && item->t[0]; item++)
		dlist_append(item, &menu.head, &menu.tail);

	create_master_list();

	/* Init items */
	list_for_each_entry(item, &menu.master, master) {
		item->icon = NULL;
		item->tag = NULL;
		item->selectable = 1;
		item->area.h = config.item_height;
		if (!strncmp(item->t[0], "^sep()", 6)) {
			item->selectable = 0;
			item->area.h = config.sep_height;
		} else if (!strncmp(item->t[0], "^sep(", 5)) {
			item->selectable = 0;
		}
	}

	/* Populate tag field */
	list_for_each_entry(item, &menu.master, master) {
		if (strncmp("^tag(", item->t[1], 5))
			continue;
		item->tag = parse_caret_action(item->t[1], "^tag(");
	}

	build_tree();
	hang_items_off_nodes();
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
		icon_init();
		icon_set_size(config.icon_size);
		icon_set_theme(config.icon_theme);

		/* Get icons in top level menu (or the one specified with --check-out= */
		for (item = menu.subhead; item && item->t[0] && item != menu.subtail + 1; item++)
			if (item->t[2])
				icon_set_name(item->t[2]);

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

				if (DEBUG_ICONS_LOADED_NOTIFICATION && all_icons_have_been_requested)
					fprintf(stderr, "All icons loaded\n");

				if (DEBUG_ICONS_LOADED_NOTIFICATION && !all_icons_have_been_requested)
					fprintf(stderr, "Root menu icons loaded\n");

				pthread_join(thread, NULL);

				for (item = menu.head; item && item->t[0]; item++)
					if (!item->icon)
						item->icon = icon_get_surface(item->t[2]);

				draw_menu();

				if (all_icons_have_been_requested)
					continue;

				/* Get remaining icons */
				for (item = menu.head; item && item->t[0]; item++)
					if (item->t[2])
						icon_set_name(item->t[2]);

				pthread_create(&thread, NULL, load_icons, NULL);
				all_icons_have_been_requested = 1;
			}
		}

		if (XPending(ui->dpy)) {
			XNextEvent(ui->dpy, &ev);

			switch (ev.type) {
			case ButtonPress:
				mouse_event(&ev);
				break;
			case KeyPress:
				key_event(&ev.xkey);
				draw_menu();
				break;
			case Expose:
				if (ev.xexpose.count == 0)
					ui_map_window(geo_get_menu_width(), geo_get_menu_height());
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
	geo_set_menu_width(config.menu_width);
	geo_set_item_margin_x(config.item_margin_x);
	geo_set_item_margin_y(config.item_margin_y);
	geo_set_item_height(config.item_height);
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
		geo_set_menu_halign("left");
		geo_set_menu_margin_x(pos.x);
	} else {
		geo_set_menu_halign("right");
		geo_set_menu_margin_x(geo_get_screen_width() - pos.x);
	}

	if (pos.y < geo_get_screen_height() - geo_get_menu_height()) {
		geo_set_menu_valign("top");
		geo_set_menu_margin_y(pos.y);
	} else if (geo_get_menu_height() < pos.y) {
		geo_set_menu_valign("bottom");
		geo_set_menu_margin_y(geo_get_screen_height() - pos.y);
	} else {
		geo_set_menu_valign("bottom");
		geo_set_menu_margin_y(0);
	}
}

void set_theme_and_font(void)
{
	config_xs_get_theme(&config.icon_theme, config.ignore_xsettings,
			    config.ignore_icon_cache);

	if (!config.font && !config.ignore_xsettings)
		config_xs_get_font(&config.font);
	if (!config.font)
		config.font = strdup(JGMENU_DEFAULT_FONT);
}

int main(int argc, char *argv[])
{
	int i;
	struct sbuf config_file;
	struct stat sb;
	char *checkout_arg = NULL;

	lockfile_init();
	config_set_defaults();
	menu.title = NULL;
	menu.current_node = NULL;
	INIT_LIST_HEAD(&menu.master);
	INIT_LIST_HEAD(&menu.filter);
	INIT_LIST_HEAD(&menu.nodes);
	sbuf_init(&config_file);

	for (i = 1; i < argc; i++) {
		if (!strncmp(argv[i], "--config-file=", 14)) {
			if (stat(argv[i] + 14, &sb) != 0)
				fprintf(stderr,
					"warning: file '%s' does not exist\n",
					argv[i] + 14);
			else
				sbuf_cpy(&config_file, argv[i] + 14);
			break;
		}
	}
	if (!config_file.len) {
		sbuf_cpy(&config_file, "~/.config/jgmenu/jgmenurc");
		sbuf_expand_tilde(&config_file);
		if (stat(config_file.buf, &sb) != 0)
			sbuf_cpy(&config_file, "");
	}
	if (config_file.len)
		config_parse_file(config_file.buf);

	for (i = 1; i < argc; i++)
		if (!strncmp(argv[i], "--version", 9)) {
			printf("%s\n", VERSION);
			exit(0);
		} else if (!strncmp(argv[i], "--help", 6) ||
			   !strncmp(argv[i], "-h", 2)) {
			usage();
		} else if (!strncmp(argv[i], "--no-spawn", 10)) {
			config.spawn = 0;
		} else if (!strncmp(argv[i], "--checkout=", 11)) {
			checkout_arg = argv[i] + 11;
		} else if (!strncmp(argv[i], "--icon-size=", 12)) {
			xatoi(&config.icon_size, argv[i] + 12, XATOI_NONNEG,
			      "config.icon_size");
		} else if (!strncmp(argv[i], "--die-when-loaded", 17)) {
			die_when_loaded = 1;
		} else if (!strncmp(argv[i], "--at-pointer", 12)) {
			config.at_pointer = 1;
		} else if (!strncmp(argv[i], "--stay-alive", 12)) {
			config.stay_alive = 1;
		} else if (!strncmp(argv[i], "--hide-on-startup", 17)) {
			config.hide_on_startup = 1;
		}

	set_theme_and_font();

	ui_init();
	geo_init();
	init_geo_variables_from_config();

	read_stdin();

	if (checkout_arg)
		checkout_submenu(checkout_arg);
	else if (menu.head->tag)
		checkout_submenu(menu.head->tag);
	else
		checkout_submenu(NULL);

	grabkeyboard();
	grabpointer();

	if (config.at_pointer)
		launch_menu_at_pointer();

	ui_create_window(geo_get_menu_x0(), geo_get_menu_y0(),
			 geo_get_menu_width(), geo_get_menu_height());
	ui_init_canvas(geo_get_screen_width(), geo_get_screen_height());
	ui_init_cairo(geo_get_screen_width(), geo_get_screen_height(), config.font);

	init_empty_item();
	filter_init();
	update_filtered_list();
	init_menuitem_coordinates();
	if (config.hide_on_startup) {
		fprintf(stderr, "info: menu started in 'hidden' mode; ");
		fprintf(stderr, "show by `jgmenu_run`\n");
		hide_menu();
	} else {
		XMapRaised(ui->dpy, ui->win);
	}
	draw_menu();

	run();

	ui_cleanup();

	return 0;
}
