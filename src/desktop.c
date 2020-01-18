/*
 * desktop.c - read and process .desktop files
 *
 * Copyright (C) Johan Malm 2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>

#include "desktop.h"
#include "xdgdirs.h"
#include "util.h"
#include "sbuf.h"
#include "list.h"
#include "charset.h"
#include "compat.h"
#include "lang.h"
#include "banned.h"

static struct app *apps;
static int nr_apps, alloc_apps;

/* TODO: parse this properly and share code with lx */
static void format_exec(char *s)
{
	char *p;

	if (!s)
		return;
	/* Remove %U, %f, etc at the end of Exec cmd */
	p = strchr(s, '%');
	if (p)
		*p = '\0';
}

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
	if (!strcmp("Name", key)) {
		app->name = strdup(value);
	} else if (!strcmp("GenericName", key)) {
		app->generic_name = strdup(value);
	} else if (!strcmp("Exec", key)) {
		app->exec = strdup(value);
	} else if (!strcmp("Icon", key)) {
		app->icon = strdup(value);
	} else if (!strcmp("Categories", key)) {
		app->categories = strdup(value);
	} else if (!strcmp("NoDisplay", key)) {
		if (!strcasecmp(value, "true"))
			app->nodisplay = true;
	} else if (!strcmp("Terminal", key)) {
		if (!strcasecmp(value, "true"))
			app->terminal = true;
	}

	/* localized name */
	if (!strcmp(key, lang_name_llcc()))
		app->name_localized = xstrdup(value);
	if (!app->name_localized && !strcmp(key, lang_name_ll()))
		app->name_localized = xstrdup(value);

	/* localized generic name */
	if (!strcmp(key, lang_gname_llcc()))
		app->generic_name_localized = xstrdup(value);
	if (!app->generic_name_localized && !strcmp(key, lang_gname_ll()))
		app->generic_name_localized = xstrdup(value);
}

bool is_duplicate_desktop_file(char *filename)
{
	int i;

	if (!filename)
		return false;
	for (i = 0; i < nr_apps; i++) {
		if (!apps[i].filename)
			continue;
		if (!strcmp(apps[i].filename, filename))
			return true;
	}
	return false;
}

/**
 * This makes the code a bit simpler in jgmenu-apps.c
 */
static void strdup_null_variables(struct app *app)
{
	if (!app->name)
		app->name = strdup("");
	if (!app->name_localized)
		app->name_localized = strdup("");
	if (!app->generic_name)
		app->generic_name = strdup("");
	if (!app->generic_name_localized)
		app->generic_name_localized = strdup("");
	if (!app->exec)
		app->exec = strdup("");
	if (!app->icon)
		app->icon = strdup("");
	if (!app->categories)
		app->categories = strdup("");
	if (!app->filename)
		app->filename = strdup("");
}

static struct app *grow_vector_by_one_app(void)
{
	struct app *app;

	if (nr_apps == alloc_apps) {
		alloc_apps = (alloc_apps + 16) * 2;
		apps = xrealloc(apps, alloc_apps * sizeof(struct app));
	}
	app = apps + nr_apps;
	memset(app, 0, sizeof(*app));
	nr_apps++;
	return app;
}

static int add_app(FILE *fp, char *filename)
{
	char line[4096];
	char *p;
	struct app *app;
	int is_desktop_entry;

	app = grow_vector_by_one_app();
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
	format_exec(app->exec);
	app->filename = strdup(filename);
	strdup_null_variables(app);
	return 0;
}

static void process_file(char *filename, const char *path)
{
	FILE *fp;
	char fullname[4096];
	int ret = 0;
	size_t len;

	if (!strcasestr(filename, ".desktop"))
		return;
	len = strlen(path);
	strlcpy(fullname, path, sizeof(fullname));
	strlcpy(fullname + len, filename, sizeof(fullname) - len);
	fp = fopen(fullname, "r");
	if (!fp) {
		warn("could not open file %s", filename);
		return;
	}
	if (is_duplicate_desktop_file(filename))
		goto out;
	ret = add_app(fp, filename);
	if (ret < 0)
		warn("file '%s' is not utf-8 compatible", filename);
out:
	fclose(fp);
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

struct app *desktop_read_files(void)
{
	struct list_head xdg_data_dirs;
	struct sbuf *dir;
	struct sbuf s;
	static int has_already_run;

	BUG_ON(has_already_run);
	has_already_run = 1;

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

	/* NULL terminate vector */
	grow_vector_by_one_app();
	return apps;
}
