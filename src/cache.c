#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

#include "cache.h"
#include "util.h"
#include "banned.h"

#define CACHE_LOCATION "~/.cache/jgmenu/icons"
static struct sbuf *cache_location;

static struct sbuf icon_theme;
static int icon_size;

void cache_set_icon_theme(const char *theme)
{
	static int first_run = 1;

	if (first_run) {
		sbuf_init(&icon_theme);
		first_run = 0;
	}
	if (theme)
		sbuf_cpy(&icon_theme, theme);
}

void cache_set_icon_size(int size)
{
	icon_size = size;
}

static int cache_write_index_theme(const char *buf, unsigned int size)
{
	struct sbuf f;
	FILE *file;

	sbuf_init(&f);
	sbuf_cpy(&f, cache_location->buf);
	sbuf_addstr(&f, "/index.theme");
	file = fopen(f.buf, "w");
	free(f.buf);
	if (!file)
		return -1;
	if (!fwrite(buf, size, 1, file))
		warn("error writing index.theme");
	fclose(file);
	return 0;
}

static void cache_create_index_theme(const char *theme, int size)
{
	struct sbuf buf;
	char iconsize[8];

	sbuf_init(&buf);
	sbuf_cpy(&buf, "Inherits=");
	sbuf_addstr(&buf, theme);
	sbuf_addch(&buf, '\n');
	sbuf_addstr(&buf, "Size=");
	snprintf(iconsize, 8, "%d", size);
	sbuf_addstr(&buf, iconsize);
	sbuf_addch(&buf, '\n');
	if (cache_write_index_theme(buf.buf, buf.len) < 0)
		warn("error writing index.theme");
	free(buf.buf);
}

static int cache_check_index_theme(const char *theme, int size)
{
	struct sbuf f;
	FILE *fp;
	char line[2048];
	int ret = 0;
	int theme_correct = 0, size_correct = 0;

	sbuf_init(&f);
	sbuf_cpy(&f, cache_location->buf);
	sbuf_addstr(&f, "/index.theme");
	fp = fopen(f.buf, "r");
	free(f.buf);
	if (!fp) {
		warn("icon cache does not exist");
		return -1;
	}
	while (fgets(line, sizeof(line), fp)) {
		char *option, *value, *p;

		p = strchr(line, '\n');
		if (p)
			*p = '\0';
		if (!parse_config_line(line, &option, &value))
			continue;
		if (!strncmp(option, "Inherits", 8)) {
			if (!strcmp(value, theme))
				theme_correct = 1;
		} else if (!strncmp(option, "Size", 4)) {
			if (atoi(value) == size)
				size_correct = 1;
		}
	}
	fclose(fp);
	if (!theme_correct || !size_correct)
		ret = -1;
	if (ret < 0)
		info("icon theme and/or size has changed");
	return ret;
}

static void cache_delete(void)
{
	char cmd[512];
	int ret;

	if (!cache_location || !cache_location->len)
		die("must do cache_init() before cache_delete()!");
	if (cache_location->len > 500)
		die("path to icon path is too long");
	snprintf(cmd, sizeof(cmd), "rm -rf %s", cache_location->buf);
	cmd[511] = '\0';
	ret = system(cmd);
	if (ret)
		warn("deleting cache returned %d (cmd='%s')", ret, cmd);
}

static void cache_init(void)
{
	static int first_run = 1;

	if (!first_run)
		return;
	if (!icon_theme.len || !icon_size)
		die("cache.c: icon_{theme,size} needs to be set");
	cache_location = xmalloc(sizeof(struct sbuf));
	sbuf_init(cache_location);
	sbuf_cpy(cache_location, CACHE_LOCATION);
	sbuf_expand_tilde(cache_location);
	if (cache_check_index_theme(icon_theme.buf, icon_size) < 0) {
		cache_delete();
		mkdir_p(CACHE_LOCATION);
		cache_create_index_theme(icon_theme.buf, icon_size);
	}
	first_run = 0;
}

static void resolve_symlink(struct sbuf *f)
{
	struct stat sb;
	char *linkname;
	ssize_t r, bufsiz;

	if (!f || !f->buf) {
		warn("empty argument passed to resolve_symlink()");
		return;
	}
	if (lstat(f->buf, &sb) != 0)
		return;
	/* The cache contains empty regular files for missing icons */
	if ((sb.st_mode & S_IFMT) == S_IFREG)
		return;
	bufsiz = sb.st_size + 1;
	if (sb.st_size == 0)
		bufsiz = PATH_MAX;
	linkname = xmalloc(bufsiz);
	r = readlink(f->buf, linkname, bufsiz);
	if (r == -1) {
		warn("readlink error for %s", f->buf);
		return;
	}
	linkname[r] = '\0';
	if (r == bufsiz)
		warn("readlink: returned buffer may have been truncated");
	sbuf_cpy(f, linkname);
	free(linkname);
}

int cache_touch(const char *name)
{
	struct sbuf f;
	int ret = 0;

	if (!name || name[0] == '\0')
		return -1;
	cache_init();
	sbuf_init(&f);
	sbuf_cpy(&f, cache_location->buf);
	sbuf_addch(&f, '/');
	sbuf_addstr(&f, name);
	ret = open(f.buf, O_WRONLY | O_CREAT | O_TRUNC,
		   S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	free(f.buf);
	return ret;
}

int cache_strdup_path(const char *name, struct sbuf *path)
{
	struct sbuf f;
	struct stat sb;
	int ret = 0;

	if (!name || name[0] == '\0')
		return -1;
	cache_init();
	sbuf_init(&f);
	sbuf_cpy(&f, cache_location->buf);
	sbuf_addch(&f, '/');
	sbuf_addstr(&f, name);
	resolve_symlink(&f);
	if (stat(f.buf, &sb) == 0) {
		sbuf_cpy(path, f.buf);
		ret = 1;
	}
	free(f.buf);
	return ret;
}

int cache_create_symlink(char *path, char *name)
{
	struct sbuf target, linkpath;
	int ret = 0;

	cache_init();
	if (!path || path[0] == '\0')
		return -1;
	sbuf_init(&linkpath);
	sbuf_cpy(&linkpath, cache_location->buf);
	sbuf_addch(&linkpath, '/');
	sbuf_addstr(&linkpath, name);
	sbuf_init(&target);
	sbuf_cpy(&target, path);
	if (symlink(target.buf, linkpath.buf) == -1)
		ret = (errno == EEXIST) ? 0 : -1;
	if (ret < 0)
		warn("cache: failed to create symlink for '%s'", target.buf);
	free(target.buf);
	free(linkpath.buf);
	return ret;
}

void cache_atexit_cleanup(void)
{
	xfree(cache_location->buf);
	xfree(cache_location);
}
