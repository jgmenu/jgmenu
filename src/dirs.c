/*
 * dirs.c - process schema file to produce directory structure
 *
 * Copyright (C) Johan Malm 2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <stdbool.h>

#include "dirs.h"
#include "xdgdirs.h"
#include "util.h"
#include "sbuf.h"
#include "list.h"
#include "compat.h"
#include "banned.h"

static struct dir *dirs;
static int nr_dirs, alloc_dirs;

static struct dir *add_dir(void)
{
	struct dir *dir;

	if (nr_dirs == alloc_dirs) {
		alloc_dirs = (alloc_dirs + 16) * 2;
		dirs = xrealloc(dirs, alloc_dirs * sizeof(struct dir));
	}
	dir = dirs + nr_dirs;
	memset(dir, 0, sizeof(*dir));
	nr_dirs++;
	return dir;
}

static void process_line(char *line)
{
	char *key, *value;
	static struct dir *dir;

	if (!parse_config_line(line, &key, &value))
		return;
	if (!strcmp("Name", key)) {
		dir = add_dir();
		dir->name = strdup(value);
	}
	if (!dir)
		die("dir not set '%s'", line);

	if (!strcmp("Icon", key))
		dir->icon = strdup(value);
}

static int process_file(char *filename)
{
	FILE *fp;
	char line[4096];

	BUG_ON(!filename);
	fp = fopen(filename, "r");
	if (!fp)
		return -1;
	while (fgets(line, sizeof(line), fp)) {
		char *p;

		if (line[0] == '\0')
			continue;
		p = strrchr(line, '\n');
		if (!p)
			continue;
		*p = '\0';
		process_line(line);
	}
	fclose(fp);
	return 0;
}

int dirs_nr(void)
{
	return nr_dirs;
}

struct dir *dirs_read_schema(void)
{
	struct list_head xdg_config_dirs;
	struct sbuf *dir;
	struct sbuf schema_filename;
	static bool has_already_run;

	BUG_ON(has_already_run);
	has_already_run = true;

	INIT_LIST_HEAD(&xdg_config_dirs);
	xdgdirs_get_configdirs(&xdg_config_dirs);

	sbuf_init(&schema_filename);

	/* Try /etc/xdg/, ~/.config/ and associated $XDG_CONFIG_* variables */
	list_for_each_entry(dir, &xdg_config_dirs, list) {
		sbuf_cpy(&schema_filename, dir->buf);
		sbuf_addstr(&schema_filename, "/jgmenu/schema");
		if (!process_file(schema_filename.buf))
			goto schema_read_success;
	}
	info("not able to find schema file");
schema_read_success:
	sbuf_expand_tilde(&schema_filename);
	xfree(schema_filename.buf);
	return dirs;
}
