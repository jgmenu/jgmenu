#include <stdio.h>

#include "xsettings-helper.h"
#include "sbuf.h"
#include "util.h"

static const char jgmenu_xsettings[] =
"Usage: jgmenu-xsettings (--icon-theme | --all)\n";

static void usage(void)
{
	printf("%s", jgmenu_xsettings);
	exit(1);
}

int main(int argc, char **argv)
{
	int i;
	struct sbuf s;
	static char *key;
	static int ret;

	if (argc < 2)
		usage();

	for (i = 1; i < argc; i++) {
		if (!strncmp(argv[i], "--icon-theme", 12))
			key = strdup("Net/IconThemeName");
		if (!strncmp(argv[i], "--all", 5))
			key = strdup("*");
	}

	if (!key)
		usage();

	sbuf_init(&s);

	ret = xsettings_get(&s, key);

	if (s.len)
		printf("%s\n", s.buf);

	return ret;
}
