#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "util.h"
#include "config.h"

struct config config;

void config_set_defaults(void)
{
	config.spawn		 = 1;

	config.menu_margin_x	 = 0;
	config.menu_margin_y	 = 31;
	config.menu_width	 = 200;
	config.menu_radius	 = 1;
	config.menu_border	 = 0;
	config.menu_halign	 = NULL;
	config.menu_valign	 = NULL;

	config.item_margin_x	 = 3;
	config.item_margin_y	 = 3;
	config.item_height	 = 25;
	config.item_padding_x	 = 4;
	config.item_radius	 = 1;
	config.item_border	 = 0;

	config.max_items	 = 80;
	config.min_items	 = 1;
	config.font		 = NULL; /* Leave as NULL. Is set in jgmenu.c */
	config.icon_size	 = 22;
	config.icon_theme	 = NULL; /* Leave as NULL. Is set in config-xs.c */
	config.ignore_xsettings  = 0;
	config.ignore_icon_cache = 0;
	config.show_title	 = 0;

	config.arrow_string	 = strdup("▸");
	config.arrow_show	 = 1;
	config.search_all_items  = 1;

	parse_hexstr("#000000 70", config.color_menu_bg);
	parse_hexstr("#eeeeee 20", config.color_menu_fg);
	parse_hexstr("#000000 00", config.color_norm_bg);
	parse_hexstr("#eeeeee 100", config.color_norm_fg);
	parse_hexstr("#ffffff 20", config.color_sel_bg);
	parse_hexstr("#eeeeee 100", config.color_sel_fg);
	parse_hexstr("#eeeeee 100", config.color_noprog_fg);
	parse_hexstr("#ffffff 20", config.color_title_bg);
}

static void process_line(char *line)
{
	char *option, *value;

	if (!parse_config_line(line, &option, &value))
		return;

	if (!strncmp(option, "menu_margin_x", 13))
		xatoi(&config.menu_margin_x, value, XATOI_NONNEG, "config.margin_x");
	else if (!strncmp(option, "menu_margin_y", 13))
		xatoi(&config.menu_margin_y, value, XATOI_NONNEG, "config.margin_y");
	else if (!strncmp(option, "menu_width", 10))
		xatoi(&config.menu_width, value, XATOI_GT_0, "config.menu_width");
	else if (!strncmp(option, "menu_radius", 11))
		xatoi(&config.menu_radius, value, XATOI_NONNEG, "config.menu_radius");
	else if (!strncmp(option, "menu_border", 11))
		xatoi(&config.menu_border, value, XATOI_NONNEG, "config.menu_border");
	else if (!strncmp(option, "menu_halign", 11))
		config.menu_halign = strdup(value);
	else if (!strncmp(option, "menu_valign", 11))
		config.menu_valign = strdup(value);

	else if (!strncmp(option, "item_margin_x", 13))
		xatoi(&config.item_margin_x, value, XATOI_NONNEG, "config.item_margin_x");
	else if (!strncmp(option, "item_margin_y", 13))
		xatoi(&config.item_margin_y, value, XATOI_NONNEG, "config.item_margin_y");
	else if (!strncmp(option, "item_height", 11))
		xatoi(&config.item_height, value, XATOI_GT_0, "config.item_height");
	else if (!strncmp(option, "item_padding_x", 14))
		xatoi(&config.item_padding_x, value, XATOI_NONNEG, "config.item_padding_x");
	else if (!strncmp(option, "item_radius", 11))
		xatoi(&config.item_radius, value, XATOI_NONNEG, "config.item_radius");
	else if (!strncmp(option, "item_border", 11))
		xatoi(&config.item_border, value, XATOI_NONNEG, "config.item_border");

	else if (!strncmp(option, "max_items", 9))
		xatoi(&config.max_items, value, XATOI_GT_0, "config.max_items");
	else if (!strncmp(option, "min_items", 9))
		xatoi(&config.min_items, value, XATOI_GT_0, "config.min_items");
	else if (!strncmp(option, "font", 4))
		config.font = strdup(value);
	else if (!strncmp(option, "icon_size", 9))
		xatoi(&config.icon_size, value, XATOI_NONNEG, "config.icon_size");
	else if (!strncmp(option, "icon_theme", 10))
		config.icon_theme = strdup(value);
	else if (!strncmp(option, "ignore_xsettings", 16))
		xatoi(&config.ignore_xsettings, value, XATOI_NONNEG, "config.ignore_xsettings");
	else if (!strncmp(option, "ignore_icon_cache", 17))
		xatoi(&config.ignore_icon_cache, value, XATOI_NONNEG, "config.ignore_icon_cache");
	else if (!strncmp(option, "show_title", 10))
		xatoi(&config.show_title, value, XATOI_NONNEG, "config.show_title");

	else if (!strncmp(option, "arrow_string", 11))
		config.arrow_string = strdup(value);
	else if (!strncmp(option, "arrow_show", 10))
		xatoi(&config.arrow_show, value, XATOI_NONNEG, "config.arrow_show");
	else if (!strncmp(option, "search_all_items", 16))
		xatoi(&config.search_all_items, value, XATOI_NONNEG, "config.search_all_items");

	else if (!strncmp(option, "color_menu_bg", 13))
		parse_hexstr(value, config.color_menu_bg);
	else if (!strncmp(option, "color_menu_fg", 13))
		parse_hexstr(value, config.color_menu_fg);
	else if (!strncmp(option, "color_norm_bg", 13))
		parse_hexstr(value, config.color_norm_bg);
	else if (!strncmp(option, "color_norm_fg", 13))
		parse_hexstr(value, config.color_norm_fg);
	else if (!strncmp(option, "color_sel_bg", 12))
		parse_hexstr(value, config.color_sel_bg);
	else if (!strncmp(option, "color_sel_fg", 12))
		parse_hexstr(value, config.color_sel_fg);
	else if (!strncmp(option, "color_noprog_fg", 14))
		parse_hexstr(value, config.color_noprog_fg);
	else if (!strncmp(option, "color_title_bg", 14))
		parse_hexstr(value, config.color_title_bg);
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
