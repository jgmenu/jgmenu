#include <stdio.h>
#include <stdlib.h>

#include "argv-buf.h"
#include "xdgdirs.h"

static void print_dirs(const char *xdg_config_dirs)
{
	LIST_HEAD(config_dirs);
	struct sbuf *s;

	/*
	 * The test harness messes with $HOME, so had better unset for this
	 * Any unset $HOME, $XDG_CONFIG_* simply gets left of the list
	 */
	unsetenv("HOME");
	unsetenv("XDG_CONFIG_HOME");

	setenv("XDG_CONFIG_DIRS", xdg_config_dirs, 1);
	xdgdirs_get_configdirs(&config_dirs);
	list_for_each_entry(s, &config_dirs, list)
		printf("%s@", s->buf);
	sbuf_list_free(&config_dirs);
	printf("\n");
}

#define DELIM " \t\r\n"
int main(int argc, char **argv)
{
	char line[1024];

	while (fgets(line, sizeof(line), stdin)) {
		char *cmd, *p1 = NULL;

		cmd = strtok(line, DELIM);
		if (!cmd || *cmd == '#')
			continue;
		p1 = strtok(NULL, DELIM);

		if (!strcmp("dirs", cmd) && p1)
			print_dirs(p1);
	}
	return 0;
}
