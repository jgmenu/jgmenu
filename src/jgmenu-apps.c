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

static void print_desktop_files(struct app *apps, int nr_apps)
{
	int i;

	cat("~/.config/jgmenu/prepend.csv");
	for (i = 0; i < nr_apps; i++) {
		if (!apps[i].name)
			continue;
		printf("%s,%s,%s\n", apps[i].name, apps[i].exec, apps[i].icon);
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
