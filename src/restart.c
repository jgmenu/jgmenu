#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "restart.h"
#include "banned.h"

#define JGMENU_MAX_ARGS (32)

static char *args[JGMENU_MAX_ARGS];

void restart_init(int argc, char **argv)
{
	int i, j;

	for (i = 0, j = 0; i < argc && i < JGMENU_MAX_ARGS && argv[i]; i++) {
		if (!strcmp(argv[i], "--hide-on-startup"))
			continue;
		args[j++] = argv[i];
	}
	args[j] = NULL;
}

void restart(void)
{
	fprintf(stderr, "info: restarting jgmenu...\n");
	if (execvp(args[0], args) < 0)
		fprintf(stderr, "warn: restart failed\n");
}
