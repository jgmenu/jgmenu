/*
 * icon-find.c: Finds icon by name, theme and size
 *
 * Loosely iaw XDG spec
 */

#include <ftw.h>
#include <dirent.h>

#include "icon-find.h"
#include "xdgdirs.h"
#include "list.h"
#include "util.h"
#include "banned.h"

#define DEBUG_PRINT_FINAL_SELECTION 0
#define DEBUG_PRINT_ALL_HITS 0		/* regardless of size */
#define DEBUG_PRINT_INHERITED_THEMES 0
#define DEBUG_PRINT_ICON_DIRS 0

/*
 * e.g. "/usr/share/icons", "/usr/loca/share/icons"
 * see xdgdirs.c for more details
 */
static struct list_head icon_dirs;
static struct list_head pixmap_dirs;

/* e.g. "Adwaita", "default", "hicolor" */
static struct list_head theme_list;

static int has_been_inited;

/* Variables used in the "find" algorithm */
static int  requested_icon_size;
static int base_dir_length;

static void get_parent_themes(struct list_head *parent_themes, const char *child_theme)
{
	FILE *fp;
	char line[1024];
	char *option, *value;
	struct sbuf *s;
	struct sbuf filename;

	sbuf_init(&filename);

	list_for_each_entry(s, &icon_dirs, list) {
		sbuf_cpy(&filename, s->buf);
		sbuf_addstr(&filename, "/");
		sbuf_addstr(&filename, child_theme);
		sbuf_addstr(&filename, "/index.theme");

		fp = fopen(filename.buf, "r");
		if (!fp)
			continue;

		while (fgets(line, sizeof(line), fp)) {
			if (!parse_config_line(line, &option, &value))
				continue;

			if (!strncmp(option, "Inherits", 8)) {
				if (DEBUG_PRINT_INHERITED_THEMES)
					fprintf(stderr, "%s inherits %s\n", child_theme, value);
				sbuf_split(parent_themes, value, ',');
				fclose(fp);
				goto out2;
			}
		}
	}
out2:
	free(filename.buf);
}

/*
 * Some themes have case-insensitive "Inherit" definitions.
 * For example, if an index.theme contains Inherit=adwaita
 * we want this changed to Adwaita (with a capital A).
 */
static void case_sensitize_themes(struct list_head *parent_themes)
{
	struct sbuf *theme, *dir;
	struct dirent *entry;
	DIR *dp;

	list_for_each_entry(dir, &icon_dirs, list) {
		dp = opendir(dir->buf);
		if (!dp)
			continue;

		while ((entry = readdir(dp))) {
			list_for_each_entry(theme, parent_themes, list) {
				if (!strcasecmp(entry->d_name, theme->buf)) {
					sbuf_cpy(theme, entry->d_name);
					break;
				}
			}
		}
		closedir(dp);
	}
}

void icon_find_add_theme(const char *theme)
{
	struct sbuf *t;
	struct list_head parent_themes;
	struct sbuf *tmp, *tmp_tmp;

	/* ignore duplicates */
	list_for_each_entry(t, &theme_list, list)
		if (!strcasecmp(t->buf, theme))
			return;

	sbuf_list_append(&theme_list, theme);

	/* Add parent themes too */
	INIT_LIST_HEAD(&parent_themes);
	get_parent_themes(&parent_themes, theme);
	case_sensitize_themes(&parent_themes);

	list_for_each_entry(tmp, &parent_themes, list)
		icon_find_add_theme(tmp->buf);
	list_for_each_entry_safe(tmp, tmp_tmp, &parent_themes, list) {
		xfree(tmp->buf);
		list_del(&tmp->list);
		xfree(tmp);
	}
}

void icon_find_print_themes(void)
{
	struct sbuf *t;

	fprintf(stderr, "THEMES: ");
	list_for_each_entry(t, &theme_list, list)
		fprintf(stderr, "%s, ", t->buf);
	fprintf(stderr, "\n");
}

static void init_theme_list(void)
{
	INIT_LIST_HEAD(&theme_list);
}

static void init_icon_dirs(void)
{
	struct sbuf *path;

	INIT_LIST_HEAD(&icon_dirs);
	xdgdirs_get_datadirs(&icon_dirs);
	list_for_each_entry(path, &icon_dirs, list)
		sbuf_addstr(path, "/icons");

	if (DEBUG_PRINT_ICON_DIRS)
		list_for_each_entry(path, &icon_dirs, list)
			fprintf(stderr, "%s\n", path->buf);
}

static void init_pixmap_dirs(void)
{
	struct sbuf *path;

	INIT_LIST_HEAD(&pixmap_dirs);
	xdgdirs_get_datadirs(&pixmap_dirs);
	list_for_each_entry(path, &pixmap_dirs, list)
		sbuf_addstr(path, "/pixmaps");
}

/*
 * Simplistic approach to getting icon size.
 * If this isn't going to work for all icon-themes, we will have to
 * start parsing the index.theme file.
 *
 * .../22x22/apps/	(Adwaita)
 * .../22/apps/		(Numix)
 * .../apps/22/		(elementary-xfce)
 */
static int parse_icon_size(const char *fpath)
{
	int size = 0;
	char *s;
	char *p;

	s = strdup(fpath + base_dir_length);

	/*
	 * Remove filename (after last '/') in case it contains digits
	 * (e.g. gtk3-foo.png)
	 */
	p = strrchr(s, '/');
	if (p) {
		*p = '\0';
		size = get_first_num_from_str(s);
	}

	/*
	 * There are a few without an iconsize in the path.
	 * For example:
	 *	- $XDG_DATA_DIRS/icons/<theme>/scalable
	 *	- /usr/share/pixmaps/
	 *	- those without a theme (directly in /usr/share/icons)
	 */
	if (!size)
		size = 65535;

	if (s)
		free(s);

	return size;
}

/* Like process_file, but without global variables (other than requested_icon_size). */
static void process_file(const char *fpath, struct icon_path *item)
{
	int size_of_this_one = 0;

	size_of_this_one = parse_icon_size(fpath);
	if (!size_of_this_one)
		return;

	if (DEBUG_PRINT_ALL_HITS)
		fprintf(stderr, "    %s:%d:%s: %s - %d\n",
			__FILE__, __LINE__, __FUNCTION__, fpath,
			size_of_this_one);

	if (!item->smallest_match && size_of_this_one >= requested_icon_size) {
		item->smallest_match = size_of_this_one;
		sbuf_cpy(&item->path, fpath);
		if (DEBUG_PRINT_ALL_HITS)
			fprintf(stderr, "    %s:%d:%s: Grab\n",
				__FILE__, __LINE__, __FUNCTION__);
	} else if (size_of_this_one < item->smallest_match &&
		   size_of_this_one >= requested_icon_size) {
		item->smallest_match = size_of_this_one;
		sbuf_cpy(&item->path, fpath);
		if (DEBUG_PRINT_ALL_HITS)
			fprintf(stderr, "    %s:%d:%s: Grab\n",
				__FILE__, __LINE__, __FUNCTION__);
	}
}

static int str_has_prefix(const char *str, const char *prefix)
{
	return strncmp(prefix, str, strlen(prefix)) == 0;
}

static void search_dir_for_files(const char *path, struct list_head *files, int depth_limit)
{
	struct dirent *entry;
	DIR *dp;
	struct sbuf s;
	struct icon_path *file;

	if (!depth_limit)
		return;

	if (DEBUG_PRINT_ALL_HITS)
		fprintf(stderr, "  %s:%d:%s: searching %s for all icons\n",
			__FILE__, __LINE__, __FUNCTION__, path);

	sbuf_init(&s);
	dp = opendir(path);
	if (!dp)
		return;

	while ((entry = readdir(dp))) {
		sbuf_cpy(&s, path);
		sbuf_addch(&s, '/');
		sbuf_addstr(&s, entry->d_name);
		if (entry->d_type == DT_DIR) {
			if (entry->d_name[0] != '.')
				search_dir_for_files(s.buf, files, depth_limit > 0 ? depth_limit - 1 : depth_limit);
		} else if (entry->d_type == DT_REG || entry->d_type == DT_LNK) {
			list_for_each_entry(file, files, list) {
				if (!file->found && str_has_prefix(entry->d_name, file->name.buf))
					process_file(s.buf, file);
			}
		}
	}

	closedir(dp);
}

void icon_find_init(void)
{
	if (has_been_inited)
		return;
	init_icon_dirs();
	init_pixmap_dirs();
	init_theme_list();
	has_been_inited = 1;
}

/* Removes the extension. */
static void remove_pngsvgxpm_extensions(struct sbuf *name)
{
	char *ext;

	if (name->len < 4)
		return;

	ext = name->buf + name->len - 4;

	if (!strncmp(ext, ".png", 4) ||
	    !strncmp(ext, ".svg", 4) ||
	    !strncmp(ext, ".xpm", 4)) {
		*ext = '\0';
		name->len -= 4;
	}
}

static int all_icons_found(struct list_head *icons)
{
	int found = 1;
	struct icon_path *icon;

	list_for_each_entry(icon, icons, list) {
		if (!icon->path.len)
			found = 0;
		else
			icon->found = 1;
	}
	return found;
}

void icon_find_all(struct list_head *icons, int size)
{
	struct icon_path *icon;
	struct sbuf path;
	struct sbuf *s, *t;

	if (list_empty(icons))
		return;
	icon_find_init();

	/*
	 * .desktop files should not specify file-extensions, but some do.
	 * Themes use different file-formats, so it's best to remove them.
	 */
	list_for_each_entry(icon, icons, list) {
		remove_pngsvgxpm_extensions(&icon->name);
		sbuf_addch(&icon->name, '.');
	}

	requested_icon_size = size;

	sbuf_init(&path);
	/* Search through $XDG_DATA_DIRS/icons/<theme>/ */
	list_for_each_entry(t, &theme_list, list) {
		list_for_each_entry(s, &icon_dirs, list) {
			sbuf_cpy(&path, s->buf);
			sbuf_addch(&path, '/');
			sbuf_addstr(&path, t->buf);

			base_dir_length = strlen(path.buf);
			if (DEBUG_PRINT_ALL_HITS)
				fprintf(stderr, "%s:%d:%s: searching directory tree %s for all icons\n",
					__FILE__, __LINE__, __FUNCTION__, path.buf);
			search_dir_for_files(path.buf, icons, -1);

			if (all_icons_found(icons))
				goto out;
		}
	}

	/*
	 * A small number of icons are stored in other places:
	 *	- $XDG_DATA_DIRS/pixmaps/  (e.g. xpm icons)
	 *	- $XDG_DATA_DIRS/icons/    (i.e. top level directory)
	 */
	list_for_each_entry(s, &icon_dirs, list)
		search_dir_for_files(s->buf, icons, 1);

	if (all_icons_found(icons))
		goto out;

	list_for_each_entry(s, &pixmap_dirs, list)
		search_dir_for_files(s->buf, icons, 1);

out:
	free(path.buf);
}

void icon_find_cleanup(void)
{
	struct sbuf *theme, *tmp_theme;
	struct sbuf *pmpath, *tmp_pmpath;

	if (!has_been_inited)
		return;
	list_for_each_entry_safe(theme, tmp_theme, &theme_list, list) {
		xfree(theme->buf);
		list_del(&theme->list);
		xfree(theme);
	}
	list_for_each_entry_safe(pmpath, tmp_pmpath, &pixmap_dirs, list) {
		xfree(pmpath->buf);
		list_del(&pmpath->list);
		xfree(pmpath);
	}
	sbuf_list_free(&icon_dirs);
	sbuf_list_free(&pixmap_dirs);
}
