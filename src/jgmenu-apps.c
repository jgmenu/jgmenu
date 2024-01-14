/*
 * jgmenu-apps.c - replacement for pmenu
 *
 * Copyright (C) Johan Malm 2019
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "desktop.h"
#include "dirs.h"
#include "util.h"
#include "sbuf.h"
#include "fmt.h"
#include "i18n.h"
#include "banned.h"

static char *append_filename;
static char *prepend_filename;
static bool no_append;
static bool no_prepend;
static bool single_window;
static bool no_duplicates;

static const struct option long_options[] = {
	{"append-file", required_argument, NULL, 'a'},
	{"no-append", no_argument, NULL, 'n'},
	{"no-prepend", no_argument, NULL, 'm'},
	{"prepend-file", required_argument, NULL, 'p'},
	{"help", no_argument, NULL, 'h'},
	{0, 0, 0, 0}
};

static const char jgmenu_apps_usage[] =
"Usage: jgmenu_run apps [options...]\n"
"  -a, --append-file <file>   Specify menu file to append to the root menu\n"
"                             Use $HOME/.config/jgmenu/append.csv by default\n"
"  -h, --help                 Show help message and quit\n"
"  -n, --no-append            Do not use an append file\n"
"  -m, --no-prepend           Do not use a prepend file\n"
"  -p, --prepend-file <file>  Specify menu file to prepend to the root menu\n"
"                             Use $HOME/.config/jgmenu/prepend.csv by default\n";

static void
usage(void)
{
	printf("%s", jgmenu_apps_usage);
	exit(0);
}

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

static bool ismatch(struct argv_buf dir_categories, const char *app_categories)
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

static void print_app_to_buffer(struct app *app, struct sbuf *buf)
{
	struct sbuf s;
	char *name, *generic_name;

	sbuf_init(&s);
	name = app->name_localized[0] != '\0' ? app->name_localized : app->name;
	generic_name = app->generic_name_localized[0] != '\0' ?
			       app->generic_name_localized :
			       app->generic_name;

	/* name */
	sbuf_cpy(&s, name);
	fmt_name(&s, name, generic_name);
	sbuf_replace(&s, "&", "&amp;");
	sbuf_addstr(buf, "\"\"\"");
	sbuf_addstr(buf, s.buf);
	sbuf_addstr(buf, "\"\"\"");
	sbuf_addstr(buf, ",");

	/* command */
	/* ^term() closing bracket is not needed */
	if (app->terminal)
		sbuf_addstr(buf, "^term(");
	sbuf_addstr(buf, app->exec);
	sbuf_addstr(buf, ",");

	/* icon */
	sbuf_addstr(buf, app->icon);
	sbuf_addstr(buf, ",");

	/* working directory */
	sbuf_addstr(buf, app->working_dir);
	sbuf_addstr(buf, ",");

	/* metadata */
	replace_semicolons_with_hashes(app->categories);
	sbuf_addstr(buf, "#");
	sbuf_addstr(buf, app->categories);
	sbuf_addstr(buf, "\n");

	xfree(s.buf);
}

static bool should_not_display(struct app *app)
{
	return app->nodisplay || app->tryexec_not_in_path;
}

static void print_apps_in_other_directory(struct app *apps, struct sbuf *buf)
{
	struct app *app;

	for (app = apps; !app->end; app += 1) {
		if (should_not_display(app))
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

	for (app = apps; !app->end; app += 1) {
		if (should_not_display(app))
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

		if (no_duplicates && app->has_been_mapped)
			continue;
		app->has_been_mapped = true;
		print_app_to_buffer(app, submenu);
	}
	xfree(categories.buf);
}

static void print_menu_with_dirs(struct dir *dirs, struct app *apps)
{
	struct sbuf submenu;
	struct dir *dir;

	sbuf_init(&submenu);

	/* Draw top level menu */
	if (!no_prepend)
		i18n_cat(prepend_filename ? : "~/.config/jgmenu/prepend.csv");
	for (dir = dirs; dir->name; dir += 1) {
		if (dir->name_localized)
			printf("%s", dir->name_localized);
		else
			printf("%s", dir->name);
		if (!single_window)
			printf(",^checkout(apps-dir-%s),%s\n", dir->name,
			       dir->icon);
		else
			printf(",^root(apps-dir-%s),%s\n", dir->name,
			       dir->icon);
	}
	if (!no_append)
		i18n_cat(append_filename ? : "~/.config/jgmenu/append.csv");

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

static void print_menu_no_dirs(struct app *apps)
{
	struct app *app;
	struct sbuf buf;

	sbuf_init(&buf);
	if (!no_prepend)
		cat(prepend_filename ? : "~/.config/jgmenu/prepend.csv");
	for (app = apps; !app->end; app += 1) {
		if (should_not_display(app))
			continue;
		print_app_to_buffer(app, &buf);
	}
	if (!no_append)
		cat(append_filename ? : "~/.config/jgmenu/append.csv");
	printf("\n%s\n", buf.buf);
	xfree(buf.buf);
}

int main(int argc, char **argv)
{
	struct app *apps;
	struct dir *dirs;

	int c;
	while (1) {
		int index = 0;
		c = getopt_long(argc, argv, "a:nmp:h", long_options, &index);
		if (c == -1) {
			break;
		}
		switch (c) {
		case 'a':
			append_filename = optarg;
			break;
		case 'n':
			no_append = true;
			break;
		case 'm':
			no_prepend = true;
			break;
		case 'p':
			prepend_filename = optarg;
			break;
		case 'h':
		default:
			usage();
		}
	}
	if (optind < argc) {
		usage();
	}

	if (getenv("JGMENU_NO_PEND")) {
		no_append = true;
		no_prepend = true;
	}
	if (getenv("JGMENU_SINGLE_WINDOW"))
		single_window = true;
	if (getenv("JGMENU_NO_DUPLICATES"))
		no_duplicates = true;
	i18n_init(getenv("JGMENU_I18N"));

	apps = desktop_read_files();
	if (getenv("JGMENU_NO_DIRS")) {
		print_menu_no_dirs(apps);
	} else {
		dirs_read_schema(&dirs);
		print_menu_with_dirs(dirs, apps);
	}
	i18n_cleanup();
	xfree(apps);
	return 0;
}
