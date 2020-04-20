/*
 * hooks.c
 *
 * Copyright (C) Johan Malm 2018
 *
 * Watch files for change in "modified time" and take action (incl. restart)
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <stdbool.h>

#include "list.h"
#include "sbuf.h"
#include "util.h"
#include "hooks.h"
#include "config.h"
#include "restart.h"
#include "spawn.h"
#include "banned.h"

static const char * const files_to_watch[] = {
	"~/.config/jgmenu/jgmenurc",
	"~/.config/jgmenu/prepend.csv",
	"~/.config/jgmenu/append.csv",
	"~/.config/jgmenu/hooks",
	"~/.config/tint2/tint2rc",
	"~/.local/share/applications",
	"/usr/share/applications",
	"/usr/local/share/applications",
	"/opt/share/applications",
	"~/.config/openbox/menu.xml",
	NULL
};

static LIST_HEAD(hooks);
static bool need_restart;

struct hook {
	char *watched_file;
	char *action;
	struct timeval tv;
	struct list_head list;
};

static void clear_list(void)
{
	struct hook *f, *tmp;

	list_for_each_entry_safe(f, tmp, &hooks, list) {
		xfree(f->watched_file);
		xfree(f->action);
		list_del(&f->list);
		xfree(f);
	}
}

static void print_hooks(void)
{
	struct hook *f;

	list_for_each_entry(f, &hooks, list) {
		printf("[hooks] %s", f->watched_file);
		if (f->action)
			printf(" --> %s", f->action);
		printf("\n");
	}
}

static void add_hook(const char *filename, const char *action)
{
	struct stat sb;
	struct hook *hook;
	struct sbuf f;

	sbuf_init(&f);
	sbuf_addstr(&f, filename);
	sbuf_expand_tilde(&f);
	sbuf_expand_env_var(&f);
	hook = malloc(sizeof(struct hook));
	hook->watched_file = f.buf;
	hook->action = action ? xstrdup(action) : NULL;

	/*
	 * We add files even if they don't yet exist in order to be able
	 * to detect if they are added in the future.
	 */
	if (stat(f.buf, &sb) == -1)
		hook->tv.tv_sec = 0;
	else
		hook->tv.tv_sec = sb.st_mtime;
	list_add_tail(&hook->list, &hooks);
}

static void process_one_line(const char *line)
{
	char *action;

	if (!line)
		return;
	if (!strcmp(line, "clear")) {
		clear_list();
		return;
	}
	if (!strcmp(line, "print")) {
		print_hooks();
		return;
	}
	action = strrchr(line, ',');
	if (action)
		*(action++) = '\0';
	add_hook(line, action);
}

static void add_user_defined_hooks(const char *filename)
{
	FILE *fp;
	char line[4096];
	char *p;
	struct sbuf f;

	if (!filename)
		return;
	sbuf_init(&f);
	sbuf_addstr(&f, filename);
	sbuf_expand_tilde(&f);
	fp = fopen(f.buf, "r");
	if (!fp) {
		info("could not open hooks file %s", filename);
		goto clean;
	}
	while (fgets(line, sizeof(line), fp)) {
		if (line[0] == '\0')
			continue;
		p = strrchr(line, '\n');
		if (!p)
			continue;
		*p = '\0';
		process_one_line(line);
	}
clean:
	xfree(f.buf);
}

static void add_built_in_hooks(void)
{
	int i;

	for (i = 0; files_to_watch[i]; i++)
		add_hook(files_to_watch[i], NULL);
}

void hooks_init(void)
{
	static bool done;

	if (done)
		return;
	done = true;
	add_built_in_hooks();
	add_user_defined_hooks("~/.config/jgmenu/hooks");
}

static void run_hook_action(const char *command)
{
	/*
	 * We always restart on file/dir change - whether there is an
	 * associated action or not
	 */
	need_restart = true;
	if (!command)
		return;
	info("execute '%s'", command);
	spawn_command_line_sync(command);
}

static void check_and_action_hooks(void)
{
	struct hook *f;
	struct stat sb;

	hooks_init();
	list_for_each_entry(f, &hooks, list) {
		if (stat(f->watched_file, &sb) == -1) {
			if (!f->tv.tv_sec) {
				continue;
			} else {
				if (config.verbosity >= 2)
					info("file/dir removed '%s'",
					     f->watched_file);
				run_hook_action(f->action);
				continue;
			}
		}
		if (f->tv.tv_sec != sb.st_mtime) {
			if (config.verbosity >= 2 && !f->tv.tv_sec)
				info("file/dir added '%s'", f->watched_file);
			else if (config.verbosity >= 2 && f->tv.tv_sec)
				info("file/dir changed '%s'", f->watched_file);
			run_hook_action(f->action);
			continue;
		}
	}
}

void hooks_check(void)
{
	check_and_action_hooks();
	if (need_restart) {
		restart();
		need_restart = false;
	}
}

void hooks_cleanup(void)
{
	clear_list();
}
