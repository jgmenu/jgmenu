#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<string.h>

#include "util.h"
#include "config.h"

struct Config config;

void config_set_defaults(void)
{
	config.spawn		= 1;
	config.debug_mode	= 0;
	config.item_height	= 30;
	config.font		= strdup("Ubuntu condensed 18px");
	config.menu_margin_x	= 2;
	config.menu_margin_y	= 32;
	config.menu_width	= 200;

	parse_hexstr("#000000 40", config.color_norm_bg);
	parse_hexstr("#eeeeee 100", config.color_norm_fg);
	parse_hexstr("#ffbb11 80", config.color_sel_bg);
	parse_hexstr("#000000 100", config.color_sel_fg);
	parse_hexstr("#ff0000 100", config.color_broke_fg);
	parse_hexstr("#ff0000 70", config.color_title_bg);
}

void process_line(char *line)
{
	char *option, *value;

	if (!parse_config_line(line, &option, &value))
		return;

	if (!strncmp(option, "item_height", 11))
		config.item_height = atoi(value);
	else if (!strncmp(option, "font", 4))
		config.font = strdup(value);
	else if (!strncmp(option, "menu_margin_x", 13))
		config.menu_margin_x = atoi(value);
	else if (!strncmp(option, "menu_margin_y", 13))
		config.menu_margin_y = atoi(value);
	else if (!strncmp(option, "menu_width", 10))
		config.menu_width = atoi(value);

	else if (!strncmp(option, "color_norm_bg", 13))
		parse_hexstr(value, config.color_norm_bg);
	else if (!strncmp(option, "color_norm_fg", 13))
		parse_hexstr(value, config.color_norm_fg);
	else if (!strncmp(option, "color_sel_bg", 12))
		parse_hexstr(value, config.color_sel_bg);
	else if (!strncmp(option, "color_sel_fg", 12))
		parse_hexstr(value, config.color_sel_fg);
	else if (!strncmp(option, "color_broke_fg", 14))
		parse_hexstr(value, config.color_broke_fg);
}


void read_file(FILE *fp)
{
	char line[1024];

	while (fgets(line, sizeof(line), fp))
		process_line(line);
}

void config_parse_file(char *filename)
{
	FILE *fp;

	fp = fopen(filename, "r");
	if (!fp)
		die("could not open file %s", filename);
	read_file(fp);
	fclose(fp);
}
