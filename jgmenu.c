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

#include "x11-ui.h"
#include "config.h"
#include "util.h"
#include "geometry.h"
#include "prog-finder.h"

#define MAX_FIELDS 3		/* nr fields to parse for each stdin line */

struct Item {
	char *t[MAX_FIELDS];	/* pointers name, cmd			  */
	char *tag;		/* used to tag the start of a submenu	  */
	struct Item *next, *prev;
};

struct Menu {
	struct Item *head;	/* pointer to the first menu item	   */
	struct Item *tail;	/* end of dynamic array			   */
	int end;		/* number of items in dynamic array	   */
	struct Item *sel;	/* pointer to the currently selected item  */
	struct Item *first;	/* pointer to the first item in submenu	   */
	struct Item *last;	/* pointer to the first item in submenu	   */
	int nr_items;		/* number of items in menu/submenu	   */
	char *title;
};

struct Menu menu;

void usage(void)
{
	printf("Usage: jgmenu [OPTIONS]\n"
	       "    --version             show version\n"
	       "    -r                    redirect command to stdout rather than executing\n"
	       "    --debug               debug mode\n"
	       "    --config-file=<file>  read config file\n");
	exit(0);
}


void draw_menu(void)
{
	struct Item *p;
	int x, y, w, h, offset;

	x = 0;
	y = 0;
	h = geo_get_item_height();
	w = geo_get_menu_width();

	offset = (h - geo_get_font_height()) / 2;

	/* Set background */
	ui_clear_canvas(0, 0, w, geo_get_menu_height());
	ui_draw_rectangle(0, 0, w, geo_get_menu_height(), 1, config.color_norm_bg);
	ui_draw_rectangle(0, 0, w, geo_get_menu_height(), 0, config.color_sel_bg);

	/* Draw title */
	if (menu.title) {
		ui_draw_rectangle(x, y, w, h, 1, config.color_sel_bg);
		ui_insert_text(menu.title, x, y + offset, h, config.color_sel_fg);
		y += h;
	}

	/* Draw menu items */
	for (p = menu.first; p && p->t[0] && p->prev != menu.last; p++) {
		if (p == menu.sel)
			ui_draw_rectangle(x, y, w, h, 1, config.color_sel_bg);

		/* Draw submenu arrow */
		if (!strncmp(p->t[1], "^checkout(", 10) &&
		    strncmp(p->t[0], "..", 2)) {
			ui_draw_line(x + w - 8, y + h / 2 - 2, x + w - 2, y + h / 2, config.color_norm_fg);
			ui_draw_line(x + w - 8, y + h / 2 + 2, x + w - 2, y + h / 2, config.color_norm_fg);
		}

		if (strncmp(p->t[1], "^checkout(", 10) &&
		    strncmp(p->t[0], "..", 2) &&
		    !is_prog(p->t[1]))
			ui_insert_text(p->t[0], x, y + offset, h, config.color_broke_fg);
		else if (p == menu.sel)
			ui_insert_text(p->t[0], x, y + offset, h, config.color_sel_fg);
		else
			ui_insert_text(p->t[0], x, y + offset, h, config.color_norm_fg);

		y += h;
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
		for (item = menu.head; item && item->t[0]; item++) {
			if (item->tag && !strncmp(item->tag, tag, strlen(tag))) {
				menu.title = item->t[0];
				if (item->next)
					menu.first = item + 1;
				else
					menu.first = NULL;
				break;
			}
		}

		if (!menu.title)
			die("menu.title pointer not set.  Tag not found.");
		if (!menu.first)
			die("menu.first not set. Menu has no content");
	}

	menu.last = NULL;

	if (!menu.first->next) {
		menu.last = menu.first;
	} else {
		item = menu.first->next;

		while (!menu.last && item && item->t[0]) {
			if (!item->next || !strncmp(item->next->t[1], tagtok, strlen(tagtok)))
				menu.last = item;
			else
				item++;
		}

		if (!menu.last)
			die("menu.last pointer not set");
	}

	menu.nr_items = 1;
	for (item = menu.first; item && item->t[0] && item != menu.last; item++)
		menu.nr_items++;
}

char *parse_caret_action(char *s, char *token)
{
	/* Returns bar from ^foo(bar)
	 * s="^foo(bar)"
	 * token="^foo("
	 */

	char *p, *q;

	p = NULL;
	q = NULL;

	if (s)
		if (strncmp(s, token, strlen(token)) == 0) {
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
	if (cmd[0] == '^') {
		p = parse_caret_action(cmd, "^checkout(");
		if (p) {
			checkout_submenu(p);
			menu.sel = menu.first;
			geo_set_show_title(menu.title);
			geo_set_nr_items(menu.nr_items);

			/* menu height has changed - need to redraw window */
			XMoveResizeWindow(ui->dpy, ui->win, geo_get_menu_x0(), geo_get_menu_y0(),
					  geo_get_menu_width(), geo_get_menu_height());

			draw_menu();
		} else {
			die("item began with ^ but was not checkout()");
		}
	} else {
		spawn(cmd);
		exit(0);
	}
}

void key_event(XKeyEvent *ev)
{
	char buf[32];
	KeySym ksym = NoSymbol;
	Status status;

	XmbLookupString(ui->xic, ev, buf, sizeof(buf), &ksym, &status);
	if (status == XBufferOverflow)
		return;
	switch (ksym) {
	default:
		break;
	case XK_End:
		menu.sel = menu.last;
		break;
	case XK_Escape:
		exit(0);
	case XK_Home:
		menu.sel = menu.first;
		break;
	case XK_Up:
		if (menu.sel && menu.sel->prev && (menu.sel != menu.first))
			menu.sel = menu.sel->prev;
		break;
	case XK_Next:
		menu.sel = menu.last;
		break;
	case XK_Prior:
		menu.sel = menu.first;
		break;
	case XK_Return:
	case XK_KP_Enter:
		if (config.spawn)
			action_cmd(menu.sel->t[1]);
		else
			puts(menu.sel->t[1]);
		break;
	case XK_Down:
		if (menu.sel && menu.sel->next && (menu.sel != menu.last))
			menu.sel = menu.sel->next;
		break;
	}
}

void mouse_event(XEvent *e)
{
	struct Item *item;
	XButtonPressedEvent *ev = &e->xbutton;
	int y;
	int mousex, mousey;

	mousex = ev->x - geo_get_menu_x0();
	mousey = ev->y - geo_get_menu_y0();

	/* Die if mouse clicked outside window */
	if ((ev->x < geo_get_menu_x0() ||
	     ev->x > geo_get_menu_x0() + geo_get_menu_width() ||
	     ev->y < geo_get_menu_y0() ||
	     ev->y > geo_get_menu_y0() + geo_get_menu_height()) &&
	     (ev->button != Button4 && ev->button != Button5)) {
		ui_cleanup();
		die("Clicked outside menu.");
		return;
	}

	/* right-click */
	if (ev->button == Button3)
		die("Right clicked.");

	/* scroll up */
	if (ev->button == Button4 && menu.sel->prev && menu.sel != menu.first) {
		menu.sel = menu.sel->prev;
		draw_menu();
		return;
	}

	/* scroll down */
	if (ev->button == Button5 && menu.sel->next && menu.sel != menu.last) {
		menu.sel = menu.sel->next;
		draw_menu();
		return;
	}

	/* left-click */
	if (ev->button == Button1) {
		y = 0;
		if (menu.title)
			y += geo_get_item_height();
		for (item = menu.first; item && item->t[0] && item->prev != menu.last ; item++) {
			if (mousey >= y && mousey <= (y + geo_get_item_height())) {
				if (config.spawn) {
					action_cmd(item->t[1]);
					break;
				} else {
					puts(item->t[1]);
				}
			}
			y += geo_get_item_height();
		}
	}
}

void tabulate(char *s)
{
	int i = 0;
	int n = 20;		/* column spacing */

	if (s)
		n -= strlen(s);
	else
		n -= 6;		/* because strlen("(null)") = 6 */

	if (n < 0)
		n = 0;

	for (i = 0; i < n; i++)
		printf(" ");
}

void debug_dlist(void)
{
	struct Item *item;

	printf("---------------------------------------------------------------\n");
	printf("Name                Cmd                 Spare               Tag\n");
	printf("---------------------------------------------------------------\n");

	for (item = menu.head; item && item->t[0]; item++) {
		printf("%s", item->t[0]);
		tabulate(item->t[0]);
		printf("%s", item->t[1]);
		tabulate(item->t[1]);
		printf("%s", item->t[2]);
		tabulate(item->t[2]);
		printf("%s", item->tag);
		tabulate(item->tag);
		printf("\n");
	}
	printf("menu.head->t[0]: %s\n", menu.head->t[0]);
	printf("menu.tail->t[0]: %s\n", menu.tail->t[0]);
	printf("menu.first->t[0]: %s\n", menu.first->t[0]);
	printf("menu.sel->t[0]: %s\n", menu.sel->t[0]);
	printf("menu.last->t[0]: %s\n", menu.last->t[0]);
	printf("menu.nr_items: %d\n", menu.nr_items);
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
			p->t[j+1] = next_field(p->t[j]);

	while ((q = strrchr(p->t[0], ',')))
		*q = '\0';
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


void read_stdin(void)
{
	char buf[BUFSIZ], *p;
	size_t i, size = 0;
	struct Item *item;

	menu.head = NULL;

	for (i = 0; fgets(buf, sizeof(buf), stdin); i++) {
		if (size <= (i+1) * sizeof(*menu.head)) {
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

	if (!menu.head)
		die("No stdin.");

	/*
	 * Using "menu.head[i].t[0] = NULL" as a dynamic array end-marker
	 * rather than menu.end or menu.tail.
	 */
	menu.head[i].t[0] = NULL;

	/* menu.end holds the number of items in the dynamic array */
	/* Not currently using menu.end in code except to define the cairo buf */
	menu.end = i;

	/* Create doubly-linked list */
	menu.tail = NULL;
	for (item = menu.head; item && item->t[0]; item++)
		dlist_append(item, &menu.head, &menu.tail);

	/* Populate tag field */
	for (item = menu.head; item && item->t[0]; item++)
		item->tag = parse_caret_action(item->t[1], "^tag(");

}

void run(void)
{
	XEvent ev;
	struct Item *item;
	int y, oldy = 0;

	while (!XNextEvent(ui->dpy, &ev)) {
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

		/* Move highlighting with mouse */
		/* Current mouse position is (ev.xbutton.x, ev.xbutton.y) */
		y = geo_get_menu_y0();

		if ((oldy != 0) && (ev.xbutton.y != oldy) && (ev.xbutton.x < geo_get_menu_width())) {
			if (menu.title)
				y += geo_get_item_height();
			for (item = menu.first; item && item->t[0] && item->prev != menu.last; item++) {
				if (ev.xbutton.y > y && ev.xbutton.y <= (y + geo_get_item_height())) {
					if (menu.sel != item) {
						menu.sel = item;
						draw_menu();
						break;
					}
				}
				y += geo_get_item_height();
			}
		}
		oldy = ev.xbutton.y;
	}
}

int main(int argc, char *argv[])
{
	int i;
	char *config_file = NULL;

	config_set_defaults();
	menu.title = NULL;

	for (i = 1; i < argc; i++)
		if (!strncmp(argv[i], "--config-file=", 14))
			config_file = strdup(argv[i] + 14);
	if (config_file) {
		if (config_file[0] == '~')
			config_file = expand_tilde(config_file);
		config_parse_file(config_file);
	}

	for (i = 1; i < argc; i++)
		if (!strncmp(argv[i], "--version", 9)) {
			printf("%s\n", VERSION);
			exit(0);
		} else if (!strcmp(argv[i], "-r")) {
			config.spawn = 0;
		} else if (!strncmp(argv[i], "--debug", 7)) {
			config.debug_mode = 1;
		}

	read_stdin();

	if (menu.head->tag)
		checkout_submenu(menu.head->tag);
	else
		checkout_submenu(NULL);
	menu.sel = menu.first;

	if (config.debug_mode)
		debug_dlist();

	ui_init();

	geo_init();
	geo_set_menu_margin_x(config.menu_margin_x);
	geo_set_menu_margin_y(config.menu_margin_y);
	geo_set_menu_width(config.menu_width);
	geo_set_font(config.font);
	geo_set_item_height(config.item_height);

	/* calculate menu geometry */
	geo_set_show_title(menu.title);
	geo_set_nr_items(menu.nr_items);

	/*
	 * FIXME Would be tidier to calc height to largest submenu rather than
	 * allocating memory for (menu.end * geo_get_item_height())
	 */

	ui_create_window(geo_get_menu_x0(), geo_get_menu_y0(),
			 geo_get_menu_width(), geo_get_menu_height());
	ui_init_canvas(geo_get_menu_width(), menu.end * geo_get_item_height());

	ui_init_cairo(geo_get_menu_width(), menu.end * geo_get_item_height(), config.font);

	draw_menu();

	run();

	ui_cleanup();

	return 0;
}
