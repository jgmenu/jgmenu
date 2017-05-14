#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include "cache.h"
#include "util.h"

#define CACHE_LOCATION "~/.local/share/icons/jgmenu-cache"
static struct sbuf *cache_location;

static void mkdir_p(const char *path)
{
	struct sbuf s;
	char *p;

	sbuf_init(&s);
	if (strlen(path) > PATH_MAX) {
		warn("mkdir_p: path too long");
		return;
	}
	sbuf_cpy(&s, path);
	if (!s.len)
		return;
	sbuf_expand_tilde(&s);
	if (s.buf[s.len - 1] != '/')
		sbuf_addch(&s, '/');
	for (p = s.buf + 1; *p; p++) {
		if (*p != '/')
			continue;
		*p = '\0';
		if (mkdir(s.buf, S_IRWXU) != 0 && errno != EEXIST)
			warn("mkdir_p: could not create '%s'", s.buf);
		*p = '/';
	}
}

static int cache_write_index_theme(const char *buf, unsigned int size)
{
	struct sbuf f;
	int fd;

	sbuf_init(&f);
	sbuf_cpy(&f, cache_location->buf);
	sbuf_addstr(&f, "/index.theme");
	fd = open(f.buf, O_WRONLY | O_CREAT | O_EXCL, 0666);
	free(f.buf);
	if (fd < 0)
		return (errno == EEXIST) ? 0 : -1;
	if (write(fd, buf, size) < 0)
		warn("error writing index.theme");
	close(fd);
	return 0;
}

static void cache_create_index_theme(const char *theme, int size)
{
	struct sbuf buf;
	char iconsize[8];

	sbuf_init(&buf);
	sbuf_cpy(&buf, "Inherites=");
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
		char *option, *value;

		if (!parse_config_line(line, &option, &value))
			continue;
		if (!strncmp(option, "Inherits", 8)) {
			if (strcmp(value, theme) != 0)
				ret = -1;
		} else if (!strncmp(option, "Size", 4)) {
			if (atoi(value) != size)
				ret = -1;
		}
	}
	fclose(fp);
	if (ret < 0)
		warn("the icon theme and/or size has changed");
	return ret;
}

static void cache_init(void)
{
	static int first_run = 1;

	if (!first_run)
		return;
	cache_location = xmalloc(sizeof(struct sbuf));
	sbuf_init(cache_location);
	sbuf_cpy(cache_location, CACHE_LOCATION);
	sbuf_expand_tilde(cache_location);
	if (cache_check_index_theme("foo", 22) < 0) {
		fprintf(stderr, "info: creating index.theme\n");
		mkdir_p(CACHE_LOCATION);
		cache_create_index_theme("foo", 22);
	}
	first_run = 0;
}

int cache_exists(void)
{
	struct sbuf f;
	struct stat sb;

	sbuf_init(&f);
	sbuf_cpy(&f, cache_location->buf);
	sbuf_addstr(&f, "/index.theme");
	return (stat(f.buf, &sb) == 0) ? 1 : 0;
}

void resolve_symlink(struct sbuf *f)
{
	struct stat sb;
	char *linkname;
	ssize_t r, bufsiz;

	if (lstat(f->buf, &sb) == -1)
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
	free(cache_location->buf);
	free(cache_location);
}
