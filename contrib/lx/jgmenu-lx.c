#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <locale.h>
#include <menu-cache.h>

#include "sbuf.h"
#include "util.h"
#include "list.h"
#include "back.h"
#include "xdgdirs.h"
#include "fmt.h"

struct menu {
	struct sbuf buf;
	struct menu *parent;
	struct list_head list;
};

static struct list_head menu;
static struct menu *cur;
static int no_dirs;
static int no_pend; /* no append or prepend */

static void append(void)
{
	static int done;

	if (done || no_pend)
		return;
	cat("~/.config/jgmenu/append.csv");
	done = 1;
}

static void print_menu(void)
{
	struct menu *m;

	if (!no_pend)
		cat("~/.config/jgmenu/prepend.csv");
	list_for_each_entry(m, &menu, list) {
		sbuf_replace(&m->buf, "&", "&amp;");
		printf("%s", m->buf.buf);
		append();
	}
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
	static char unique_number[8];
	static int nr;

	if (no_dirs) {
		cur = menu_add(cur);
		return;
	}
	snprintf(unique_number, sizeof(unique_number), "%d", nr);
	sbuf_addstr(&cur->buf, menu_cache_item_get_name(MENU_CACHE_ITEM(app)));
	sbuf_addstr(&cur->buf, ",^checkout(");
	sbuf_addstr(&cur->buf, menu_cache_item_get_id(MENU_CACHE_ITEM(app)));
	sbuf_addstr(&cur->buf, unique_number);
	sbuf_addstr(&cur->buf, "),");
	sbuf_addstr(&cur->buf, menu_cache_item_get_icon(MENU_CACHE_ITEM(app)));
	sbuf_addstr(&cur->buf, "\n");
	cur = menu_add(cur);
	sbuf_addstr(&cur->buf, menu_cache_item_get_name(MENU_CACHE_ITEM(app)));
	sbuf_addstr(&cur->buf, ",^tag(");
	sbuf_addstr(&cur->buf, menu_cache_item_get_id(MENU_CACHE_ITEM(app)));
	sbuf_addstr(&cur->buf, unique_number);
	sbuf_addstr(&cur->buf, ")\n");
	sbuf_addstr(&cur->buf, back_string());
	sbuf_addstr(&cur->buf, ",^back(),go-previous\n");
	++nr;
}

static void add_metadata(const char * const *categories)
{
	const char **c = (const char **)categories;
	int i;

	for (i = 0; c && c[i]; i++) {
		sbuf_addstr(&cur->buf, "#");
		sbuf_addstr(&cur->buf, c[i]);
	}
}

static void process_app(MenuCacheApp *app)
{
	/* TODO: Check visibility flag here too */
	char *p = NULL;
	char *exec;
	static struct sbuf s;
	static int inited;

	if (!inited) {
		sbuf_init(&s);
		inited = 1;
	}

	/* name */
	fmt_name(&s, menu_cache_item_get_name(MENU_CACHE_ITEM(app)),
		 menu_cache_app_get_generic_name(MENU_CACHE_APP(app)));
	if (strchr(s.buf, ','))
		sbuf_addstr(&cur->buf, "\"\"\"");
	sbuf_addstr(&cur->buf, s.buf);
	if (strchr(s.buf, ','))
		sbuf_addstr(&cur->buf, " \"\"\"");
	sbuf_addstr(&cur->buf, ",");

	/* command */
	if (menu_cache_app_get_use_terminal(app))
		sbuf_addstr(&cur->buf, "^term(");
	exec = (char *)menu_cache_app_get_exec(MENU_CACHE_APP(app));
	if (strchr(exec, ','))
		sbuf_addstr(&cur->buf, "\"\"\"");
	sbuf_addstr(&cur->buf, exec);
	if (strchr(exec, ','))
		sbuf_addstr(&cur->buf, " \"\"\"");
	/* TODO: be more sophisticated with handling '%'s */
	p = strchr(cur->buf.buf, '%');
	if (p) {
		*p = '\0';
		cur->buf.len = strlen(cur->buf.buf);
	}
	sbuf_rtrim(&cur->buf);
	if (menu_cache_app_get_use_terminal(app))
		sbuf_addstr(&cur->buf, ")");
	sbuf_addstr(&cur->buf, ",");

	/* icon */
	sbuf_addstr(&cur->buf, menu_cache_item_get_icon(MENU_CACHE_ITEM(app)));
	sbuf_addstr(&cur->buf, ",");

	/* working directory */
	sbuf_addstr(&cur->buf, menu_cache_app_get_working_dir(MENU_CACHE_APP(app)));
	sbuf_addstr(&cur->buf, ",");

	/* metadata */
	add_metadata(menu_cache_app_get_categories(MENU_CACHE_APP(app)));

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
			if (!no_dirs)
				sbuf_addstr(&cur->buf, "^sep()\n");
			break;
		case MENU_CACHE_TYPE_APP:
			process_app(l->data);
		}
	}
}

static void set_if_unset_xdg_menu_prefix(void)
{
	struct sbuf f;
	char *p, *q;

	if (getenv("XDG_MENU_PREFIX"))
		return;
	sbuf_init(&f);
	xdgdirs_find_menu_file(&f);
	if (!f.len)
		die("cannot find a menu file");
	p = strrchr(f.buf, '/');
	BUG_ON(!p);
	p++;
	q = strchr(p, '-');
	BUG_ON(!q);
	q++;
	*q = '\0';
	setenv("XDG_MENU_PREFIX", p, 1);
}

int main(int argc, char **argv)
{
	MenuCache *cache;
	MenuCacheDir *rootdir;

	INIT_LIST_HEAD(&menu);
	cur = menu_add(NULL);
	setlocale(LC_ALL, "");

	if (getenv("JGMENU_NO_DIRS"))
		no_dirs = 1;
	if (getenv("JGMENU_NO_PEND"))
		no_pend = 1;

	set_if_unset_xdg_menu_prefix();
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
