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

#include "util.h"
#include "sbuf.h"
#include "compat.h"

static char obtheme[80];

static char *rcxml_files[] = {
	"~/.config/openbox/rc.xml", "/etc/xdg/openbox/rc.xml", NULL
};

static char *theme_paths[] = {
	"~/.themes/", "/usr/share/themes/", NULL
};

static void process_line(char *line)
{
	/* menu colours */
	if (!strncmp(line, "menu.items.bg.color:", 20)) {
		printf("color_menu_bg = %s 100\n", strstrip(line + 20));
	} else if (!strncmp(line, "menu.items.bg.border.color:", 34)) {
		printf("color_sel_border = %s 100\n", strstrip(line + 34));
		printf("item_border = 1\n");
	} else if (!strncmp(line, "menu.items.text.color:", 22)) {
		printf("color_norm_fg = %s 100\n", strstrip(line + 22));

	/* borders */
	} else if (!strncmp(line, "menu.border.color:", 18)) {
		printf("color_menu_border = %s 100\n", strstrip(line + 18));
	} else if (!strncmp(line, "menu.border.width:", 18)) {
		printf("menu_border = %s\n", strstrip(line + 18));

	/* selected item */
	} else if (!strncmp(line, "menu.items.active.bg.color:", 27)) {
		printf("color_sel_bg = %s 100\n", strstrip(line + 27));
	} else if (!strncmp(line, "menu.items.active.text.color:", 29)) {
		printf("color_sel_fg = %s 100\n", strstrip(line + 29));

	/* separators */
	} else if (!strncmp(line, "menu.title.bg.color:", 20)) {
		printf("color_title_bg = %s 100\n", strstrip(line + 20));
	} else if (!strncmp(line, "menu.title.bg.border.color:", 27)) {
		printf("color_title_border = %s 100\n", strstrip(line + 27));
	} else if (!strncmp(line, "menu.title.text.color:", 22)) {
		printf("color_title_fg = %s 100\n", strstrip(line + 22));
		printf("sep_markup =\n");
	} else if (!strncmp(line, "menu.title.text.justify:", 24)) {
		printf("sep_halign = %s\n", strstrip(line + 24));
	} else if (!strncmp(line, "menu.separator.color:", 21)) {
		printf("color_sep_fg = %s 100\n", strstrip(line + 21));

	/* general */
	} else if (!strncmp(line, "*.text.justify:", 15)) {
		printf("sep_halign = %s\n", strstrip(line + 15));
	} else if (!strncmp(line, "menu.overlap:", 13)) {
		printf("sub_spacing = %d\n", -1 * atoi(line + 13));
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

	sbuf_init(&filename);
	LIBXML_TEST_VERSION

	if (find_rcxml(&filename) < 0)
		die("cannot find rc.xml");
	get_obtheme_from_rcxml(filename.buf);
	info("detected theme '%s' from file '%s'", obtheme, filename.buf);
	if (find_themerc(&filename) < 0)
		die("cannot find openbox theme file 'themerc'");
	info("found '%s'", filename.buf);
	process_themerc(filename.buf);
	xfree(filename.buf);
	return 0;
}
