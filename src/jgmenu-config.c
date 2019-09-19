/*
 * jgmenu-config.c
 *
 * Copyright (C) Johan Malm 2019
 *
 * Create or amend config file.
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdarg.h>

#include "util.h"
#include "sbuf.h"
#include "compat.h"
#include "set.h"
#include "spawn.h"

struct option {
	const char *key;
	const char *value;
};

static struct option options[] = {
	{ "verbosity", "0" },
	{ "stay_alive", "1" },
	{ "hide_on_startup", "0" },
	{ "csv_cmd", "pmenu" },
	{ "tint2_look", "0" },
	{ "position_mode", "fixed" },
	{ "edge_snap_x", "30" },
	{ "terminal_exec", "x-terminal-emulator" },
	{ "terminal_args", "-e" },
	{ "monitor", "0" },
	{ "hover_delay", "100" },
	{ "hide_back_items", "1" },
	{ "columns", "1" },
	{ "tabs", "120" },
	{ "menu_margin_x", "0" },
	{ "menu_margin_y", "0" },
	{ "menu_width", "200" },
	{ "menu_height_min", "0" },
	{ "menu_height_max", "0" },
	{ "menu_height_mode", "static" },
	{ "menu_padding_top", "5" },
	{ "menu_padding_right", "5" },
	{ "menu_padding_bottom", "5" },
	{ "menu_padding_left", "5" },
	{ "menu_radius", "1" },
	{ "menu_border", "0" },
	{ "menu_halign", "left" },
	{ "menu_valign", "bottom" },
	{ "sub_spacing", "1" },
	{ "sub_padding_top", "auto" },
	{ "sub_padding_right", "auto" },
	{ "sub_padding_bottom", "auto" },
	{ "sub_padding_left", "auto" },
	{ "sub_hover_action", "1" },
	{ "item_margin_x", "3" },
	{ "item_margin_y", "3" },
	{ "item_height", "25" },
	{ "item_padding_x", "4" },
	{ "item_radius", "1" },
	{ "item_border", "0" },
	{ "item_halign", "left" },
	{ "sep_height", "5" },
	{ "sep_halign", "left" },
	{ "sep_markup", "" },
	{ "font", "" },
	{ "font_fallback", "xtg" },
	{ "icon_size", "22" },
	{ "icon_text_spacing", "10" },
	{ "icon_theme", "" },
	{ "icon_theme_fallback", "xtg" },
	{ "arrow_string", "▸" },
	{ "arrow_width", "15" },
	{ "color_menu_bg", "#000000 100" },
	{ "color_menu_border", "#eeeeee 8" },
	{ "color_norm_bg", "#000000 00" },
	{ "color_norm_fg", "#eeeeee 100" },
	{ "color_sel_bg", "#ffffff 20" },
	{ "color_sel_fg", "#eeeeee 100" },
	{ "color_sel_border", "#eeeeee 8" },
	{ "color_sep_fg", "#ffffff 20" },
	{ "color_scroll_ind", "#eeeeee 40" },
	{ "color_title_fg", "#eeeeee 50" },
	{ "color_title_bg", "#000000 0" },
	{ "color_title_border", "#000000 0" },
	{ "csv_name_format", "%n (%g)" },
	{ "csv_single_window", "0" },
	{ "csv_no_dirs", "0" },
	{ "csv_i18n", "" },
	{ NULL, NULL },
};

static const char config_usage[] =
"Usage: jgmenu_run config [options]\n"
"Creates or amend config file\n"
"Options:\n"
"    -a <file>      amend config file with missing items\n"
"    -c             write default config file to stdout\n"
"    -k <key>       specify key (needed for -s)\n"
"    -s <file>      set key/value pair (add if does not exist)\n"
"    -v <value>     specify value (needed for -s)\n"
"    -h             display this message\n";

static void usage(void)
{
	printf("%s", config_usage);
	exit(0);
}

static void create(void)
{
	struct option *o;

	for (o = options; o->key; o++)
		printf("# %s = %s\n", o->key, o->value);
}

static void check_file(struct sbuf *f, const char *filename)
{
	struct stat sb;

	if (!filename)
		die("no filename specified");
	sbuf_cpy(f, filename);
	sbuf_expand_tilde(f);

	/* if file does not exist, we create it */
	if (stat(f->buf, &sb)) {
		static const char * const command[] = { "jgmenu_run", "init",
							NULL };

		spawn_sync(command);
	}
}

static void amend(const char *filename)
{
	struct sbuf f;
	struct option *o;

	sbuf_init(&f);
	check_file(&f, filename);
	set_read(f.buf);
	for (o = options; o->key; o++) {
		if (!set_key_exists(o->key)) {
			info("add '%s = %s'", o->key, o->value);
			set_set(o->key, o->value, 1);
		}
	}
	set_write(f.buf);
	xfree(f.buf);
}

static void set_key_value_pair(const char *filename, const char *key,
			       const char *value)
{
	struct sbuf f;

	sbuf_init(&f);
	check_file(&f, filename);
	set_read(f.buf);

	/*
	 * Aborting at this point if the key/value pair is already set
	 * correctly, avoid updating the config file and thereby indirectly
	 * avoids restarting jgmenu too.
	 */
	if (set_is_already_set_correctly(key, value))
		goto out;
	set_set(key, value, 0);
	set_write(f.buf);
out:
	xfree(f.buf);
}

int main(int argc, char **argv)
{
	enum action { AMEND, CREATE, SET, UNKNOWN };
	enum action action = UNKNOWN;
	char *key = NULL, *value = NULL, *filename = NULL;
	int i;

	if (argc < 2)
		usage();
	for (i = 1; i < argc; i++) {
		const char *arg = argv[i];

		if (*arg == '-') {
			switch (arg[1]) {
			case 'a':
				action = AMEND;
				filename = argv[i + 1];
				++i;
				continue;
			case 'c':
				action = CREATE;
				continue;
			case 'h':
				usage();
				continue;
			case 'k':
				key = argv[i + 1];
				++i;
				continue;
			case 's':
				action = SET;
				filename = argv[i + 1];
				++i;
				continue;
			case 'v':
				value = argv[i + 1];
				++i;
				continue;
			default:
				die("unknown argument '%s'", arg);
			}
		}
		die("unknown argument '%s'", arg);
	}

	switch (action) {
	case AMEND:
		amend(filename);
		break;
	case CREATE:
		create();
		break;
	case SET:
		if (!key || !value)
			die("'key' and 'value' need to be specified");
		set_key_value_pair(filename, key, value);
		break;
	default:
		fprintf(stderr, "info: use -a, -c or -s\n");
		break;
	}

	return 0;
}
