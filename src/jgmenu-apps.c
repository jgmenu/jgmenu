/*
 * jgmenu-apps.c - replacement for pmenu
 *
 * Copyright (C) Johan Malm 2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "desktop.h"
#include "util.h"
#include "sbuf.h"

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

static void print_desktop_files(struct app *apps, int nr_apps)
{
	int i;
	struct app *app;

	cat("~/.config/jgmenu/prepend.csv");
	for (i = 0; i < nr_apps; i++) {
		app = apps + i;
		if (app->nodisplay)
			continue;
		printf("%s,%s,%s,,", app->name, app->exec, app->icon);
		if (app->categories[0] == '\0')
			continue;
		replace_semicolons_with_hashes(app->categories);
		printf("#%s\n", app->categories);
	}
	cat("~/.config/jgmenu/append.csv");
}

int main(int argc, char **argv)
{
	int i = 1;
	struct app *apps;

	while (i < argc) {
		if (!strncmp(argv[i], "--help", 6))
			printf("usage: jgmenu_run apps [--help]");
		else
			die("unknown option '%s'", argv[i]);
		i++;
	}
	apps = desktop_read_files();
	print_desktop_files(apps, desktop_nr_apps());
	return 0;
}
