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
"       jgmenu_run i18n --init\n\n"
"Options:\n"
"    --init                print missing msgid entries for po file\n\n"
"Example:\n"
" 1) jgmenu_run ob | jgmenu_run i18n --init >sv\n"
" 2) Translate entries in po file.\n"
" 3) jgmenu_run ob | jgmenu_run i18n sv | jgmenu --simple\n";

static int arg_init;

void usage(void)
{
	printf("%s", jgmenu_i18n_usage);
	exit(0);
}

static int is_reserved_word(const char *buf)
{
	if (buf[0] == '^')
		return 1;
	return 0;
}

static void create_msgid_if_missing(char *buf)
{
	char *p, *translation;

	BUG_ON(!buf);
	if (buf[0] == '\0')
		return;
	if (is_reserved_word(buf))
		return;
	p = strchr(buf, ',');
	if (p)
		*p = '\0';
	translation = i18n_translate(buf);
	if (translation)
		return;
	printf("msgid \"%s\"\nmsgstr \"\"\n\n", buf);
}

static void translate_and_print(char *buf)
{
	struct sbuf s;

	BUG_ON(!buf);
	sbuf_init(&s);
	sbuf_cpy(&s, buf);
	i18n_translate_first_field(&s);
	printf("%s\n", s.buf);
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
		} else if (!strcmp(argv[i], "--init")) {
			arg_init = 1;
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
		if (arg_init)
			create_msgid_if_missing(buf);
		else
			translate_and_print(buf);
	}
	i18n_cleanup();
	return 0;
}
