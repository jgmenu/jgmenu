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
#include <errno.h>

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

#define DEBUG_ICONS_LOADED_NOTIFICATION 0

#define MAX_FIELDS 3		/* nr fields to parse for each stdin line */

#define MOUSE_FUDGE 3		/* Pointer vertical offset		  */
				/* Not sure why I need this		  */

static pthread_t thread;	/* worker thread for loading icons	  */
static int pipe_fds[2];		/* pipe to communicate between threads	  */

struct Item {
	char *t[MAX_FIELDS];	/* pointers to name, cmd, icon		  */
	char *tag;		/* used to tag the start of a submenu	  */
	struct Area area;
	cairo_surface_t *icon;
	struct Item *parent;
	struct Item *next, *prev;
	struct list_head master;
	struct list_head filter;
};

static struct Item empty_item;

/*
 * All menu items in the input file are stored in a dynamic array (=vector).
 * These items are also joined in a doubly linked list.
 *
 * When a submenu is checked out, *subhead and *subtail are set.
 *
 * *first and *last point to the first/last visible menu items (i.e. what can
 * pysically be seen on the screen.)
 * The "number of visible menu items" is not a variable in the Menu struct,
 * but can be got by calling geo_get_nr_visible_items().
 */
struct Menu {
	struct Item *head;	/* first item in linked list		  */
	struct Item *tail;	/* last item in linked list		  */
	int nr_items_in_list;

	struct Item *subhead;	/* first item in checked out submenu	  */
	struct Item *subtail;	/* last item in checked out submenu	  */
	int nr_items_in_submenu;

	struct Item *first;	/* first visible item			  */
	struct Item *last;	/* last visible item			  */
	struct Item *sel;	/* currently selected item		  */

	struct Item *filter_head;
	struct Item *filter_tail;

	char *title;

	struct list_head master;
	struct list_head filter;
};

struct Menu menu;

void init_empty_item(void)
{
	empty_item.t[0] = strdup("<empty>");
	empty_item.t[1] = strdup(":");
	empty_item.t[2] = NULL;
	empty_item.tag = NULL;
	empty_item.icon = NULL;
}

void usage(void)
{
	printf("Usage: jgmenu [OPTIONS]\n"
	       "    --version             show version\n"
	       "    --no-spawn            redirect command to stdout rather than executing it\n"
	       "    --checkout=<tag>      checkout submenu <tag> on startup\n"
	       "    --config-file=<file>  read config file\n");
	exit(0);
}

int get_nr_items(void)
{
	int i = 0;
	struct Item *item;

	list_for_each_entry(item, &menu.filter, filter)
		++i;

	return i;
}

void step_back(struct Item **ptr, int nr)
{
	int i;

	for (i = 0; i < nr; i++)
		*ptr = container_of((*ptr)->filter.prev, struct Item, filter);
}

void step_fwd(struct Item **ptr, int nr)
{
	int i;

	for (i = 0; i < nr; i++)
		*ptr = container_of((*ptr)->filter.next, struct Item, filter);
}

void update_filtered_list(void)
{
	struct Item *item;

	INIT_LIST_HEAD(&menu.filter);

	/* APPLY FILTER ON CURRENT SUBMENU */
	list_for_each_entry(item, &menu.master, master) {
		if (item == menu.subhead) {
			break;
		}
	}

	list_for_each_entry_from(item, &menu.master, master) {
		if (filter_ismatch(item->t[0]))
			list_add_tail(&item->filter, &menu.filter);
		if (item == menu.subtail)
			break;
	}

	/* APPLY FILTER ON WHOLE LIST (DISREGARDING MENU TREE) */
/*		list_for_each_entry(item, &menu.master, master)
 *		if (filter_ismatch(item->t[0]))
 *			list_add_tail(&item->filter, &menu.filter);
 */

	menu.filter_head = list_first_entry_or_null(&menu.filter, struct Item, filter);
	if (!menu.filter_head) {
		list_add_tail(&empty_item.filter, &menu.filter);
		menu.filter_tail = &empty_item;
		menu.filter_head = &empty_item;
		menu.first = &empty_item;
		menu.last = &empty_item;
		menu.sel = &empty_item;
		return;
	}

	menu.filter_tail = list_last_entry(&menu.filter, struct Item, filter);
	menu.first = menu.filter_head;

	if (geo_get_nr_visible_items() < get_nr_items()) {
		menu.last = menu.first;
		step_fwd(&menu.last, geo_get_nr_visible_items() - 1);
	} else {
		menu.last = menu.filter_tail;
	}

	/* FIXME: change this to something more sophisticated */
	menu.sel = menu.first;
}

void init_menuitem_coordinates(void)
{
	struct Item *p;
	int i = 0;

	if (list_empty(&menu.filter))
		return;

	if (menu.title && config.show_title)
		++i;

	p = menu.first;
	list_for_each_entry_from(p, &menu.filter, filter) {
		p->area = geo_get_item_coordinates(i);
		++i;
		if (p == menu.last)
			break;
	}

}

void draw_menu(void)
{
	struct Item *p;
	int w, h;
	int text_x_coord;
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
	ui_draw_rectangle(0, 0, w, geo_get_menu_height(), config.menu_radius,
			  config.menu_border, 0, config.color_menu_fg);

	/* Draw menu items */
	p = menu.first;
	list_for_each_entry_from(p, &menu.filter, filter) {
		/* Draw Item background */
		if (p == menu.sel) {
			ui_draw_rectangle(p->area.x, p->area.y, p->area.w,
					  p->area.h, config.item_radius, 0.0, 1,
					  config.color_sel_bg);
			ui_draw_rectangle(p->area.x, p->area.y, p->area.w,
					  p->area.h, config.item_radius,
					  config.item_border, 0,
					  config.color_sel_fg);
		} else {
			ui_draw_rectangle(p->area.x, p->area.y, p->area.w,
					  p->area.h, config.item_radius, 0.0, 1,
					  config.color_norm_bg);
			ui_draw_rectangle(p->area.x, p->area.y, p->area.w,
					  p->area.h, config.item_radius,
					  config.item_border, 0,
					  config.color_norm_fg);
		}


		/* Draw : to indicate "filtered in" items */
/*		if (filter_ismatch(p->t[0]))
 *			ui_insert_text(":", p->area.x + p->area.w -
 *				       config.item_padding_x - 6, p->area.y,
 *				       p->area.h, config.color_norm_fg);
 */
		/* Draw submenu arrow */
		if (config.arrow_show && ((!strncmp(p->t[1], "^checkout(", 10) &&
		    strncmp(p->t[0], "..", 2)) || !strncmp(p->t[1], "^sub(", 5)))
			ui_insert_text(config.arrow_string, p->area.x + p->area.w -
				       config.item_padding_x - (p->area.h / 3), p->area.y,
				       p->area.h, config.color_norm_fg);

		/* Draw menu items text */
		text_x_coord = p->area.x + config.item_padding_x;
		if (config.icon_size)
			text_x_coord += config.icon_size + config.item_padding_x;

		if (strncmp(p->t[1], "^checkout(", 10) &&
		    strncmp(p->t[1], "^sub(", 5) &&
		    strncmp(p->t[0], "..", 2) &&
		    !is_prog(p->t[1]))
			/* Set color for programs not available in $PATH */
			ui_insert_text(p->t[0], text_x_coord, p->area.y,
				       p->area.h, config.color_noprog_fg);
		else if (p == menu.sel)
			ui_insert_text(p->t[0], text_x_coord, p->area.y,
				       p->area.h, config.color_sel_fg);
		else
			ui_insert_text(p->t[0], text_x_coord, p->area.y,
				       p->area.h, config.color_norm_fg);

		/* Draw Icons */
		if (config.icon_size && p->icon) {
			icon_y_coord = p->area.y + 1 + (config.item_height - config.icon_size) / 2;
			ui_insert_image(p->icon, p->area.x, icon_y_coord, config.icon_size);
		}

		if (p == menu.last)
			break;
	}

	ui_map_window(geo_get_menu_width(), geo_get_menu_height());
}

/*
 * Sets *menu.first and *menu.last pointing to the beginnning and end of
 * the submenu
 */
void checkout_submenu(char *tag)
{
	struct Item *item;
	char *tagtok = "^tag(";

	menu.first = NULL;
	menu.title = NULL;

	if (!tag) {
		menu.first = menu.head;
	} else {
		list_for_each_entry(item, &menu.master, master) {
			if (item->tag && !strncmp(item->tag, tag, strlen(tag))) {
				menu.title = item->t[0];
				if (item->next)
					menu.first = item + 1;
				else
					menu.first = NULL;
				break;
			}
		}

		/* If ^checkout() called without associated ^tag() */
		if (!menu.title)
			die("cannot find ^tag(%s)", tag);
		if (!menu.first)
			die("menu.first not set. Menu has no content");
	}

	menu.last = NULL;

	if (!menu.first->next) {
		menu.last = menu.first;
	} else {
		item = menu.first;

		while (!menu.last && item && item->t[0]) {
			if (!item->next || !strncmp(item->next->t[1], tagtok, strlen(tagtok)))
				menu.last = item;
			else
				item++;
		}

		if (!menu.last)
			die("menu.last pointer not set");
	}

	menu.nr_items_in_submenu = 1;
	for (item = menu.first; item && item->t[0] && item != menu.last; item++)
		menu.nr_items_in_submenu++;

	/*
	 * menu.subhead remembers where the submenu starts.
	 * menu.first will change with scrolling
	 */
	/* FIXME - subhead and subtail should be used above for first/last
	 * only initiated at the end of this function
	 */
	menu.subhead = menu.first;
	menu.subtail = menu.last;

	menu.sel = menu.first;

	if (config.show_title)
		geo_set_show_title(menu.title);
	else
		geo_set_show_title(0);

	if (config.max_items < menu.nr_items_in_submenu)
		geo_set_nr_visible_items(config.max_items);
	else if (config.min_items > menu.nr_items_in_submenu)
		geo_set_nr_visible_items(config.min_items);
	else
		geo_set_nr_visible_items(menu.nr_items_in_submenu);

	if (config.min_items > menu.nr_items_in_submenu)
		menu.last = menu.first + menu.nr_items_in_submenu - 1;
	else
		menu.last = menu.first + geo_get_nr_visible_items() - 1;
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

void action_cmd(char *cmd)
{
	char *p = NULL;

	if (!cmd)
		return;

	if (!strncmp(cmd, "^checkout(", 10)) {
		p = parse_caret_action(cmd, "^checkout(");
		if (p) {
			checkout_submenu(p);
			/* menu height has changed - need to redraw window */
			XMoveResizeWindow(ui->dpy, ui->win, geo_get_menu_x0(), geo_get_menu_y0(),
					  geo_get_menu_width(), geo_get_menu_height());
			update_filtered_list();
			init_menuitem_coordinates();
			draw_menu();
		}
	} else if (!strncmp(cmd, "^sub(", 5)) {
		p = parse_caret_action(cmd, "^sub(");
		if (p) {
			spawn(p);
			exit(0);
		}
	} else {
		spawn(cmd);
		exit(0);
	}
}

int scroll_step_down(void)
{
	int i = 0;
	struct Item *item;

	list_for_each_entry_reverse(item, &menu.filter, filter) {
		if (item == menu.last)
			break;
		++i;
	}

	if (i <= geo_get_nr_visible_items())
		return i;
	else
		return geo_get_nr_visible_items();
}

int scroll_step_up(void)
{
	int i = 0;
	struct Item *item;

	list_for_each_entry(item, &menu.filter, filter) {
		if (item == menu.first)
			break;
		++i;
	}

	if (i <= geo_get_nr_visible_items())
		return i;
	else
		return geo_get_nr_visible_items();
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
	int nr_steps;

	len = XmbLookupString(ui->xic, ev, buf, sizeof(buf), &ksym, &status);
	if (status == XBufferOverflow)
		return;
	switch (ksym) {
	case XK_End:
		if (get_nr_items() > geo_get_nr_visible_items()) {
			menu.last = menu.filter_tail;
			menu.first = menu.filter_tail;
			step_back(&menu.first, geo_get_nr_visible_items() - 1);
			init_menuitem_coordinates();
		}
		menu.sel = menu.last;
		break;
	case XK_Escape:
		exit(0);
	case XK_Home:
		if (get_nr_items() > geo_get_nr_visible_items()) {
			menu.first = menu.filter_head;
			menu.last = menu.filter_head;
			step_fwd(&menu.last, geo_get_nr_visible_items() - 1);
			init_menuitem_coordinates();
		}
		menu.sel = menu.first;
		break;
	case XK_Up:
		if (menu.sel == menu.filter_head)
			break;
		if (menu.sel != menu.first) {
			step_back(&menu.sel, 1);
		} else {
			step_back(&menu.first, 1);
			step_back(&menu.last, 1);
			menu.sel = menu.first;
			init_menuitem_coordinates();
		}
		break;
	case XK_Next:	/* PageDown */
		if (get_nr_items() > geo_get_nr_visible_items()) {
			nr_steps = scroll_step_down();
			step_fwd(&menu.first, nr_steps);
			step_fwd(&menu.last, nr_steps);
			init_menuitem_coordinates();
		}
		menu.sel = menu.last;
		break;
	case XK_Prior:	/* PageUp */
		if (get_nr_items() > geo_get_nr_visible_items()) {
			nr_steps = scroll_step_up();
			step_back(&menu.first, nr_steps);
			step_back(&menu.last, nr_steps);
			init_menuitem_coordinates();
		}
		menu.sel = menu.first;
		break;
	case XK_Return:
	case XK_KP_Enter:
		if (config.spawn) {
			action_cmd(menu.sel->t[1]);
		} else {
			printf("%s", menu.sel->t[1]);
			exit(0);
		}
		break;
	case XK_Down:
		if (menu.sel == menu.filter_tail)
			break;
		if (menu.sel != menu.last) {
			step_fwd(&menu.sel, 1);
		} else {
			step_fwd(&menu.first, 1);
			step_fwd(&menu.last, 1);
			menu.sel = menu.last;
			init_menuitem_coordinates();
		}
		break;
	case XK_F2:
		checkout_submenu("root");
		XMoveResizeWindow(ui->dpy, ui->win, geo_get_menu_x0(), geo_get_menu_y0(),
				  geo_get_menu_width(), geo_get_menu_height());
		update_filtered_list();
		init_menuitem_coordinates();
		draw_menu();
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
	case XK_BackSpace:
		filter_backspace();
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

struct Point mousexy(void)
{
	Window dw;
	int di;
	unsigned int du;
	struct Point coords;

	XQueryPointer(ui->dpy, ui->win, &dw, &dw, &di, &di, &coords.x,
		      &coords.y, &du);

	return coords;
}

void mouse_event(XEvent *e)
{
	struct Item *item;
	XButtonPressedEvent *ev = &e->xbutton;
	struct Point mouse_coords;

	mouse_coords = mousexy();
	mouse_coords.y -= MOUSE_FUDGE;

	/* Die if mouse clicked outside window */
	if ((ev->x < geo_get_menu_x0() ||
	     ev->x > geo_get_menu_x0() + geo_get_menu_width() ||
	     ev->y < geo_get_menu_y0() ||
	     ev->y > geo_get_menu_y0() + geo_get_menu_height()) &&
	     (ev->button != Button4 && ev->button != Button5)) {
		exit(0);
	}

	/* right-click */
	if (ev->button == Button3)
		die("Right clicked.");

	/* scroll up */
	if (ev->button == Button4 && menu.first != menu.filter_head) {
		step_back(&menu.first, 1);
		step_back(&menu.last, 1);
		step_back(&menu.sel, 1);
		init_menuitem_coordinates();
		draw_menu();
		return;
	}

	/* scroll down */
	if (ev->button == Button5 && menu.last != menu.filter_tail) {
		step_fwd(&menu.first, 1);
		step_fwd(&menu.last, 1);
		step_fwd(&menu.sel, 1);
		init_menuitem_coordinates();
		draw_menu();
		return;
	}

	/* left-click */
	if (ev->button == Button1) {
		for (item = menu.first; item && item->t[0] && item->prev != menu.last ; item++) {
			if (ui_is_point_in_area(mouse_coords, item->area)) {
				if (config.spawn) {
					action_cmd(item->t[1]);
					break;
				}
				puts(item->t[1]);
				exit(0);
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

void parse_csv(struct Item *p)
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

void dlist_append(struct Item *item, struct Item **list, struct Item **last)
{
	if (*last)
		(*last)->next = item;
	else
		*list = item;

	item->prev = *last;
	item->next = NULL;
	*last = item;
}

/*
 * This function is loaded in the background under a new pthread
 * X11 is not thread-safe, so load_icons() must not call any X functions.
 */
void *load_icons(void *arg)
{
	icon_load();

	if (write(pipe_fds[1], "x", 1) == -1)
		die("error writing to icon_pipe");

	return NULL;
}

void create_master_list(void)
{
	struct Item *item;

	for (item = menu.head; item && item->t[0]; item++)
		list_add_tail(&item->master, &menu.master);
}

struct Item *get_item_from_tag(const char *tag)
{
	struct Item *item;

	if (!tag)
		die("tag=(null) in get_item_from_tag()");

	list_for_each_entry(item, &menu.master, master)
		if (item->tag && !strcmp(tag, item->tag))
			return item;

	fprintf(stderr, "warning: could not find tag '%s'\n", tag);
	return NULL;
}

void walk_children(struct Item *this, struct Item *parent)
{
	static struct Item *root;
	struct sbuf s;
	struct Item *child, *p;

	sbuf_init(&s);

	if (this) {
		this->parent = parent;
		/* move to next item, as this points to a ^tag() item */
		p = container_of((this)->master.next, struct Item, master);
	} else {
		p = list_first_entry_or_null(&menu.master, struct Item,
						master);
	}

	if (!root && this)
		root = this;
	else if (!root)
		root = p;

	if (p == list_last_entry(&menu.master, struct Item, master))
		return;

	list_for_each_entry_from(p, &menu.master, master) {
		if (!strncmp("^checkout(", p->t[1], 10)) {
			sbuf_cpy(&s, parse_caret_action(p->t[1], "^checkout("));
			child = get_item_from_tag(s.buf);
			if (child->parent || child == root)
				continue;
			walk_children(child, this);
		} else if (!strncmp("^tag(", p->t[1], 5)) {
			break;
		}
	}
}


void build_tree(void)
{
	struct Item *item;

	if (list_empty(&menu.master))
		die("cannot build tree on empty menu.master list");
	item = list_first_entry_or_null(&menu.master, struct Item, master);
	if (item->tag && list_is_singular(&menu.master))
		die("cannot build a menu on a single tag item");
	if (item->tag)
		walk_children(get_item_from_tag(item->tag), NULL);
	else
		walk_children(NULL, NULL);
}

void read_stdin(void)
{
	char buf[BUFSIZ], *p;
	size_t i, size = 0;
	struct Item *item;

	menu.head = NULL;

	for (i = 0; fgets(buf, sizeof(buf), stdin); i++) {
		if (size <= (i + 1) * sizeof(struct Item)) {
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
	 * rather than menu.nr_items_in_list or menu.tail.
	 */
	menu.head[i].t[0] = NULL;

	/* menu.nr_items_in_list holds the number of items in the dynamic array */
	/* Not currently using menu.nr_items_in_list in code except to define the cairo buf */
	menu.nr_items_in_list = i;

	/* Create doubly-linked list */
	menu.tail = NULL;
	for (item = menu.head; item && item->t[0]; item++)
		dlist_append(item, &menu.head, &menu.tail);

	create_master_list();

	/* Populate tag field */
	for (item = menu.head; item && item->t[0]; item++)
		item->tag = parse_caret_action(item->t[1], "^tag(");

	for (item = menu.head; item && item->t[0]; item++) {
		item->icon = NULL;
		item->parent = NULL;
	}

	build_tree();
	list_for_each_entry(item, &menu.master, master)
		if (item->parent)
			fprintf(stderr, "name:%s; parent:%s\n", item->t[0], item->parent->tag);
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
	struct Item *item;
	struct Point mouse_coords;
	static int oldy;
	static int oldx;

	mouse_coords = mousexy();
	mouse_coords.y -= MOUSE_FUDGE;

	if ((mouse_coords.x == oldx) && (mouse_coords.y == oldy))
		return;

	item = menu.first;
	list_for_each_entry_from(item, &menu.filter, filter) {
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

/*
 * The select() call in the main loop is a better alternative than usleep().
 */
void run(void)
{
	XEvent ev;
	struct Item *item;

	char ch;
	int ready, nfds, x11_fd;
	fd_set readfds;
	static int all_icons_have_been_requested;

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

		/* The icon thread has finished */
		if (FD_ISSET(pipe_fds[0], &readfds) && ready) {
			for (;;) {
				if (read(pipe_fds[0], &ch, 1) == -1) {
					if (errno == EAGAIN)
						break;
					die("error reading pipe");
				}

				/* 'x' means that icons have finished loading */
				if (ch != 'x')
					continue;

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
	char *config_file = NULL;
	char *checkout_arg = NULL;

	config_set_defaults();
	menu.title = NULL;
	INIT_LIST_HEAD(&menu.master);
	INIT_LIST_HEAD(&menu.filter);

	for (i = 1; i < argc; i++)
		if (!strncmp(argv[i], "--config-file=", 14))
			config_file = strdup(argv[i] + 14);
	if (!config_file)
		config_file = strdup("~/.config/jgmenu/jgmenurc");

	if (config_file && config_file[0] != '\0') {
		if (config_file[0] == '~')
			config_file = expand_tilde(config_file);
		config_parse_file(config_file);
	}

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
		} else if (!strncmp(argv[i], "--fixed-height=", 15)) {
			xatoi(&config.min_items, argv[i] + 15, XATOI_NONNEG,
			      "config.min_items");
			config.max_items = config.min_items;
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

	ui_create_window(geo_get_menu_x0(), geo_get_menu_y0(),
			 geo_get_menu_width(), geo_get_menu_height());
	ui_init_canvas(geo_get_menu_width(), geo_get_screen_height());
	ui_init_cairo(geo_get_menu_width(), geo_get_screen_height(), config.font);

	init_empty_item();
	filter_init();
	update_filtered_list();
	init_menuitem_coordinates();
	draw_menu();

	run();

	ui_cleanup();

	return 0;
}
