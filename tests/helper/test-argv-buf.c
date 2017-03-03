#include <stdio.h>
#include <stdlib.h>

#include "argv-buf.h"

void print_argv_buf(struct argv_buf *buf)
{
	int i;
	int first_item = 1;

	for (i = 0; i < buf->argc; i++) {
		if (!first_item)
			printf("@");
		printf("%s", buf->argv[i]);
		first_item = 0;
	}
	printf("\n");
}

#define DELIM "@\t\r\n"
int main(int argc, char **argv)
{
	struct argv_buf buf;
	char line[1024];

	argv_set_delim(&buf, ',');
	argv_init(&buf);

	while (fgets(line, sizeof(line), stdin)) {
		char *cmd, *p1 = NULL;

		cmd = strtok(line, DELIM);
		if (!cmd || *cmd == '#')
			continue;
		p1 = strtok(NULL, DELIM);

		if (!strcmp("init", cmd))
			argv_init(&buf);
		else if (!strcmp("strdup", cmd) && p1)
			argv_strdup(&buf, p1);
		else if (!strcmp("parse", cmd))
			argv_parse(&buf);
		else if (!strcmp("argc", cmd))
			printf("%d\n", buf.argc);
		else if (!strcmp("print", cmd))
			print_argv_buf(&buf);
	}

	argv_free(&buf);
	return 0;
}
