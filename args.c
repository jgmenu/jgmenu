#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "config.h"
#include "util.h"

static char *checkout;
static char *csv_file;
static char *csv_cmd;
static int simple;
static int die_when_loaded;
static int pmenu;
static int xdg;
static int lx;
static int ob;

void args_parse(int argc, char **argv)
{
	int i;

	if (argc < 2)
		return;
	if (!strcmp(argv[1], "pmenu")) {
		pmenu = 1;
	} else if (!strcmp(argv[1], "xdg")) {
		xdg = 1;
	} else if (!strcmp(argv[1], "lx")) {
		lx = 1;
	} else if (!strcmp(argv[1], "ob")) {
		ob = 1;
	}

	for (i = 1; i < argc; i++) {
		if (!strncmp(argv[i], "--no-spawn", 10)) {
			config.spawn = 0;
		} else if (!strncmp(argv[i], "--checkout=", 11)) {
			checkout = argv[i] + 11;
		} else if (!strncmp(argv[i], "--icon-size=", 12)) {
			xatoi(&config.icon_size, argv[i] + 12, XATOI_NONNEG,
			      "config.icon_size");
		} else if (!strncmp(argv[i], "--die-when-loaded", 17)) {
			die_when_loaded = 1;
		} else if (!strncmp(argv[i], "--at-pointer", 12)) {
			config.at_pointer = 1;
		} else if (!strncmp(argv[i], "--hide-on-startup", 17)) {
			config.hide_on_startup = 1;
		} else if (!strncmp(argv[i], "--simple", 8)) {
			simple = 1;
		} else if (!strncmp(argv[i], "--csv-file=", 11)) {
			csv_file = argv[i] + 11;
		} else if (!strncmp(argv[i], "--csv-cmd=", 10)) {
			csv_cmd = argv[i] + 10;
		}
	}
}

char *args_checkout(void)
{
	return checkout;
}

char *args_csv_file(void)
{
	return csv_file;
}

char *args_csv_cmd(void)
{
	return csv_cmd;
}

int args_simple(void)
{
	return simple;
}

int args_die_when_loaded(void)
{
	return die_when_loaded;
}

int args_pmenu(void)
{
	return pmenu;
}

int args_xdg(void)
{
	return xdg;
}

int args_lx(void)
{
	return lx;
}

int args_ob(void)
{
	return ob;
}
