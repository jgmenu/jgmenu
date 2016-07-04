/*
 * icon-find.c: finds icon by theme and size loosely iaw XDG spec
 */

#include "icon-find.h"
#include "list.h"
#include <ftw.h>

#define COUNT_OF(x) (sizeof (x) / sizeof (x)[0])

static int DEBUG = 0;
static int DEBUG_MORE = 0;

static char requested_icon_name[1024];
static int  requested_icon_size;

struct String most_suitable_icon;

static int base_dir_length;
static int smallest_match = 0;

/* FIXME: Move this to xdgdirs.c and make it more sophisticated */
static char *xdg_icon_paths[] = {
	"/usr/share/icons", "/usr/local/share/icons"
};

/* FIXME: Move this to xdgdirs.c too */
struct list_head theme_list;

static void init_theme_list(const char *default_theme)
{
	INIT_LIST_HEAD(&theme_list);
	sbuf_list_append(&theme_list, default_theme);
	sbuf_list_append(&theme_list, "hicolor");

	/* TODO: Search for and add any inherted themes */
}


/*
 * FIXME: This currently ignores /scalable/
 */
int parse_icon_size(const char *fpath)
{
	int size;
	char *s;
	char *p;

	s = strdup(fpath + base_dir_length + 1); /* e.g. 22x22/... */

	p = strchr(s, 'x');
	if (p)
		*p = '\0';

	/*
	 * Some icons themes (e.g. Numix) use "22" instead of "22x22"
	 */
	p = strchr(s, '/');
	if (p)
		*p = '\0';

	size = atoi(s);
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
				most_suitable_icon.buf[0] = 0;
				most_suitable_icon.len = 0;
				sbuf_addstr(&most_suitable_icon, fpath);
				if (DEBUG_MORE)
					printf(" - Grab");
			}
			if (DEBUG_MORE)
				printf("\n");
		}


	return 0;
}

void icon_find(struct String *name, const char *theme, int size)
{
	struct String path;
	struct String *t;
	int i;

	init_theme_list(theme);

	sbuf_init(&most_suitable_icon);

	strcpy(requested_icon_name, name->buf);
	requested_icon_size = size;

	sbuf_init(&path);

	for (i = 0; i < COUNT_OF(xdg_icon_paths); i++) {
		list_for_each_entry(t, &theme_list, list) {
			sbuf_cpy(&most_suitable_icon, "");
			sbuf_cpy(&path, xdg_icon_paths[i]);
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
