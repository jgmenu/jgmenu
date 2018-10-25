/*
 * jgmenu-i18n.c
 *
 * Copyright (C) Johan Malm 2018
 *
 * Provide internationalisation for jgmenu menu data
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "util.h"
#include "sbuf.h"
#include "i18n.h"

static const char jgmenu_i18n_usage[] =
"Usage: jgmenu_run i18n <translation file>\n"
"Example:\n"
"jgmenu_run ob | jgmenu_run i18n sv.po | jgmenu --simple\n";

void usage(void)
{
	printf("%s", jgmenu_i18n_usage);
	exit(0);
}

static void translate_and_print(char *buf)
{
	struct sbuf s;
	char *remainder;

	BUG_ON(!buf);
	sbuf_init(&s);
	sbuf_cpy(&s, buf);
	remainder = i18n_translate(&s);	
	printf("%s", s.buf);
	if (remainder)
		printf(",%s", remainder);
	printf("\n");
	xfree(s.buf);
}

int main(int argc, char **argv)
{
	char buf[BUFSIZ], *p;
	int i;
	char *filename = NULL;

	if (argc < 1)
		usage();
	i = 1;
	while (i < argc) {
		if (argv[i][0] != '-') {
			filename = argv[i];
			if (argc > i + 1)
				die("<file> must be the last argument");
			break;
		} else if (!strcmp(argv[i], "--help")) {
			usage();
		}
		i++;
	}

	i18n_open(filename);

	for (i = 0; fgets(buf, BUFSIZ, stdin); i++) {
		buf[BUFSIZ - 1] = '\0';
		if (strlen(buf) == BUFSIZ - 1)
			die("line %d is too long", i);
		p = strrchr(buf, '\n');
		if (p)
			*p = '\0';
		else
			die("line %d not correctly terminated", i);
		translate_and_print(buf);
	}
	return 0;
}
