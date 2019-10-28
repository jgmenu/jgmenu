/*
 * watch.c
 *
 * Copyright (C) Johan Malm 2018
 *
 * Watch files for change in "modified time" to advise when restart is
 * required.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "list.h"
#include "sbuf.h"
#include "util.h"
#include "watch.h"
#include "config.h"
#include "banned.h"

static const char * const files_to_watch[] = {
	"~/.config/jgmenu/jgmenurc",
	"~/.config/jgmenu/prepend.csv",
	"~/.config/jgmenu/append.csv",
	"~/.config/tint2/tint2rc",
	"~/.local/share/applications",
	"/usr/share/applications",
	"/usr/local/share/applications",
	"/opt/share/applications",
	"$XDG_DATA_DIRS/applications",
	"~/.config/openbox/menu.xml",
	NULL
};

static LIST_HEAD(watched_files);

struct watched_file {
	char *filename;
	struct timeval tv;
	struct list_head list;
};

static void add_file(const char *filename)
{
	struct stat sb;
	struct watched_file *watched_file;
	struct sbuf f;

	sbuf_init(&f);
	sbuf_addstr(&f, filename);
	sbuf_expand_tilde(&f);
	sbuf_expand_env_var(&f);
	watched_file = malloc(sizeof(struct watched_file));
	watched_file->filename = f.buf;
	/*
	 * We add files even if they don't yet exist in order to be able
	 * to detect if they are added in the future.
	 */
	if (stat(f.buf, &sb) == -1)
		watched_file->tv.tv_sec = 0;
	else
		watched_file->tv.tv_sec = sb.st_mtime;
	list_add_tail(&watched_file->list, &watched_files);
}

void watch_init(void)
{
	static int done;
	int i;

	if (done)
		return;
	done = 1;
	for (i = 0; files_to_watch[i]; i++)
		add_file(files_to_watch[i]);
}

int watch_files_have_changed(void)
{
	struct watched_file *f;
	struct stat sb;

	watch_init();
	list_for_each_entry(f, &watched_files, list) {
		if (stat(f->filename, &sb) == -1) {
			if (!f->tv.tv_sec) {
				continue;
			} else {
				if (config.verbosity >= 2)
					info("file/dir removed '%s'",
					     f->filename);
				return 1;
			}
		}
		if (f->tv.tv_sec != sb.st_mtime) {
			if (config.verbosity < 2)
				return 1;
			if (!f->tv.tv_sec)
				info("file/dir added '%s'", f->filename);
			else
				info("file/dir changed '%s'", f->filename);
			return 1;
		}
	}
	return 0;
}

void watch_cleanup(void)
{
	struct watched_file *f, *tmp;

	list_for_each_entry_safe(f, tmp, &watched_files, list) {
		xfree(f->filename);
		list_del(&f->list);
		xfree(f);
	}
}
