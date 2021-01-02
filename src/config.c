#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "util.h"
#include "config.h"
#include "list.h"
#include "sbuf.h"
#include "xdgdirs.h"
#include "banned.h"

struct config config;
static struct sbuf jgmenurc_file;

/* clang-format off */
void config_set_defaults(void)
{
	config.spawn		   = 1;	/* not in jgmenurc */
	config.verbosity	   = 0;
	config.stay_alive	   = 1;
	config.persistent          = 0;
	config.hide_on_startup	   = 0;
	config.csv_cmd		   = xstrdup("apps");
	config.tint2_look	   = 0;
	config.position_mode	   = POSITION_MODE_FIXED;
	config.respect_workarea	   = 1;	/* set in config_post_process() */
	config.edge_snap_x	   = 30;
	config.terminal_exec	   = xstrdup("x-terminal-emulator");
	config.terminal_args	   = xstrdup("-e");
	config.monitor		   = 0;
	config.hover_delay	   = 100;
	config.hide_back_items	   = 1;
	config.columns		   = 1;
	config.tabs		   = 120;

	config.menu_margin_x	   = 0;
	config.menu_margin_y	   = 0;
	config.menu_width	   = 200;
	config.menu_height_min	   = 0;
	config.menu_height_max	   = 0;
	config.menu_height_mode	   = CONFIG_STATIC;
	config.menu_padding_top	   = 5;
	config.menu_padding_right  = 5;
	config.menu_padding_bottom = 5;
	config.menu_padding_left   = 5;
	config.menu_radius	   = 1;
	config.menu_border	   = 0;
	config.menu_halign	   = LEFT;
	config.menu_valign	   = BOTTOM;
	config.menu_gradient_pos   = ALIGNMENT_NONE;

	config.sub_spacing	   = 1;
	config.sub_padding_top	   = CONFIG_AUTO;
	config.sub_padding_right   = CONFIG_AUTO;
	config.sub_padding_bottom  = CONFIG_AUTO;
	config.sub_padding_left	   = CONFIG_AUTO;
	config.sub_hover_action    = 1;

	config.item_margin_x	   = 3;
	config.item_margin_y	   = 3;
	config.item_height	   = 25;
	config.item_padding_x	   = 4;
	config.item_radius	   = 1;
	config.item_border	   = 0;
	config.item_halign	   = LEFT;

	config.sep_height	   = 5;
	config.sep_markup	   = NULL;
	config.sep_halign	   = CENTER;

	config.font		   = NULL; /* Leave as NULL (see font.c) */
	config.font_fallback	   = xstrdup("xtg");
	config.icon_size	   = 22;
	config.icon_text_spacing   = 10;
	config.icon_norm_alpha	   = 100;
	config.icon_sel_alpha	   = 100;
	config.icon_theme	   = NULL; /* Leave as NULL (see theme.c) */
	config.icon_theme_fallback = xstrdup("xtg");

	config.arrow_string	   = xstrdup("▸");
	config.arrow_width	   = 15;

	parse_hexstr("#000000 100", config.color_menu_bg);
	parse_hexstr("#000000 100", config.color_menu_bg_to);
	parse_hexstr("#eeeeee 8", config.color_menu_border);
	parse_hexstr("#000000 00", config.color_norm_bg);
	parse_hexstr("#eeeeee 100", config.color_norm_fg);
	parse_hexstr("#ffffff 20", config.color_sel_bg);
	parse_hexstr("#eeeeee 100", config.color_sel_fg);
	parse_hexstr("#eeeeee 8", config.color_sel_border);
	parse_hexstr("#ffffff 20", config.color_sep_fg);
	parse_hexstr("#eeeeee 50", config.color_title_fg);
	parse_hexstr("#000000 0", config.color_title_bg);
	parse_hexstr("#000000 0", config.color_title_border);
	parse_hexstr("#eeeeee 40", config.color_scroll_ind);

	config.csv_name_format	   = NULL; /* Leave as NULL (see in fmt.c) */
	config.csv_single_window   = 0;
	config.csv_no_dirs	   = 0;
	config.csv_i18n		   = NULL;
	config.csv_no_duplicates = 0;
}

/* clang-format on */

void config_cleanup(void)
{
	xfree(config.csv_cmd);
	xfree(config.terminal_exec);
	xfree(config.terminal_args);
	xfree(config.font);
	xfree(config.font_fallback);
	xfree(config.icon_theme);
	xfree(config.icon_theme_fallback);
	xfree(config.arrow_string);
	xfree(jgmenurc_file.buf);
	xfree(config.csv_name_format);
}

static void process_line(char *line)
{
	char *option, *value;

	if (!parse_config_line(line, &option, &value))
		return;

	if (!strcmp(option, "verbosity")) {
		xatoi(&config.verbosity, value, XATOI_NONNEG,
		      "config.verbosity");
	} else if (!strcmp(option, "stay_alive")) {
		xatoi(&config.stay_alive, value, XATOI_NONNEG,
		      "config.stay_alive");
	} else if (!strcmp(option, "persistent")) {
		xatoi(&config.persistent, value, XATOI_NONNEG,
		      "config.persistent");
	} else if (!strcmp(option, "hide_on_startup")) {
		xatoi(&config.hide_on_startup, value, XATOI_NONNEG,
		      "config.hide_on_startup");
	} else if (!strcmp(option, "csv_cmd")) {
		xfree(config.csv_cmd);
		config.csv_cmd = xstrdup(value);
	} else if (!strcmp(option, "tint2_look")) {
		xatoi(&config.tint2_look, value, XATOI_NONNEG,
		      "config.tint2_look");
	} else if (!strcmp(option, "at_pointer")) {
		if (atoi(value) == 1)
			config.position_mode = POSITION_MODE_PTR;
		warn("'at_pointer' is deprecated; use 'position_mode'");

	} else if (!strcmp(option, "position_mode")) {
		if (!value)
			return;
		if (!strcasecmp(value, "fixed")) {
			config.position_mode = POSITION_MODE_FIXED;
			config.respect_workarea = 1;
		} else if (!strcasecmp(value, "ipc")) {
			config.position_mode = POSITION_MODE_IPC;
			config.respect_workarea = 0;
		} else if (!strcasecmp(value, "pointer")) {
			config.position_mode = POSITION_MODE_PTR;
			config.respect_workarea = 1;
		} else if (!strcasecmp(value, "center")) {
			config.position_mode = POSITION_MODE_CENTER;
			config.respect_workarea = 0;
			config.menu_valign = CENTER;
			config.menu_halign = CENTER;
		} else {
			warn("position_mode value '%s' not recognised", value);
		}

	} else if (!strcmp(option, "edge_snap_x")) {
		xatoi(&config.edge_snap_x, value, XATOI_NONNEG,
		      "config.edge_snap_x");
	} else if (!strcmp(option, "terminal_exec")) {
		xfree(config.terminal_exec);
		config.terminal_exec = xstrdup(value);
	} else if (!strcmp(option, "terminal_args")) {
		xfree(config.terminal_args);
		config.terminal_args = xstrdup(value);
	} else if (!strcmp(option, "monitor")) {
		xatoi(&config.monitor, value, XATOI_NONNEG, "config.monitor");
	} else if (!strcmp(option, "hover_delay")) {
		xatoi(&config.hover_delay, value, XATOI_NONNEG,
		      "config.hover_delay");
	} else if (!strcmp(option, "hide_back_items")) {
		xatoi(&config.hide_back_items, value, XATOI_NONNEG,
		      "config.hide_back_items");
	} else if (!strcmp(option, "columns")) {
		xatoi(&config.columns, value, XATOI_GT_0, "config.columns");
	} else if (!strcmp(option, "tabs")) {
		xatoi(&config.tabs, value, XATOI_NONNEG, "config.tabs");

	} else if (!strcmp(option, "menu_margin_x")) {
		xatoi(&config.menu_margin_x, value, XATOI_NONNEG,
		      "config.margin_x");
	} else if (!strcmp(option, "menu_margin_y")) {
		xatoi(&config.menu_margin_y, value, XATOI_NONNEG,
		      "config.margin_y");
	} else if (!strcmp(option, "menu_width")) {
		xatoi(&config.menu_width, value, XATOI_GT_0,
		      "config.menu_width");
	} else if (!strcmp(option, "menu_height_min")) {
		xatoi(&config.menu_height_min, value, XATOI_NONNEG,
		      "config.menu_height_min");
	} else if (!strcmp(option, "menu_height_max")) {
		xatoi(&config.menu_height_max, value, XATOI_NONNEG,
		      "config.menu_height_max");
	} else if (!strcmp(option, "menu_height_mode")) {
		if (!value)
			return;
		if (!strcasecmp(value, "static"))
			config.menu_height_mode = CONFIG_STATIC;
		else if (!strcasecmp(value, "dynamic"))
			config.menu_height_mode = CONFIG_DYNAMIC;
	} else if (!strcmp(option, "menu_padding_top")) {
		xatoi(&config.menu_padding_top, value, XATOI_NONNEG,
		      "config.menu_padding_top");
	} else if (!strcmp(option, "menu_padding_right")) {
		xatoi(&config.menu_padding_right, value, XATOI_NONNEG,
		      "config.menu_padding_right");
	} else if (!strcmp(option, "menu_padding_bottom")) {
		xatoi(&config.menu_padding_bottom, value, XATOI_NONNEG,
		      "config.menu_padding_bottom");
	} else if (!strcmp(option, "menu_padding_left")) {
		xatoi(&config.menu_padding_left, value, XATOI_NONNEG,
		      "config.menu_padding_left");
	} else if (!strcmp(option, "menu_radius")) {
		xatoi(&config.menu_radius, value, XATOI_NONNEG,
		      "config.menu_radius");
	} else if (!strcmp(option, "menu_border")) {
		xatoi(&config.menu_border, value, XATOI_NONNEG,
		      "config.menu_border");
	} else if (!strcmp(option, "menu_halign")) {
		if (!value)
			return;
		if (!strcasecmp(value, "left"))
			config.menu_halign = LEFT;
		else if (!strcasecmp(value, "right"))
			config.menu_halign = RIGHT;
		else if (!strcasecmp(value, "center"))
			config.menu_halign = CENTER;
	} else if (!strcmp(option, "menu_valign")) {
		if (!value)
			return;
		if (!strcasecmp(value, "top"))
			config.menu_valign = TOP;
		else if (!strcasecmp(value, "bottom"))
			config.menu_valign = BOTTOM;
		else if (!strcasecmp(value, "center"))
			config.menu_valign = CENTER;

	} else if (!strcmp(option, "sub_spacing")) {
		xatoi(&config.sub_spacing, value, 0, "config.sub_spacing");
	} else if (!strcmp(option, "sub_padding_top")) {
		if (!value)
			return;
		if (!strcasecmp(value, "auto"))
			config.sub_padding_top = CONFIG_AUTO;
		else
			xatoi(&config.sub_padding_top, value, XATOI_NONNEG,
			      "config.sub_padding_top");
	} else if (!strcmp(option, "sub_padding_right")) {
		if (!value)
			return;
		if (!strcasecmp(value, "auto"))
			config.sub_padding_right = CONFIG_AUTO;
		else
			xatoi(&config.sub_padding_right, value, XATOI_NONNEG,
			      "config.sub_padding_right");
	} else if (!strcmp(option, "sub_padding_bottom")) {
		if (!value)
			return;
		if (!strcasecmp(value, "auto"))
			config.sub_padding_bottom = CONFIG_AUTO;
		else
			xatoi(&config.sub_padding_bottom, value, XATOI_NONNEG,
			      "config.sub_padding_bottom");
	} else if (!strcmp(option, "sub_padding_left")) {
		if (!value)
			return;
		if (!strcasecmp(value, "auto"))
			config.sub_padding_left = CONFIG_AUTO;
		else
			xatoi(&config.sub_padding_left, value, XATOI_NONNEG,
			      "config.sub_padding_left");
	} else if (!strcmp(option, "sub_hover_action")) {
		xatoi(&config.sub_hover_action, value, XATOI_NONNEG,
		      "config.sub_hover_action");

	} else if (!strcmp(option, "item_margin_x")) {
		xatoi(&config.item_margin_x, value, XATOI_NONNEG,
		      "config.item_margin_x");
	} else if (!strcmp(option, "item_margin_y")) {
		xatoi(&config.item_margin_y, value, XATOI_NONNEG,
		      "config.item_margin_y");
	} else if (!strcmp(option, "item_height")) {
		xatoi(&config.item_height, value, XATOI_GT_0,
		      "config.item_height");
	} else if (!strcmp(option, "item_padding_x")) {
		xatoi(&config.item_padding_x, value, XATOI_NONNEG,
		      "config.item_padding_x");
	} else if (!strcmp(option, "item_radius")) {
		xatoi(&config.item_radius, value, XATOI_NONNEG,
		      "config.item_radius");
	} else if (!strcmp(option, "item_border")) {
		xatoi(&config.item_border, value, XATOI_NONNEG,
		      "config.item_border");
	} else if (!strcmp(option, "item_halign")) {
		if (!value)
			return;
		if (!strcasecmp(value, "left"))
			config.item_halign = LEFT;
		else if (!strcasecmp(value, "right"))
			config.item_halign = RIGHT;
	} else if (!strcmp(option, "sep_height")) {
		xatoi(&config.sep_height, value, XATOI_NONNEG,
		      "config.sep_height");
	} else if (!strcmp(option, "sep_markup")) {
		xfree(config.sep_markup);
		config.sep_markup = xstrdup(value);
	} else if (!strcmp(option, "sep_halign")) {
		if (!value)
			return;
		if (!strcasecmp(value, "left"))
			config.sep_halign = LEFT;
		else if (!strcasecmp(value, "right"))
			config.sep_halign = RIGHT;
		else if (!strcasecmp(value, "center"))
			config.sep_halign = CENTER;

	} else if (!strcmp(option, "font")) {
		xfree(config.font);
		config.font = xstrdup(value);
	} else if (!strcmp(option, "font_fallback")) {
		xfree(config.font_fallback);
		config.font_fallback = xstrdup(value);
	} else if (!strcmp(option, "icon_size")) {
		xatoi(&config.icon_size, value, XATOI_NONNEG,
		      "config.icon_size");
	} else if (!strcmp(option, "icon_text_spacing")) {
		xatoi(&config.icon_text_spacing, value, XATOI_NONNEG,
		      "config.icon_text_spacing");
	} else if (!strcmp(option, "icon_norm_alpha")) {
		xatoi(&config.icon_norm_alpha, value, XATOI_NONNEG,
		      "config.icon_norm_alpha");
	} else if (!strcmp(option, "icon_sel_alpha")) {
		xatoi(&config.icon_sel_alpha, value, XATOI_NONNEG,
		      "config.icon_sel_alpha");
	} else if (!strcmp(option, "icon_theme")) {
		xfree(config.icon_theme);
		config.icon_theme = xstrdup(value);
	} else if (!strcmp(option, "icon_theme_fallback")) {
		xfree(config.icon_theme_fallback);
		config.icon_theme_fallback = xstrdup(value);

	} else if (!strcmp(option, "arrow_string")) {
		xfree(config.arrow_string);
		config.arrow_string = xstrdup(value);
	} else if (!strcmp(option, "arrow_width")) {
		xatoi(&config.arrow_width, value, XATOI_NONNEG,
		      "config.arrow_width");

	} else if (!strcmp(option, "color_menu_bg")) {
		parse_hexstr(value, config.color_menu_bg);
	} else if (!strcmp(option, "color_menu_bg_to")) {
		parse_hexstr(value, config.color_menu_bg_to);
	} else if (!strcmp(option, "color_menu_border")) {
		parse_hexstr(value, config.color_menu_border);
	} else if (!strcmp(option, "color_norm_bg")) {
		parse_hexstr(value, config.color_norm_bg);
	} else if (!strcmp(option, "color_norm_fg")) {
		parse_hexstr(value, config.color_norm_fg);
	} else if (!strcmp(option, "color_sel_bg")) {
		parse_hexstr(value, config.color_sel_bg);
	} else if (!strcmp(option, "color_sel_fg")) {
		parse_hexstr(value, config.color_sel_fg);
	} else if (!strcmp(option, "color_sel_border")) {
		parse_hexstr(value, config.color_sel_border);
	} else if (!strcmp(option, "color_sep_fg")) {
		parse_hexstr(value, config.color_sep_fg);
	} else if (!strcmp(option, "color_title_fg")) {
		parse_hexstr(value, config.color_title_fg);
	} else if (!strcmp(option, "color_title_bg")) {
		parse_hexstr(value, config.color_title_bg);
	} else if (!strcmp(option, "color_title_border")) {
		parse_hexstr(value, config.color_title_border);
	} else if (!strcmp(option, "color_scroll_ind")) {
		parse_hexstr(value, config.color_scroll_ind);
	} else if (!strcmp(option, "csv_name_format")) {
		xfree(config.csv_name_format);
		config.csv_name_format = xstrdup(value);
	} else if (!strcmp(option, "csv_single_window")) {
		xatoi(&config.csv_single_window, value, XATOI_NONNEG,
		      "config.csv_single_window");
	} else if (!strcmp(option, "csv_no_dirs")) {
		xatoi(&config.csv_no_dirs, value, XATOI_NONNEG,
		      "config.csv_no_dirs");
	} else if (!strcmp(option, "csv_i18n")) {
		xfree(config.csv_i18n);
		config.csv_i18n = xstrdup(value);
	} else if (!strcmp(option, "csv_no_duplicates")) {
		xatoi(&config.csv_no_duplicates, value, XATOI_NONNEG,
		      "config.csv_no_duplicates");
	} else if (!strcmp(option, "menu_gradient_pos")) {
		if (!value)
			return;
		if (!strcasecmp(value, "top"))
			config.menu_gradient_pos = TOP;
		else if (!strcasecmp(value, "right"))
			config.menu_gradient_pos = RIGHT;
		else if (!strcasecmp(value, "left"))
			config.menu_gradient_pos = LEFT;
		else if (!strcasecmp(value, "bottom"))
			config.menu_gradient_pos = BOTTOM;
		else if (!strcasecmp(value, "top_left"))
			config.menu_gradient_pos = TOP_LEFT;
		else if (!strcasecmp(value, "top_right"))
			config.menu_gradient_pos = TOP_RIGHT;
		else if (!strcasecmp(value, "bottom_left"))
			config.menu_gradient_pos = BOTTOM_LEFT;
		else if (!strcasecmp(value, "bottom_right"))
			config.menu_gradient_pos = BOTTOM_RIGHT;
	}
}

static void read_file(FILE *fp)
{
	char line[1024];

	while (fgets(line, sizeof(line), fp))
		process_line(line);
}

static void parse_file(char *filename)
{
	FILE *fp;

	fp = fopen(filename, "r");
	if (!fp)
		return;
	read_file(fp);
	fclose(fp);
}

void config_read_jgmenurc(const char *filename)
{
	struct stat sb;
	static int initiated;
	LIST_HEAD(config_dirs);
	struct sbuf *tmp;

	/* use default values for --config-file= without file specified */
	if (filename && filename[0] == '\0')
		return;
	if (initiated)
		goto parse;
	sbuf_init(&jgmenurc_file);
	if (filename) {
		sbuf_cpy(&jgmenurc_file, filename);
		sbuf_expand_tilde(&jgmenurc_file);
	}
	/*
	 * We look for jgmenurc in the following order:
	 *   --config-file=
	 *   ${XDG_CONFIG_HOME:-$HOME/.config}
	 *   ${XDG_CONFIG_DIRS:-/etc/xdg}
	 */
	if (!jgmenurc_file.len) {
		xdgdirs_get_configdirs(&config_dirs);
		list_for_each_entry(tmp, &config_dirs, list) {
			sbuf_addstr(tmp, "/jgmenu/jgmenurc");
			if (!stat(tmp->buf, &sb)) {
				sbuf_cpy(&jgmenurc_file, tmp->buf);
				break;
			}
		}
		sbuf_list_free(&config_dirs);
	}
	initiated = 1;
	if (stat(jgmenurc_file.buf, &sb) < 0)
		return;
	if (getenv("JGMENU_CONFIG_FILE_INFO"))
		info("using config file %s", jgmenurc_file.buf);
parse:
	parse_file(jgmenurc_file.buf);
}

static void set_floor(int *var, int floor)
{
	if (floor > *var)
		*var = floor;
}

static int smallest_of_four(int a, int b, int c, int d)
{
	a = a > b ? b : a;
	a = a > c ? c : a;
	a = a > d ? d : a;
	return a;
}

void config_post_process(void)
{
	int smallest_padding;

	/*
	 * The menu-border is drawn 'inside' the menu. Therefore, padding_* has
	 * to allow for the border thickness.
	 */
	set_floor(&config.menu_padding_bottom, config.menu_border);
	set_floor(&config.menu_padding_left, config.menu_border);
	set_floor(&config.menu_padding_right, config.menu_border);
	set_floor(&config.menu_padding_top, config.menu_border);

	smallest_padding = smallest_of_four(config.menu_padding_top,
					    config.menu_padding_left,
					    config.menu_padding_bottom,
					    config.menu_padding_right);
	/*
	 * Use '< 0' to include both CONFIG_AUTO and -1 (for backward
	 * compatibility)
	 */
	if (config.sub_padding_top < 0)
		config.sub_padding_top = smallest_padding;
	if (config.sub_padding_right < 0)
		config.sub_padding_right = smallest_padding;
	if (config.sub_padding_bottom < 0)
		config.sub_padding_bottom = smallest_padding;
	if (config.sub_padding_left < 0)
		config.sub_padding_left = smallest_padding;

	/* Resolve csv_cmd keywords */
	if (!strcmp(config.csv_cmd, "pmenu")) {
		xfree(config.csv_cmd);
		config.csv_cmd = xstrdup("jgmenu_run pmenu");
	} else if (!strcmp(config.csv_cmd, "lx")) {
		xfree(config.csv_cmd);
		config.csv_cmd = xstrdup("jgmenu_run lx");
	} else if (!strcmp(config.csv_cmd, "apps")) {
		xfree(config.csv_cmd);
		config.csv_cmd = xstrdup("jgmenu_run apps");
	} else if (!strcmp(config.csv_cmd, "ob")) {
		xfree(config.csv_cmd);
		config.csv_cmd = xstrdup("jgmenu_run ob");
	}

	if (config.menu_height_max &&
	    config.menu_height_min > config.menu_height_max)
		warn("menu_height_min cannot be greater than menu_height_max");

	if (config.verbosity) {
		char buf[8];

		snprintf(buf, sizeof(buf), "%d", config.verbosity);
		setenv("JGMENU_VERBOSITY", buf, 1);
	}

	if (config.csv_name_format)
		setenv("JGMENU_NAME_FORMAT", config.csv_name_format, 1);
	if (config.csv_single_window)
		setenv("JGMENU_SINGLE_WINDOW", "1", 1);
	else
		unsetenv("JGMENU_SINGLE_WINDOW");
	if (config.csv_no_dirs)
		setenv("JGMENU_NO_DIRS", "1", 1);
	else
		unsetenv("JGMENU_NO_DIRS");
	if (config.csv_i18n)
		setenv("JGMENU_I18N", config.csv_i18n, 1);
	if (config.csv_no_duplicates)
		setenv("JGMENU_NO_DUPLICATES", "1", 1);
	else
		unsetenv("JGMENU_NO_DUPLICATES");
}
