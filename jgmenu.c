/* jgmenu.c
 *
 * Copyright (C) Johan Malm 2014
 *
 * jgmenu is a stand-alone menu which reads the menu items from stdin
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
//#include <cairo.h>
//#include <cairo-xlib.h>
//#include <pango/pangocairo.h>
#include "x11-ui.h"
#include "config.h"
#include "util.h"


struct Area {
	int x0, y0, x1, y1;
};

static int DEBUG = 0;


void draw_menu(void)
{
	struct Item *p;
	int x,y,w,h;
	int offset;

	x = 0;
	y = 0;
	h = menu.item_h;
	w = menu.menu_w;

	offset = (menu.item_h - menu.font_height) / 2;

	/* Set background */
	ui_draw_rectangle(0, 0, menu.menu_w, menu.menu_h, 1,
			  255, 255, 255, 1.0);

	/* Draw title */
	if (menu.title) {
		ui_draw_rectangle(x, y, w, h, 1, 0.8, 0.6, 0.1, 1.0);
		ui_insert_text(menu.title, x, y + offset, h);
		y += h;
	}

	/* Draw menu items */
	for(p = menu.first; p && p->t[0] && p->prev != menu.last; p++) {
		if(p == menu.sel) {
			ui_draw_rectangle(x, y, w, h, 1, 1.0, 0.7, 0.15, 0.8);
		}

		ui_insert_text(p->t[0], x, y + offset, h);

		if (strncmp(p->t[1], "^checkout(", 10) == 0 &&
		    strncmp(p->t[0], "..", 2) != 0) {
			ui_draw_line(x+w-8, y+menu.item_h/2-2, x+w-2, y+menu.item_h/2, 0,0,0,1.0);
			ui_draw_line(x+w-8, y+menu.item_h/2+2, x+w-2, y+menu.item_h/2, 0,0,0,1.0);
		}

		y += h;
	}

/*
	ui_draw_rectangle(1, 1, menu.menu_w - 2, menu.menu_h - 2, 0,
			  1.0, 0.0, 0.0, 0.7);
*/

	ui_map_window(menu.menu_w, menu.menu_h);
}

/* Sets *menu.first and *menu.last pointing to the beginnning and end of
 * the submenu */
void checkout_submenu(char *tag)
{
	struct Item *item;
	char *tagtok = "^tag(";

	menu.first = NULL;
	menu.title = NULL;

	if (!tag) {
		menu.first = menu.head;
	}
	else {
		for (item = menu.head; item && item->t[0]; item++) {
			if (item->tag && !strncmp(item->tag,tag,strlen(tag))) {
				menu.title = item->t[0];
				if (item->next)
					menu.first = item + 1;
				else
					menu.first = NULL;
				break;
			}
		}

		if (!menu.title)
			die("menu.title POINTER NOT SET.  Tag not found.");
		if (!menu.first)
			die("menu.first not set. Menu has no content");
	}

	menu.last = NULL;

	if (!menu.first->next) {
		menu.last = menu.first;
	}
	else {
		item = menu.first->next;

		while (!menu.last && item && item->t[0]) {
			if (!item->next || !strncmp(item->next->t[1],tagtok,strlen(tagtok)))
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
			if ((q = strchr(p, ')')))
				*q = '\0';
		}

	return p;
}

void action_cmd(char *cmd)
{
	char *p;
	p = NULL;

	if (!cmd)
		return;
	if (cmd[0] == '^') {
		p = parse_caret_action(cmd, "^checkout(");
		if (p) {
			checkout_submenu(p);
			menu.sel = menu.first;

			menu.menu_h = menu.nr_items * menu.item_h;
			if (menu.title)
				menu.menu_h += menu.item_h;
			menu.win_y0 = menu.screen_y0 + menu.screen_h - menu.menu_h - 32;
			XMoveResizeWindow(ui->dpy, ui->win, menu.win_x0, menu.win_y0,
						        menu.menu_w, menu.menu_h);

			draw_menu();
		}
		else {
			die("Item began with ^ but was not checkout()");
		}
	}
	else {
		spawn(cmd);
		exit(0);
	}
}

void key_event(XKeyEvent *ev)
{
	char buf[32];
	KeySym ksym = NoSymbol;
	Status status;

	XmbLookupString(ui->xic, ev, buf, sizeof buf, &ksym, &status);
	if (status == XBufferOverflow)
		return;
	switch(ksym) {
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

	/* Die if mouse clicked outside window */
	if (ev->window != ui->win && ev->button != Button4 && ev->button != Button5) {
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
			y += menu.item_h;
		for (item = menu.first; item && item->t[0] && item->prev != menu.last ; item++) {
			if (ev->y >= y && ev->y <= (y + menu.item_h)) {
				if (config.spawn) {
					action_cmd(item->t[1]);
					break;
				}
				else {
					puts(item->t[1]);
				}
			}
			y += menu.item_h;
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

void debug_dlist()
{
	struct Item *item;

	printf("---------------------------------------------------------------\n");
	printf("Name                Cmd                 Spare               Tag\n");
	printf("---------------------------------------------------------------\n");

	for (item = menu.head; item && item->t[0]; item++) {
		printf("%s",item->t[0]);
		tabulate(item->t[0]);
		printf("%s",item->t[1]);
		tabulate(item->t[1]);
		printf("%s",item->t[2]);
		tabulate(item->t[2]);
		printf("%s",item->tag);
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

	while((q = strrchr(p->t[0], ',')))
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

	/* This for-loop is loosely based on dmenu-4.5 */
	for (i = 0; fgets(buf, sizeof buf, stdin); i++) {
		if (size <= (i+1) * sizeof *menu.head) {
			size += BUFSIZ;
			menu.head = xrealloc(menu.head, size);
		}

		if ((p = strchr(buf, '\n')))
			*p = '\0';

		if ((buf[0] == '#') ||
		   (buf[0] == '\n') ||
		   (buf[0] == '\0')) {
			i--;
			continue;
		}

		if (!(menu.head[i].t[0] = strdup(buf)))
			die("cannot strdup");

		parse_csv(&menu.head[i]);
	}

	if (!menu.head)
		die("No stdin.");

	/* Using "menu.head[i].t[0] = NULL" as a dynamic array end-marker
	 * rather than menu.end or menu.tail. */
	menu.head[i].t[0] = NULL;

	/* menu.end holds the number of items in the dynamic array */
	/* Not currently using menu.end in code except to define the cairo buf */
	menu.end = i;

	/* Create doubly-linked list */
	menu.tail=NULL;
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
		switch(ev.type) {
		case ButtonPress:
			mouse_event(&ev);
			break;
		case KeyPress:
			key_event(&ev.xkey);
			draw_menu();
			break;
		case Expose:
			if(ev.xexpose.count == 0)
				ui_map_window(menu.menu_w, menu.menu_h);
			break;
		case VisibilityNotify:
			if(ev.xvisibility.state != VisibilityUnobscured)
				XRaiseWindow(ui->dpy, ui->win);
			break;
		}

		/* Move highlighting with mouse */
		/* Current mouse position is (ev.xbutton.x, ev.xbutton.y) */
		y = menu.win_y0;

		if ((oldy != 0) && (ev.xbutton.y != oldy) && (ev.xbutton.x < menu.menu_w)) {
			if (menu.title)
				y += menu.item_h;
			for (item = menu.first; item && item->t[0] && item->prev != menu.last; item++) {
				if (ev.xbutton.y > y && ev.xbutton.y <= (y + menu.item_h)) {
					if (menu.sel != item) {
						menu.sel = item;
						draw_menu();
						break;
					}
				}
				y += menu.item_h;
			}
		}

		oldy = ev.xbutton.y;

	}
}

void usage(void)
{
	fputs("usage: jgmenu [OPTIONS]\n"
	      "       -r        redirect command to stdout rather than executing\n"
	      "       -v        version\n"
	      "       --debug   debug\n", stderr);
	exit(0);
}

int main(int argc, char *argv[])
{
	int i;

	config_set_defaults();

	/* parse command line arguments */
	for (i = 1; i < argc; i++)
		if (!strcmp(argv[i], "-v")) {
			printf("jgmenu-%s\n", VERSION);
			exit(0);
		}

		/* re-direct command to stdout rather than spawning */
		else if (!strcmp(argv[i], "-r"))
			config.spawn = 0;
		else if (!strcmp(argv[i], "--debug"))
			DEBUG = 1;
		else if (i+1 == argc)
			usage();
		else
			usage();

	read_stdin();

	if (menu.head->tag)
		checkout_submenu(menu.head->tag);
	else
		checkout_submenu(NULL);

	menu.sel = menu.first;

	if (DEBUG)
		debug_dlist();

	ui_init();

	/* calculate menu geometry */
	menu.font_height =  ui_get_text_height(menu.font);
	menu.item_h = (menu.item_h > menu.font_height) ? menu.item_h : (menu.font_height);
	menu.menu_h = menu.nr_items * menu.item_h;
	if (menu.title)
		menu.menu_h += menu.item_h;
	menu.menu_w = 200;

	ui_get_screen_res(&menu.screen_x0, &menu.screen_y0, &menu.screen_w, &menu.screen_h);

	menu.win_y0 = menu.screen_y0 + menu.screen_h - menu.menu_h - 32;
	menu.win_x0 = 2;

	/* 
	 * FIXME Would be tidier to calc height to largest submenu rather than
	 * allocating memory for (menu.end * menu.item_h)
	 */
	ui_create_window(menu.win_x0, menu.win_y0, menu.menu_w, menu.menu_h, menu.end * menu.item_h);
	ui_init_cairo(menu.menu_w, menu.end * menu.item_h, menu.font);

	draw_menu();

	run();

	ui_cleanup();

	return 0;
}
