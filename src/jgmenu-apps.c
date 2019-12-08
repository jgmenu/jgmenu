/*
 * jgmenu-apps.c - replacement for pmenu
 *
 * Copyright (C) Johan Malm 2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "desktop.h"
#include "dirs.h"
#include "util.h"
#include "sbuf.h"
#include "banned.h"

static void replace_semicolons_with_hashes(char *s)
{
	char *p;

	if (!s)
		return;
	while ((p = strchr(s, ';')))
		*p = '#';
	p = s + strlen(s) - 1;
	if (*p == '#')
		*p = '\0';
}

bool ismatch(struct argv_buf dir_categories, const char *app_categories)
{
	int i;

	for (i = 0; i < dir_categories.argc; i++) {
		if (dir_categories.argv[i][0] == '\0')
			continue;
		if (strstr(app_categories, dir_categories.argv[i]))
			return true;
	}
	return false;
}

void print_app_to_buffer(struct app *app, struct sbuf *buf)
{
	if (app->name_localized)
		sbuf_addstr(buf, app->name_localized);
	else
		sbuf_addstr(buf, app->name);
	sbuf_addstr(buf, ",");

	/* ^term() closing bracket is not needed */
	if (app->terminal)
		sbuf_addstr(buf, "^term(");
	sbuf_addstr(buf, app->exec);
	sbuf_addstr(buf, ",");

	sbuf_addstr(buf, app->icon);
	sbuf_addstr(buf, ",,");

	replace_semicolons_with_hashes(app->categories);
	sbuf_addstr(buf, "#");
	sbuf_addstr(buf, app->categories);
	sbuf_addstr(buf, "\n");
}

static void print_apps_in_other_directory(struct app *apps, struct sbuf *buf)
{
	struct app *app;

	for (app = apps; app->name; app += 1) {
		if (app->nodisplay)
			continue;
		if (app->has_been_mapped)
			continue;
		print_app_to_buffer(app, buf);
	}
}

static void print_apps_for_one_directory(struct app *apps, struct dir *dir,
					 struct sbuf *submenu)
{
	struct app *app;
	struct argv_buf categories;

	BUG_ON(!dir || !dir->categories);
	argv_set_delim(&categories, ';');
	argv_init(&categories);
	argv_strdup(&categories, dir->categories);
	argv_parse(&categories);

	for (app = apps; app->name; app += 1) {
		if (app->nodisplay)
			continue;

		/*
		 * dir->categories often contains a semi-colon at the end,
		 * giving an empty field which we ignore
		 */
		if (app->categories[0] == '\0')
			continue;

		/* Only include apps with the right categories */
		if (!ismatch(categories, app->categories))
			continue;

		app->has_been_mapped = true;
		print_app_to_buffer(app, submenu);
	}
}

static void print_menu_with_dirs(struct dir *dirs, struct app *apps)
{
	struct sbuf submenu;
	struct dir *dir;

	sbuf_init(&submenu);

	/* Draw top level menu */
	cat("~/.config/jgmenu/prepend.csv");
	for (dir = dirs; dir->name; dir += 1) {
		if (dir->name_localized)
			printf("%s", dir->name_localized);
		else
			printf("%s", dir->name);
		printf(",^checkout(apps-dir-%s),%s\n", dir->name, dir->icon);
	}
	cat("~/.config/jgmenu/append.csv");

	/* Draw submenus */
	for (dir = dirs; dir->name; dir += 1) {
		/* Should just be 'Other' directory */
		if (!dir->categories)
			continue;

		sbuf_cpy(&submenu, "");
		print_apps_for_one_directory(apps, dir, &submenu);

		if (!submenu.len)
			continue;
		printf("\n^tag(apps-dir-%s)\n%s", dir->name, submenu.buf);
	}

	/* Deal with any 'left-over' applications */
	sbuf_cpy(&submenu, "");
	print_apps_in_other_directory(apps, &submenu);
	if (submenu.len)
		printf("\n^tag(apps-dir-Other)\n%s", submenu.buf);

	xfree(submenu.buf);
}

int main(int argc, char **argv)
{
	int i = 1;
	struct app *apps;
	struct dir *dirs;

	while (i < argc) {
		if (!strncmp(argv[i], "--help", 6))
			printf("usage: jgmenu_run apps [--help]");
		else
			die("unknown option '%s'", argv[i]);
		i++;
	}
	dirs_read_schema(&dirs);
	apps = desktop_read_files();
	print_menu_with_dirs(dirs, apps);
	return 0;
}
