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

#define DEBUG_PRINT_FINAL_SELECTION 0
#define DEBUG_PRINT_ALL_HITS 1		/* regardless of size */
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
static char requested_icon_name[1024];
static int  requested_icon_size;
static struct sbuf most_suitable_icon;
static int base_dir_length;
static int smallest_match;

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
					printf("%s inherits %s\n", child_theme, value);
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
	struct sbuf *tmp;

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
}

void icon_find_print_themes(void)
{
	struct sbuf *t;

	printf("THEMES: ");
	list_for_each_entry(t, &theme_list, list)
		printf("%s, ", t->buf);
	printf("\n");
}

static void init_theme_list(void)
{
	INIT_LIST_HEAD(&theme_list);
}

static void init_icon_dirs(void)
{
	struct sbuf *path;

	INIT_LIST_HEAD(&icon_dirs);
	xdgdirs_get_basedirs(&icon_dirs);
	list_for_each_entry(path, &icon_dirs, list)
		sbuf_addstr(path, "/icons");

	if (DEBUG_PRINT_ICON_DIRS)
		list_for_each_entry(path, &icon_dirs, list)
			printf("%s\n", path->buf);
}

static void init_pixmap_dirs(void)
{
	struct sbuf *path;

	INIT_LIST_HEAD(&pixmap_dirs);
	xdgdirs_get_basedirs(&pixmap_dirs);
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

static void process_file(const char *fpath)
{
	int size_of_this_one = 0;

	size_of_this_one = parse_icon_size(fpath);
	if (!size_of_this_one)
		return;

	if (DEBUG_PRINT_ALL_HITS)
		printf("%s - %d", fpath, size_of_this_one);

	if (!smallest_match &&
	    size_of_this_one >= requested_icon_size) {
		smallest_match = size_of_this_one;
		sbuf_addstr(&most_suitable_icon, fpath);
		if (DEBUG_PRINT_ALL_HITS)
			printf(" - Grab");
	} else if (size_of_this_one < smallest_match &&
		   size_of_this_one >= requested_icon_size) {
		smallest_match = size_of_this_one;
		sbuf_cpy(&most_suitable_icon, fpath);
		if (DEBUG_PRINT_ALL_HITS)
			printf(" - Grab");
	}
	if (DEBUG_PRINT_ALL_HITS)
		printf("\n");
}

/*
 * Alternative to ftw, does not call fstat on all files in the directory tree
 */
int search_dir_for_file(const char *path, const char *file)
{
	struct dirent *entry;
	DIR *dp;
	struct sbuf s;

	if (DEBUG_PRINT_ALL_HITS)
		fprintf(stderr, "  %s:%d:%s: searching %s for %s\n",
				__FILE__, __LINE__, __FUNCTION__, path, file);

	sbuf_init(&s);
	dp = opendir(path);
	if (!dp)
		return 1;

	while ((entry = readdir(dp))) {
		sbuf_cpy(&s, path);
		sbuf_addch(&s, '/');
		sbuf_addstr(&s, entry->d_name);
		if (entry->d_type == DT_DIR) {
			if (entry->d_name[0] != '.') {
				search_dir_for_file(s.buf, file);
			}
		} else if (entry->d_type == DT_REG || entry->d_type == DT_LNK) {
			if (strstr(s.buf, file)) {
				process_file(s.buf);
			}
		}
	}

	closedir(dp);
	return 0;
}

void icon_find_init(void)
{
	init_icon_dirs();
	init_pixmap_dirs();
	init_theme_list();
	sbuf_init(&most_suitable_icon);
	has_been_inited = 1;
}

void remove_pngsvgxpm_extensions(struct sbuf *name)
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

void icon_find(struct sbuf *name, int size)
{
	struct sbuf path;
	struct sbuf *s, *t;
	char *p;

	if (!has_been_inited)
		icon_find_init();

	/*
	 * Don't search for icon if path (absolute or relative)
	 * has been specified.
	 */
	p = strchr(name->buf, '/');
	if (p)
		return;

	/*
	 * .desktop files should not specify file-extensions, but some do.
	 * Themes use different file-formats, so it's best to remove them.
	 */
	remove_pngsvgxpm_extensions(name);

	/*
	 * Ensure we only find exact matches. E.g. if we search for "folder"
	 * we don't want "folder-documents.png" returned.
	 */
	sbuf_prepend(name, "/");
	sbuf_addch(name, '.');

	sbuf_init(&path);
	strcpy(requested_icon_name, name->buf);
	requested_icon_size = size;

	/* Search through $XDG_DATA_DIRS/icons/<theme>/ */
	list_for_each_entry(t, &theme_list, list) {
		list_for_each_entry(s, &icon_dirs, list) {
			sbuf_cpy(&most_suitable_icon, "");
			sbuf_cpy(&path, s->buf);
			sbuf_addch(&path, '/');
			sbuf_addstr(&path, t->buf);

			base_dir_length = strlen(path.buf);
			smallest_match = 0;

			if (DEBUG_PRINT_ALL_HITS)
				fprintf(stderr, "%s:%d:%s: searching directory tree %s for %s\n",
						__FILE__, __LINE__, __FUNCTION__, path.buf, name->buf);
			search_dir_for_file(path.buf, name->buf);

			if (DEBUG_PRINT_FINAL_SELECTION && most_suitable_icon.len)
				printf("OUTPUT: %s\n", most_suitable_icon.buf);

			if (most_suitable_icon.len)
				goto out;
		}
	}

	/*
	 * A small number of icons are stored in other places:
	 *	- $XDG_DATA_DIRS/pixmaps/  (e.g. xpm icons)
	 *	- $XDG_DATA_DIRS/icons/    (i.e. top level directory)
	 *
	 * For this search we remove the prepended '/'
	 */
	sbuf_shift_left(name, 1);
	strcpy(requested_icon_name, name->buf);

	list_for_each_entry(s, &icon_dirs, list)
		search_dir_for_file(s->buf, requested_icon_name);

	if (most_suitable_icon.len)
		goto out;

	list_for_each_entry(s, &pixmap_dirs, list)
		search_dir_for_file(s->buf, requested_icon_name);

out:
	sbuf_cpy(name, most_suitable_icon.buf);
	free(path.buf);
	sbuf_cpy(&most_suitable_icon, "");
}
