#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "util.h"
#include "config.h"

struct config config;

void config_set_defaults(void)
{
	config.spawn		   = 1;	/* not in jgmenurc */
	config.stay_alive	   = 1;
	config.hide_on_startup	   = 0;
	/* jgmenurc has a csv_cmd variable here */
	config.tint2_look	   = 1;
	config.tint2_button	   = 1;
	config.tint2_rules	   = 1;
	config.at_pointer	   = 0;
	config.terminal_exec	   = xstrdup("x-terminal-emulator");
	config.terminal_args	   = xstrdup("-e");

	config.menu_margin_x	   = 0;
	config.menu_margin_y	   = 31;
	config.menu_width	   = 200;
	config.menu_padding_top	   = 10;
	config.menu_padding_right  = 10;
	config.menu_padding_bottom = 10;
	config.menu_padding_left   = 10;
	config.menu_radius	   = 1;
	config.menu_border	   = 0;
	config.menu_halign	   = LEFT;
	config.menu_valign	   = BOTTOM;

	config.item_margin_x	   = 3;
	config.item_margin_y	   = 3;
	config.item_height	   = 25;
	config.item_padding_x	   = 4;
	config.item_radius	   = 1;
	config.item_border	   = 0;
	config.item_halign	   = LEFT;
	config.sep_height	   = 5;

	config.font		   = NULL; /* Leave as NULL (see font.c) */
	config.font_fallback	   = xstrdup("xtg");
	config.icon_size	   = 22;
	config.icon_text_spacing   = 10;
	config.icon_theme	   = NULL; /* Leave as NULL (see theme.c) */
	config.icon_theme_fallback = xstrdup("xtg");

	config.arrow_string	   = xstrdup("▸");
	config.arrow_width	   = 15;

	parse_hexstr("#000000 70", config.color_menu_bg);
	parse_hexstr("#eeeeee 20", config.color_menu_fg);
	parse_hexstr("#eeeeee 8", config.color_menu_border);
	parse_hexstr("#000000 00", config.color_norm_bg);
	parse_hexstr("#eeeeee 100", config.color_norm_fg);
	parse_hexstr("#ffffff 20", config.color_sel_bg);
	parse_hexstr("#eeeeee 100", config.color_sel_fg);
	parse_hexstr("#eeeeee 8", config.color_sel_border);
	parse_hexstr("#ffffff 20", config.color_sep_fg);
}

static void process_line(char *line)
{
	char *option, *value;

	if (!parse_config_line(line, &option, &value))
		return;

	if (!strncmp(option, "stay_alive", 10)) {
		xatoi(&config.stay_alive, value, XATOI_NONNEG, "config.stay_alive");
	} else if (!strncmp(option, "hide_on_startup", 15)) {
		xatoi(&config.hide_on_startup, value, XATOI_NONNEG, "config.hide_on_startup");
	} else if (!strncmp(option, "tint2_look", 10)) {
		xatoi(&config.tint2_look, value, XATOI_NONNEG, "config.tint2_look");
	} else if (!strncmp(option, "tint2_button", 12)) {
		xatoi(&config.tint2_button, value, XATOI_NONNEG, "config.tint2_button");
	} else if (!strncmp(option, "tint2_rules", 11)) {
		xatoi(&config.tint2_rules, value, XATOI_NONNEG, "config.tint2_rules");
	} else if (!strncmp(option, "at_pointer", 10)) {
		xatoi(&config.at_pointer, value, XATOI_NONNEG, "config.at_pointer");
	} else if (!strcmp(option, "terminal_exec")) {
		xfree(config.terminal_exec);
		config.terminal_exec = xstrdup(value);
	} else if (!strcmp(option, "terminal_args")) {
		xfree(config.terminal_args);
		config.terminal_args = xstrdup(value);

	} else if (!strncmp(option, "menu_margin_x", 13)) {
		xatoi(&config.menu_margin_x, value, XATOI_NONNEG, "config.margin_x");
	} else if (!strncmp(option, "menu_margin_y", 13)) {
		xatoi(&config.menu_margin_y, value, XATOI_NONNEG, "config.margin_y");
	} else if (!strncmp(option, "menu_width", 10)) {
		xatoi(&config.menu_width, value, XATOI_GT_0, "config.menu_width");
	} else if (!strcmp(option, "menu_padding_top")) {
		xatoi(&config.menu_padding_top, value, XATOI_NONNEG, "config.menu_padding_top");
	} else if (!strcmp(option, "menu_padding_right")) {
		xatoi(&config.menu_padding_right, value, XATOI_NONNEG, "config.menu_padding_right");
	} else if (!strcmp(option, "menu_padding_bottom")) {
		xatoi(&config.menu_padding_bottom, value, XATOI_NONNEG, "config.menu_padding_bottom");
	} else if (!strcmp(option, "menu_padding_left")) {
		xatoi(&config.menu_padding_left, value, XATOI_NONNEG, "config.menu_padding_left");
	} else if (!strncmp(option, "menu_radius", 11)) {
		xatoi(&config.menu_radius, value, XATOI_NONNEG, "config.menu_radius");
	} else if (!strncmp(option, "menu_border", 11)) {
		xatoi(&config.menu_border, value, XATOI_NONNEG, "config.menu_border");
	} else if (!strncmp(option, "menu_halign", 11)) {
		if (!value)
			return;
		if (!strcasecmp(value, "left"))
			config.menu_halign = LEFT;
		else if (!strcasecmp(value, "right"))
			config.menu_halign = RIGHT;
	} else if (!strncmp(option, "menu_valign", 11)) {
		if (!value)
			return;
		if (!strcasecmp(value, "top"))
			config.menu_valign = TOP;
		else if (!strcasecmp(value, "bottom"))
			config.menu_valign = BOTTOM;

	} else if (!strncmp(option, "item_margin_x", 13)) {
		xatoi(&config.item_margin_x, value, XATOI_NONNEG, "config.item_margin_x");
	} else if (!strncmp(option, "item_margin_y", 13)) {
		xatoi(&config.item_margin_y, value, XATOI_NONNEG, "config.item_margin_y");
	} else if (!strncmp(option, "item_height", 11)) {
		xatoi(&config.item_height, value, XATOI_GT_0, "config.item_height");
	} else if (!strncmp(option, "item_padding_x", 14)) {
		xatoi(&config.item_padding_x, value, XATOI_NONNEG, "config.item_padding_x");
	} else if (!strncmp(option, "item_radius", 11)) {
		xatoi(&config.item_radius, value, XATOI_NONNEG, "config.item_radius");
	} else if (!strncmp(option, "item_border", 11)) {
		xatoi(&config.item_border, value, XATOI_NONNEG, "config.item_border");
	} else if (!strncmp(option, "item_halign", 11)) {
		if (!value)
			return;
		if (!strcasecmp(value, "left"))
			config.item_halign = LEFT;
		else if (!strcasecmp(value, "right"))
			config.item_halign = RIGHT;
	} else if (!strncmp(option, "sep_height", 10)) {
		xatoi(&config.sep_height, value, XATOI_NONNEG, "config.sep_height");

	} else if (!strcmp(option, "font")) {
		xfree(config.font);
		config.font = xstrdup(value);
	} else if (!strncmp(option, "font_fallback", 13)) {
		xfree(config.font_fallback);
		config.font_fallback = xstrdup(value);
	} else if (!strncmp(option, "icon_size", 9)) {
		xatoi(&config.icon_size, value, XATOI_NONNEG, "config.icon_size");
	} else if (!strncmp(option, "icon_text_spacing", 17)) {
		xatoi(&config.icon_text_spacing, value, XATOI_NONNEG, "config.icon_text_spacing");
	} else if (!strcmp(option, "icon_theme")) {
		xfree(config.icon_theme);
		config.icon_theme = xstrdup(value);
	} else if (!strncmp(option, "icon_theme_fallback", 14)) {
		xfree(config.icon_theme_fallback);
		config.icon_theme_fallback = xstrdup(value);

	} else if (!strncmp(option, "arrow_string", 11)) {
		xfree(config.arrow_string);
		config.arrow_string = xstrdup(value);
	} else if (!strncmp(option, "arrow_width", 11)) {
		xatoi(&config.arrow_width, value, XATOI_NONNEG, "config.arrow_width");

	} else if (!strncmp(option, "color_menu_bg", 13)) {
		parse_hexstr(value, config.color_menu_bg);
	} else if (!strncmp(option, "color_menu_fg", 13)) {
		parse_hexstr(value, config.color_menu_fg);
	} else if (!strncmp(option, "color_menu_border", 17)) {
		parse_hexstr(value, config.color_menu_border);
	} else if (!strncmp(option, "color_norm_bg", 13)) {
		parse_hexstr(value, config.color_norm_bg);
	} else if (!strncmp(option, "color_norm_fg", 13)) {
		parse_hexstr(value, config.color_norm_fg);
	} else if (!strncmp(option, "color_sel_bg", 12)) {
		parse_hexstr(value, config.color_sel_bg);
	} else if (!strncmp(option, "color_sel_fg", 12)) {
		parse_hexstr(value, config.color_sel_fg);
	} else if (!strncmp(option, "color_sel_border", 16)) {
		parse_hexstr(value, config.color_sel_border);
	} else if (!strncmp(option, "color_sep_fg", 12)) {
		parse_hexstr(value, config.color_sep_fg);
	}
}

static void read_file(FILE *fp)
{
	char line[1024];

	while (fgets(line, sizeof(line), fp))
		process_line(line);
}

void config_parse_file(char *filename)
{
	FILE *fp;

	fp = fopen(filename, "r");
	if (!fp) {
		fprintf(stderr, "warning: could not open config file %s\n", filename);
		return;
	}

	read_file(fp);
	fclose(fp);
}
