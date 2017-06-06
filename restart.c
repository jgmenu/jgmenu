#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static char **argv;

void restart_init(char **cmd_line_argv)
{
	argv = cmd_line_argv;
}

void restart(void)
{
	fprintf(stderr, "info: restarting jgmenu...\n");
	if (execvp(argv[0], argv) < 0)
		fprintf(stderr, "warn: restart failed\n");
}
