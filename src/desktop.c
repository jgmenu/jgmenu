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
#include <glib.h>

#include "desktop.h"
#include "xdgdirs.h"
#include "util.h"
#include "sbuf.h"
#include "list.h"
#include "charset.h"
#include "compat.h"
#include "lang.h"
#include "isprog.h"
#include "banned.h"

static struct app *apps;
static int nr_apps, alloc_apps;

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
		strlcpy(app->name, value, sizeof(app->name));
	} else if (!strcmp("GenericName", key)) {
		strlcpy(app->generic_name, value, sizeof(app->generic_name));
	} else if (!strcmp("Exec", key)) {
		strlcpy(app->exec, value, sizeof(app->exec));
	} else if (!strcmp("TryExec", key)) {
		strlcpy(app->tryexec, value, sizeof(app->tryexec));
	} else if (!strcmp("Path", key)) {
		strlcpy(app->working_dir, value, sizeof(app->working_dir));
	} else if (!strcmp("Icon", key)) {
		strlcpy(app->icon, value, sizeof(app->icon));
	} else if (!strcmp("Categories", key)) {
		strlcpy(app->categories, value, sizeof(app->categories));
	} else if (!strcmp("NoDisplay", key)) {
		if (!strcasecmp(value, "true"))
			app->nodisplay = true;
	} else if (!strcmp("Terminal", key)) {
		if (!strcasecmp(value, "true"))
			app->terminal = true;
	}

	/* localized name */
	if (!strcmp(key, lang_name_llcc()))
		strlcpy(app->name_localized, value,
			sizeof(app->name_localized));
	if (app->name_localized[0] == '\0' && !strcmp(key, lang_name_ll()))
		strlcpy(app->name_localized, value,
			sizeof(app->name_localized));

	/* localized generic name */
	if (!strcmp(key, lang_gname_llcc()))
		strlcpy(app->generic_name_localized, value,
			sizeof(app->generic_name_localized));
	if (app->generic_name_localized[0] == '\0' &&
	    !strcmp(key, lang_gname_ll()))
		strlcpy(app->generic_name_localized, value,
			sizeof(app->generic_name_localized));
}

static bool is_duplicate_desktop_file(char *filename)
{
	int i;

	if (!filename)
		return false;
	for (i = 0; i < nr_apps; i++) {
		if (!strcmp(apps[i].filename, filename))
			return true;
	}
	return false;
}

static void delchar(char *p)
{
	size_t len = strlen(p);

	memmove(p, p + 1, len);
	*(p + len) = '\0';
}


/*
 * Remove all %? fields from .desktop Exec= field
 * Note:
 *  (a) %% which becomes %
 *  (b) backslash escaped characters are resolved
 */
static void strip_exec_field_codes(char **exec)
{
	if (!**exec || !*exec)
		return;
	for (char *p = *exec; *p; p++) {
		if (*p == '\\') {
			delchar(p);
			continue;
		}
		if (*p == '%') {
			delchar(p);
			if (*p == '\0')
				break;
			if (*p != '%')
				delchar(p);
		}
	}
	rtrim(exec);
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
	strlcpy(app->filename, filename, sizeof(app->filename));
	p = &app->exec[0];
	strip_exec_field_codes(&p);

	if (app->tryexec[0] != '\0' && !isprog(app->tryexec))
		app->tryexec_not_in_path = true;

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
		if (entry->d_type == DT_DIR) {
			if (entry->d_name[0] != '.') {
				char new_path[PATH_MAX];

				snprintf(new_path, PATH_MAX, "%s%s/", path,
					 entry->d_name);
				traverse_directory(new_path);
			}
		} else if (entry->d_type == DT_REG || entry->d_type == DT_LNK) {
			process_file(entry->d_name, path);
		}
	}
	closedir(dp);
}

static int compare_app_name(const void *a, const void *b)
{
	const struct app *aa = (struct app *)a;
	const struct app *bb = (struct app *)b;
	const char *aa_name, *bb_name;
	int ret;

	BUG_ON(!aa->name || !bb->name);
	aa_name = aa->name_localized[0] != '\0' ? aa->name_localized : aa->name;
	bb_name = bb->name_localized[0] != '\0' ? bb->name_localized : bb->name;
	aa_name = g_utf8_casefold(aa_name, -1);
	bb_name = g_utf8_casefold(bb_name, -1);
	ret = strcmp(aa_name, bb_name);
	xfree(aa_name);
	xfree(bb_name);
	return ret;
}

struct app *desktop_read_files(void)
{
	struct list_head xdg_data_dirs;
	struct sbuf *dir;
	struct sbuf s;
	static int has_already_run;
	struct app *app;

	BUG_ON(has_already_run);
	has_already_run = 1;

	INIT_LIST_HEAD(&xdg_data_dirs);
	xdgdirs_get_datadirs(&xdg_data_dirs);

	sbuf_init(&s);
	list_for_each_entry(dir, &xdg_data_dirs, list) {
		sbuf_cpy(&s, dir->buf);
		sbuf_addstr(&s, "/applications/");
		/* populate *apps based on .desktop files */
		traverse_directory(s.buf);
	}
	qsort(apps, nr_apps, sizeof(struct app), compare_app_name);
	xfree(s.buf);
	sbuf_list_free(&xdg_data_dirs);

	/* NULL terminate vector */
	app = grow_vector_by_one_app();
	app->end = true;
	return apps;
}
