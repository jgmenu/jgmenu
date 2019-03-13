/*
 * desktop.c - read and process .desktop files
 *
 * Copyright (C) Johan Malm 2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "desktop.h"
#include "xdgdirs.h"
#include "util.h"
#include "sbuf.h"
#include "list.h"
#include "charset.h"
#include "compat.h"

static struct app *apps;
static int nr_apps, alloc_apps;

//static void format_exec(void)
//{
//	// FIXME: tidy up
//	/* Remove %U, %f, etc at the end of Exec cmd */
//	if (desktop_file->exec) {
//		p = strchr(desktop_file->exec, '%');
//		if (p)
//			*p = '\0';
//	}
//}

static void parse_line(char *line, struct app *app, int *is_desktop_entry)
{
	char *key, *value;

	/* We only read the [Desktop Entry] section of a .desktop file */
	if (line[0] == '[') {
		if (!strncmp(line, "[Desktop Entry]", 15))
			*is_desktop_entry = 1;
		else
			*is_desktop_entry = 0;
	}
	if (!(*is_desktop_entry))
		return;

	if (!parse_config_line(line, &key, &value))
		return;
	if (!strcmp("Name", key))
		app->name = strdup(value);
	else if (!strcmp("Exec", key))
		app->exec = strdup(value);
	else if (!strcmp("Icon", key))
		app->icon = strdup(value);
	else if (!strcmp("Categories", key))
		app->categories = strdup(value);
	else if (!strcmp("NoDisplay", key))
		if (!strcasecmp(value, "true"))
			app->nodisplay = 1;
}

static int add_app(FILE *fp)
{
	char line[4096];
	char *p;
	struct app *app;
	int is_desktop_entry;

	if (nr_apps == alloc_apps) {
		alloc_apps = (alloc_apps + 16) * 2;
		apps = xrealloc(apps, alloc_apps * sizeof(struct app));
	}
	app = apps + nr_apps;
	memset(app, 0, sizeof(*app));
	nr_apps++;

	is_desktop_entry = 0;
	while (fgets(line, sizeof(line), fp)) {
		if (line[0] == '\0')
			continue;
		p = strrchr(line, '\n');
		if (!p)
			continue;
		*p = '\0';
		if (!utf8_validate(line, p - &line[0]))
			return -1;
		parse_line(line, app, &is_desktop_entry);
	}
	if (app->nodisplay || !app->name)
		--nr_apps;
	if (app->nodisplay)
		info("app %s is NoDisplay", app->name);
	return 0;
}

static void process_file(char *filename, const char *path)
{
	FILE *fp;
	char fullname[4096];
	int ret;
	size_t len;

	len = strlen(path);
	strlcpy(fullname, path, sizeof(fullname));
	strlcpy(fullname + len, filename, sizeof(fullname) - len);
	fp = fopen(fullname, "r");
	if (!fp) {
		warn("could not open file %s", filename);
		return;
	}
	ret = add_app(fp);
	fclose(fp);
	if (ret < 0)
		warn("file '%s' is not utf-8 compatible", filename);
}

static void traverse_directory(const char *path)
{
	struct dirent *entry;
	DIR *dp;

	dp = opendir(path);
	if (!dp)
		return;
	while ((entry = readdir(dp))) {
		if (!strncmp(entry->d_name, ".", 1) ||
		    !strncmp(entry->d_name, "..", 2))
			continue;
		process_file(entry->d_name, path);
	}
	closedir(dp);
}

static int compare_app_name(const void *a, const void *b)
{
	const struct app *aa = (struct app *)a;
	const struct app *bb = (struct app *)b;

	BUG_ON(!aa->name || !bb->name);
	return strcasecmp(aa->name, bb->name);
}

int desktop_nr_apps(void)
{
	return nr_apps;
}

struct app *desktop_read_files(void)
{
	struct list_head xdg_data_dirs;
	struct sbuf *dir;
	struct sbuf s;
	static int run;

	BUG_ON(run);
	run = 1;

	INIT_LIST_HEAD(&xdg_data_dirs);
	xdgdirs_get_basedirs(&xdg_data_dirs);

	sbuf_init(&s);
	list_for_each_entry(dir, &xdg_data_dirs, list) {
		sbuf_cpy(&s, dir->buf);
		sbuf_addstr(&s, "/applications/");
		/* populate *apps based on .desktop files */
		traverse_directory(s.buf);
	}
	qsort(apps, nr_apps, sizeof(struct app), compare_app_name);
	xfree(s.buf);
	return apps;
}
