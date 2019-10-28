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
#include "banned.h"

static const char jgmenu_i18n_usage[] =
"Usage: jgmenu_run i18n <translation file>\n"
"       jgmenu_run i18n --init [<translation file>]\n"
"Options:\n"
"    --init                print missing msgid entries for po file\n"
"Notes:\n"
"    - The <translation file> can be a file or directory. If it is the\n"
"      latter, translation files will be searched for in this directory\n"
"      based on the environment variable $LANG, which will be assumed to be\n"
"      in the format ll_CC.UTF8 format where ‘ll’ is an ISO 639 two-letter\n"
"      language code and ‘CC’ is an ISO 3166 two-letter country code.\n"
"      Files named 'll_CC and 'll' will be used in said order\n"
"Example:\n"
" 1) Run `jgmenu_run ob | jgmenu_run i18n --init >sv`\n"
" 2) Translate entries in file 'sv'\n"
" 3) Run `jgmenu_run ob | jgmenu_run i18n sv | jgmenu --simple`\n";

static int arg_init;
static char *i18nfile;

static void usage(void)
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
	char *p, *translation = NULL;

	BUG_ON(!buf);
	if (buf[0] == '\0')
		return;
	if (is_reserved_word(buf))
		return;
	p = strchr(buf, ',');
	if (p)
		*p = '\0';
	if (i18nfile)
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

	if (argc < 2)
		usage();
	i = 1;
	while (i < argc) {
		if (argv[i][0] != '-') {
			i18nfile = argv[i];
			if (argc > i + 1)
				die("<translation file> must be the last argument");
			break;
		} else if (!strcmp(argv[i], "--init")) {
			arg_init = 1;
		} else if (!strcmp(argv[i], "--help")) {
			usage();
		}
		i++;
	}

	if (i18nfile)
		i18nfile = i18n_set_translation_file(i18nfile);

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
