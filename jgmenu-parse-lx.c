#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <locale.h>
#include <menu-cache.h>

#include "sbuf.h"
#include "util.h"
#include "list.h"

struct menu {
	struct sbuf buf;
	struct menu *parent;
	struct list_head list;
};

static struct list_head menu;
static struct menu *cur;

static void print_menu(void)
{
	struct menu *m;

	list_for_each_entry(m, &menu, list)
		printf("%s", m->buf.buf);
}

static struct menu *menu_add(struct menu *parent)
{
	struct menu *m;

	m = xmalloc(sizeof(struct menu));
	sbuf_init(&m->buf);
	m->parent = parent;
	list_add_tail(&m->list, &menu);
	return m;
}

static void process_dir(MenuCacheApp *app)
{
	sbuf_addstr(&cur->buf, menu_cache_item_get_name(MENU_CACHE_ITEM(app)));
	sbuf_addstr(&cur->buf, ",^checkout(");
	sbuf_addstr(&cur->buf, menu_cache_item_get_id(MENU_CACHE_ITEM(app)));
	sbuf_addstr(&cur->buf, "),");
	sbuf_addstr(&cur->buf, menu_cache_item_get_icon(MENU_CACHE_ITEM(app)));
	sbuf_addstr(&cur->buf, "\n");
	cur = menu_add(cur);
	sbuf_addstr(&cur->buf, menu_cache_item_get_name(MENU_CACHE_ITEM(app)));
	sbuf_addstr(&cur->buf, ",^tag(");
	sbuf_addstr(&cur->buf, menu_cache_item_get_id(MENU_CACHE_ITEM(app)));
	sbuf_addstr(&cur->buf, ")\n");
}

static void process_app(MenuCacheApp *app)
{
	/* TODO: Check menu_cache_app_get_use_terminal(app) here */
	/* TODO: Check visibility flag here too */
	/* TODO: Remove all the % codes */

	sbuf_addstr(&cur->buf, menu_cache_item_get_name(MENU_CACHE_ITEM(app)));
	sbuf_addstr(&cur->buf, ",");
	sbuf_addstr(&cur->buf, menu_cache_app_get_exec(MENU_CACHE_APP(app)));
	sbuf_addstr(&cur->buf, ",");
	sbuf_addstr(&cur->buf, menu_cache_item_get_icon(MENU_CACHE_ITEM(app)));
	sbuf_addstr(&cur->buf, "\n");
}

static void traverse(MenuCacheDir *dir)
{
	GSList *l = NULL;

	for (l = menu_cache_dir_get_children(dir); l; l = l->next) {
		switch ((guint)menu_cache_item_get_type(MENU_CACHE_ITEM(l->data))) {
		case MENU_CACHE_TYPE_DIR:
			process_dir(l->data);
			traverse(MENU_CACHE_DIR(l->data));
			cur = cur->parent;
			break;
		case MENU_CACHE_TYPE_SEP:
			sbuf_addstr(&cur->buf, "^sep()\n");
			break;
		case MENU_CACHE_TYPE_APP:
			process_app(l->data);
		}
	}
}

int main(int argc, char **argv)
{
	MenuCache *cache;
	MenuCacheDir *rootdir;

	INIT_LIST_HEAD(&menu);
	cur = menu_add(NULL);
	setlocale (LC_ALL, "");

	/* $XDG_MENU_PREFIX needs to be set */
	cache = menu_cache_lookup_sync("applications.menu");
	if (!cache)
		die("cannot connect to menu-cache");
	rootdir = menu_cache_dup_root_dir(cache);
	if (!rootdir)
		die("no lx menu root directory");
	if (!g_slist_length(menu_cache_dir_get_children(rootdir)))
		die("lx menu is empty");
	traverse(rootdir);
	menu_cache_item_unref(MENU_CACHE_ITEM(rootdir));
	menu_cache_unref(cache);
	print_menu();
	return 0;
}
