#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<string.h>

#include "util.h"
#include "config.h"

struct Config config;

void config_set_defaults(void)
{
	config.spawn		 = 1;

	config.menu_margin_x	 = 2;
	config.menu_margin_y	 = 33;
	config.menu_width	 = 200;
	config.menu_radius	 = 6;
	config.menu_border	 = 1;

	config.item_margin_x	 = 3;
	config.item_margin_y	 = 3;
	config.item_height	 = 25;
	config.item_padding_x	 = 4;
	config.item_radius	 = 3;
	config.item_border	 = 0;

	config.max_items	 = 80;
	config.min_items	 = 0;
	config.font		 = strdup("Ubuntu condensed 14px");
	config.icon_size	 = 22;
	config.icon_theme	 = NULL;
	config.ignore_xsettings  = 0;
	config.ignore_icon_cache = 0;
	config.show_title	 = 1;

	parse_hexstr("#000000 60", config.color_menu_bg);
	parse_hexstr("#eeeeee 20", config.color_menu_fg);
	parse_hexstr("#000000 00", config.color_norm_bg);
	parse_hexstr("#eeeeee 100", config.color_norm_fg);
	parse_hexstr("#ffffff 20", config.color_sel_bg);
	parse_hexstr("#eeeeee 100", config.color_sel_fg);
	parse_hexstr("#ff0000 80", config.color_noprog_fg);
	parse_hexstr("#ffffff 20", config.color_title_bg);
}

static void process_line(char *line)
{
	char *option, *value;

	if (!parse_config_line(line, &option, &value))
		return;

	if (!strncmp(option, "menu_margin_x", 13))
		config.menu_margin_x = atoi(value);
	else if (!strncmp(option, "menu_margin_y", 13))
		config.menu_margin_y = atoi(value);
	else if (!strncmp(option, "menu_width", 10))
		config.menu_width = atoi(value);
	else if (!strncmp(option, "menu_radius", 11))
		config.menu_radius = atoi(value);
	else if (!strncmp(option, "menu_border", 11))
		config.menu_border = atoi(value);

	else if (!strncmp(option, "item_margin_x", 13))
		config.item_margin_x = atoi(value);
	else if (!strncmp(option, "item_margin_y", 13))
		config.item_margin_y = atoi(value);
	else if (!strncmp(option, "item_height", 11))
		config.item_height = atoi(value);
	else if (!strncmp(option, "item_padding_x", 14))
		config.item_padding_x = atoi(value);
	else if (!strncmp(option, "item_radius", 11))
		config.item_radius = atoi(value);
	else if (!strncmp(option, "item_border", 11))
		config.item_border = atoi(value);

	else if (!strncmp(option, "max_items", 8))
		config.max_items = atoi(value);
	else if (!strncmp(option, "min_items", 8))
		config.min_items = atoi(value);
	else if (!strncmp(option, "font", 4))
		config.font = strdup(value);
	else if (!strncmp(option, "icon_size", 9))
		config.icon_size = atoi(value);
	else if (!strncmp(option, "icon_theme", 10))
		config.icon_theme = strdup(value);
	else if (!strncmp(option, "ignore_xsettings", 16))
		config.ignore_xsettings = atoi(value);
	else if (!strncmp(option, "ignore_icon_cache", 17))
		config.ignore_icon_cache = atoi(value);
	else if (!strncmp(option, "show_title", 10))
		config.show_title = atoi(value);

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
