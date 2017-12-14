#include <stdio.h>

#include "icon-find.h"
#include "sbuf.h"
#include "util.h"

static char *theme;
static char *icon_name;
static int icon_size = 22;

static void usage(void)
{
	printf("Usage: jgmenu-icon-find [<options>] <icon-name>\n"
	       "    --theme=<theme>       specify icon-theme\n"
	       "    --icon-size=<size>    specify icon-size\n");
	exit(1);
}

int main(int argc, char **argv)
{
	int i;
	struct sbuf s;

	if (argc < 2)
		usage();

	for (i = 1; i < argc; i++) {
		if (argv[i][0] != '-')
			icon_name = argv[i];
		if (!strncmp(argv[i], "--icon-size=", 12))
			xatoi(&icon_size, argv[i] + 12, XATOI_NONNEG,
			      "config.icon_size");
		if (!strncmp(argv[i], "--theme=", 8))
			theme = argv[i] + 8;
		if (!strncmp(argv[i], "--help", 6))
			usage();
	}

	if (!icon_name) {
		fprintf(stderr, "fatal: no icon-name specified\n");
		exit(1);
	}

	sbuf_init(&s);
	icon_find_init();
	if (theme)
		icon_find_add_theme(theme);
	else
		icon_find_add_theme("default");
	icon_find_add_theme("hicolor");

	sbuf_cpy(&s, icon_name);
	icon_find(&s, icon_size);

	if (s.len)
		printf("%s\n", s.buf);
	else
		fprintf(stderr, "warning: could not find icon '%s'\n", icon_name);

	return 0;
}
