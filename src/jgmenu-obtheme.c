/*
 * jgmenu-obtheme.c
 *
 * Copyright (C) Johan Malm 2019
 *
 * Parses openbox theme and outputs config file key/value pairs for colours
 */

#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "util.h"
#include "sbuf.h"
#include "compat.h"
#include "set.h"

static char obtheme[80];

static char *rcxml_files[] = {
	"~/.config/openbox/rc.xml", "/etc/xdg/openbox/rc.xml", NULL
};

static char *theme_paths[] = {
	"~/.themes/", "/usr/share/themes/", NULL
};

static const char obtheme_usage[] =
"Usage: jgmenu_run obtheme <jgmenurc filename>\n";

void usage(void)
{
	printf("%s", obtheme_usage);
	exit(0);
}

static void vset(const char *key, const char *fmt, ...)
{
	int size = 0;
	char *p = NULL;
	va_list ap;

	va_start(ap, fmt);
	size = vsnprintf(p, size, fmt, ap);
	va_end(ap);
	if (size < 0)
		return;
	size++;
	p = xmalloc(size);
	va_start(ap, fmt);
	size = vsnprintf(p, size, fmt, ap);
	va_end(ap);
	if (size < 0)
		goto out;
	set_set(key, p);
out:
	xfree(p);
}

static void process_line(char *line)
{
	/* menu colours */
	if (!strncmp(line, "menu.items.bg.color:", 20)) {
		vset("color_menu_bg", "%s 100", strstrip(line + 20));
	} else if (!strncmp(line, "menu.items.bg.border.color:", 34)) {
		vset("color_sel_border", "%s 100", strstrip(line + 34));
		set_set("item_border", "1");
	} else if (!strncmp(line, "menu.items.text.color:", 22)) {
		vset("color_norm_fg", "%s 100", strstrip(line + 22));

	/* borders */
	} else if (!strncmp(line, "menu.border.color:", 18)) {
		vset("color_menu_border", "%s 100", strstrip(line + 18));
	} else if (!strncmp(line, "menu.border.width:", 18)) {
		set_set("menu_border", strstrip(line + 18));

	/* selected item */
	} else if (!strncmp(line, "menu.items.active.bg.color:", 27)) {
		vset("color_sel_bg", "%s 100", strstrip(line + 27));
	} else if (!strncmp(line, "menu.items.active.text.color:", 29)) {
		vset("color_sel_fg", "%s 100", strstrip(line + 29));

	/* separators */
	} else if (!strncmp(line, "menu.title.bg.color:", 20)) {
		vset("color_title_bg", "%s 100", strstrip(line + 20));
	} else if (!strncmp(line, "menu.title.bg.border.color:", 27)) {
		vset("color_title_border", "%s 100", strstrip(line + 27));
	} else if (!strncmp(line, "menu.title.text.color:", 22)) {
		vset("color_title_fg", "%s 100", strstrip(line + 22));
		set_set("sep_markup", "");
	} else if (!strncmp(line, "menu.title.text.justify:", 24)) {
		set_set("sep_halign", strstrip(line + 24));
	} else if (!strncmp(line, "menu.separator.color:", 21)) {
		vset("color_sep_fg", "%s 100", strstrip(line + 21));

	/* general */
	} else if (!strncmp(line, "*.text.justify:", 15)) {
		set_set("sep_halign", strstrip(line + 15));
	} else if (!strncmp(line, "menu.overlap:", 13)) {
		vset("sub_spacing", "%d", -1 * atoi(line + 13));
	}
}

static void process_themerc(const char *filename)
{
	FILE *fp;
	char line[4096];
	char *p;

	fp = fopen(filename, "r");
	if (!fp) {
		warn("could not open file %s", filename);
		return;
	}
	while (fgets(line, sizeof(line), fp)) {
		if (line[0] == '\0')
			continue;
		p = strrchr(line, '\n');
		if (!p) {
			warn("line too long");
			continue;
		}
		*p = '\0';
		process_line((char *)strstrip(line));
	}
	fclose(fp);
}

static void xml_tree_walk(xmlNode *node)
{
	xmlNode *n;
	static int insidetheme;

	for (n = node; n && n->name; n = n->next) {
		if (!strcasecmp((char *)n->name, "theme")) {
			insidetheme = 1;
			xml_tree_walk(n->children);
			return;
		}
		if (!strcasecmp((char *)n->name, "name")) {
			if (!insidetheme)
				continue;
			strlcpy(obtheme, (const char *)xmlNodeGetContent(n),
				sizeof(obtheme));
			return;
		}
		xml_tree_walk(n->children);
	}
}

static void get_obtheme_from_rcxml(const char *filename)
{
	xmlDoc *d;

	d = xmlReadFile(filename, NULL, 0);
	if (!d)
		die("error reading file '%s'\n", filename);
	xml_tree_walk(xmlDocGetRootElement(d));
	xmlFreeDoc(d);
	xmlCleanupParser();
}

static int find_themerc(struct sbuf *filename)
{
	struct stat sb;
	int i;

	for (i = 0; theme_paths[i]; i++) {
		sbuf_cpy(filename, theme_paths[i]);
		sbuf_expand_tilde(filename);
		sbuf_addstr(filename, obtheme);
		sbuf_addstr(filename, "/openbox-3/themerc");
		if (!stat(filename->buf, &sb))
			return 0;
	}
	return -1;
}

static int find_rcxml(struct sbuf *filename)
{
	struct stat sb;
	int i;

	for (i = 0; rcxml_files[i]; i++) {
		sbuf_cpy(filename, rcxml_files[i]);
		sbuf_expand_tilde(filename);
		if (!stat(filename->buf, &sb))
			return 0;
	}
	return -1;
}

int main(int argc, char **argv)
{
	struct sbuf filename;

	if (argc != 2)
		usage();
	sbuf_init(&filename);
	LIBXML_TEST_VERSION

	if (find_rcxml(&filename) < 0)
		die("cannot find rc.xml");
	get_obtheme_from_rcxml(filename.buf);
	info("detected theme '%s' from file '%s'", obtheme, filename.buf);
	if (find_themerc(&filename) < 0)
		die("cannot find openbox theme file 'themerc'");
	info("found '%s'", filename.buf);
	set_read(argv[1]);
	process_themerc(filename.buf);
	xfree(filename.buf);
	set_write(argv[1]);
	return 0;
}
