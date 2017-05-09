#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <menu-cache.h>

#include "sbuf.h"
#include "util.h"

static struct sbuf root, sub;

void process_dir(MenuCacheApp *app)
{
	sbuf_addstr(&root, menu_cache_item_get_id(MENU_CACHE_ITEM(app)));
	sbuf_addstr(&root, ",^checkout(");
	sbuf_addstr(&root, menu_cache_item_get_name(MENU_CACHE_ITEM(app)));
	sbuf_addstr(&root, "),");
	sbuf_addstr(&root, menu_cache_item_get_icon(MENU_CACHE_ITEM(app)));
	sbuf_addstr(&root, "\n");
	sbuf_addstr(&sub, menu_cache_item_get_id(MENU_CACHE_ITEM(app)));
	sbuf_addstr(&sub, ",^tag(");
	sbuf_addstr(&sub, menu_cache_item_get_name(MENU_CACHE_ITEM(app)));
	sbuf_addstr(&sub, ")\n");
}

void process_app(MenuCacheApp *app)
{
	/* TODO: Check menu_cache_app_get_use_terminal(app) here */
	/* TODO: Check visibility flag here too */
	/* TODO: Remove all the % codes */

	sbuf_addstr(&sub, menu_cache_item_get_name(MENU_CACHE_ITEM(app)));
	sbuf_addstr(&sub, ",");
	sbuf_addstr(&sub, menu_cache_app_get_exec(MENU_CACHE_APP(app)));
	sbuf_addstr(&sub, ",");
	sbuf_addstr(&sub, menu_cache_item_get_icon(MENU_CACHE_ITEM(app)));
	sbuf_addstr(&sub, "\n");
}

void traverse(MenuCacheDir *dir)
{
	GSList *l = NULL;

	for (l = menu_cache_dir_get_children(dir); l; l = l->next) {
		switch ((guint)menu_cache_item_get_type(MENU_CACHE_ITEM(l->data))) {
		case MENU_CACHE_TYPE_DIR:
			process_dir(l->data);
			traverse(MENU_CACHE_DIR(l->data));
			break;
		case MENU_CACHE_TYPE_SEP:
			sbuf_addstr(&sub, "^sep()\n");
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

	sbuf_init(&root);
	sbuf_init(&sub);

	/* $XDG_MENU_PREFIX needs to be set */
	cache = menu_cache_lookup_sync("-applications.menu");
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
	printf("%s", root.buf);
	printf("%s", sub.buf);
	free(root.buf);
	free(sub.buf);
	return 0;
}
