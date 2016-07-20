/*
 * icon-find.c: Finds icon by name, theme and size
 *
 * Loosely iaw XDG spec
 */

#include <ftw.h>

#include "icon-find.h"
#include "xdgdirs.h"
#include "list.h"
#include "util.h"

static int DEBUG = 0;
static int DEBUG_MORE = 0;
static int DEBUG_THEME = 0;

/*
 * e.g. "/usr/share/icons", "/usr/loca/share/icons"
 * see xdgdirs.c for more details
 */
static struct list_head icon_dirs;

/* e.g. "Adwaita", "default", "hicolor" */
static struct list_head theme_list;

static int has_been_inited = 0;

/* Variables used in the "find" algorithm */
static char requested_icon_name[1024];
static int  requested_icon_size;
static struct String most_suitable_icon;
static int base_dir_length;
static int smallest_match = 0;


static void get_parent_themes(struct list_head *parent_themes, const char *child_theme)
{
	FILE *fp;
	char line[1024];
	char *option, *value;
	struct String *s;
	struct String filename;

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
				if (DEBUG_THEME)
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


void icon_find_add_theme(const char *theme)
{
	struct String *t;
	struct list_head parent_themes;
	struct String *tmp;

	/* ignore duplicates */
	list_for_each_entry(t, &theme_list, list)
		if (!strcasecmp(t->buf, theme))
			return;

	sbuf_list_append(&theme_list, theme);

	/* Add parent themes too */
	INIT_LIST_HEAD(&parent_themes);
	get_parent_themes(&parent_themes, theme);

	list_for_each_entry(tmp, &parent_themes, list)
		icon_find_add_theme(tmp->buf);
}


void icon_find_print_themes(void)
{
	struct String *t;

	printf("THEMES:\n");
	list_for_each_entry(t, &theme_list, list)
		printf("%s\n", t->buf);
}

static void init_theme_list(void)
{
	INIT_LIST_HEAD(&theme_list);
}

static void init_icon_dirs(void)
{
	struct String *path;

	INIT_LIST_HEAD(&icon_dirs);

	xdgdirs_get_basedirs(&icon_dirs);

	list_for_each_entry(path, &icon_dirs, list)
		sbuf_addstr(path, "/icons");
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
	int size;
	char *s;

	s = strdup(fpath + base_dir_length + 1);

	if (strstr(s, "scalable"))
		return 65535;

	size = get_first_num_from_str(s);

	if (s)
		free(s);

	return size;
}

static int process_file(const char *fpath, const struct stat *sb, int typeflag)
{
	int size_of_this_one = 0;

	if (typeflag == FTW_F)
		if (strstr(fpath, requested_icon_name)) {
			size_of_this_one = parse_icon_size(fpath);
			if (!size_of_this_one)
				return 0;
			if (DEBUG_MORE)
				printf("%s -- %d", fpath, size_of_this_one);

			if (!smallest_match) {
				smallest_match = size_of_this_one;
				sbuf_addstr(&most_suitable_icon, fpath);
				if (DEBUG_MORE)
					printf(" - Grab");
			} else if (size_of_this_one < smallest_match &&
				   size_of_this_one >= requested_icon_size) {
				smallest_match = size_of_this_one;
				sbuf_cpy(&most_suitable_icon, fpath);
				if (DEBUG_MORE)
					printf(" - Grab");
			}
			if (DEBUG_MORE)
				printf("\n");
		}


	return 0;
}

void icon_find_init(void)
{
	init_icon_dirs();
	init_theme_list();
	sbuf_init(&most_suitable_icon);
	has_been_inited = 1;
}

void icon_find(struct String *name, int size)
{
	struct String path;
	struct String *s, *t;
	char *p;

	if (!has_been_inited) {
		fprintf(stderr, "warn: icon_find() was called before icon_find_init()");
		icon_find_init();
	}

	/*
	 * Don't search for icon if path (absolute or relative)
	 * has been specified.
	 */
	p = strchr(name->buf, '/');
	if (p)
		return;

	/*
	 * Ensure we only find exact matches. E.g. if we search for "folder"
	 * we don't want "folder-documents.png" returned.
	 */
	sbuf_prepend(name, "/");
	if (!strstr(name->buf, ".png") && !strstr(name->buf, ".svg"))
		sbuf_addch(name, '.');

	sbuf_init(&path);
	strcpy(requested_icon_name, name->buf);
	requested_icon_size = size;

	list_for_each_entry(s, &icon_dirs, list) {
		list_for_each_entry(t, &theme_list, list) {
			sbuf_cpy(&most_suitable_icon, "");
			sbuf_cpy(&path, s->buf);
			sbuf_addch(&path, '/');
			sbuf_addstr(&path, t->buf);

			base_dir_length = strlen(path.buf);
			smallest_match = 0;

			ftw(path.buf, process_file, 32);

			if (DEBUG) {
				if (most_suitable_icon.len)
					printf("OUTPUT: %s\n", most_suitable_icon.buf);
				else
					printf("OUTPUT: (NULL)\n");
			}

			if (most_suitable_icon.len)
				goto out;
		}
	}

out:
	sbuf_cpy(name, most_suitable_icon.buf);
	free(path.buf);
	sbuf_cpy(&most_suitable_icon, "");
}
