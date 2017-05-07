#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "cache.h"
#include "sbuf.h"

int cache_exists(void)
{
	struct sbuf f;
	struct stat sb;
	int exists;

	sbuf_init(&f);
	sbuf_addstr(&f, getenv("HOME"));
	sbuf_addstr(&f, "/.local/share/icons/jgmenu-cache/index.theme");
	exists = stat(f.buf, &sb) == 0;
	if (!exists)
		fprintf(stderr, "warning: jgmenu icon-cache has not been created\n");
	return exists;
}

