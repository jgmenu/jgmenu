/* Simple tint2rc parser for jgmenu */

#include <string.h>
#include <sys/stat.h>

#include "util.h"
#include "config.h"
#include "sbuf.h"
#include "t2conf.h"
#include "align.h"

#define DEBUG_PRINT_VARIABLES 0
#define DEFAULT_TINT2RC "~/.config/tint2/tint2rc"
#define DELIM " \t\r\n"

static int g_screen_height, g_screen_width;
static int bg_id;

static double panel_background_color[4];
static double taskbar_background_color[4];
static int use_taskbar_background;

#define MAX_NR_BGS (16)

struct bg {
	int rounded;
	int border_width;
	char *background_color;
	char *border_color;
};

/* bg[0] is used for fully transparent bg */
static struct bg bg[MAX_NR_BGS + 1];

static enum alignment valign = UNKNOWN;
static enum alignment halign = UNKNOWN;
static enum alignment orientation = UNKNOWN;
static char *panel_width;
static char *panel_height;
static int panel_margin_h;
static int panel_margin_v;
static char *font;
static char *icon_theme;
static int override_xsettings;
static int taskbar_hpadding;
static int taskbar_vpadding;
static int taskbar_spacing;
static int panel_hpadding;
static int panel_vpadding;
static int panel_spacing;

static void say(const char *err, ...)
{
	va_list params;

	if (!DEBUG_PRINT_VARIABLES)
		return;
	fprintf(stderr, "  - ");
	va_start(params, err);
	vfprintf(stderr, err, params);
	va_end(params);
	fprintf(stderr, "\n");
}

static int parse_height(char *h)
{
	char *p;

	p = strchr(h, '%');
	if (!p)
		return atoi(h);
	*p = '\0';
	return atoi(h) / 100.0 * g_screen_height;
}

static int parse_width(char *w)
{
	char *p;

	p = strchr(w, '%');
	if (!p)
		return atoi(w);
	*p = '\0';
	return atoi(w) / 100.0 * g_screen_width;
}

static void process_line(char *line)
{
	char *option, *value, *field;
	int id;

	if (!parse_config_line(line, &option, &value))
		return;

	if (!strcmp(option, "rounded")) {
		bg_id++;
		bg[bg_id].rounded = atoi(value);
	} else if (!strcmp(option, "border_width")) {
		bg[bg_id].border_width = atoi(value);
	} else if (!strcmp(option, "background_color")) {
		bg[bg_id].background_color = strdup(value);
	} else if (!strcmp(option, "border_color")) {
		bg[bg_id].border_color = strdup(value);

	} else if (!strncmp(option, "panel_background_id", 19)) {
		id = atoi(value);
		if (id > MAX_NR_BGS) {
			warn("id too big");
			return;
		}
		say("panel_bg_col     = %s", bg[id].background_color);
		parse_hexstr(bg[id].background_color, panel_background_color);
		if (!id)
			use_taskbar_background = 1;

		/*
		 * We could parse set color_menu_border and border_width here,
		 * but choose not too because it sometimes looks a bit strange.
		 * parse_hexstr(bg[id].border_color, config.color_menu_border);
		 * config.menu_border = bg[id].border_width;
		 */

	} else if (!strncmp(option, "taskbar_background_id", 21)) {
		id = atoi(value);
		if (id > MAX_NR_BGS) {
			warn("id too big");
			return;
		}
		say("taskbar_bg_col   = %s", bg[id].background_color);
		parse_hexstr(bg[id].background_color, taskbar_background_color);
	} else if (!strncmp(option, "task_background_id", 18)) {
		id = atoi(value);
		if (id > MAX_NR_BGS) {
			warn("id too big");
			return;
		}

		/*
		 * We don't set color_norm_bg. Only the selected item gets
		 * a colour. If we wanted it, it would be:
		 * parse_hexstr(bg[id].background_color, config.color_norm_bg);
		 */

	} else if (!strncmp(option, "task_active_background_id", 25)) {
		id = atoi(value);
		if (id > MAX_NR_BGS) {
			warn("id too big");
			return;
		}
		say("item_radius      = %d", bg[id].rounded);
		config.item_radius = bg[id].rounded;
		say("color_sel_bg     = %s", bg[id].background_color);
		parse_hexstr(bg[id].background_color, config.color_sel_bg);
		say("color_sel_border = %s", bg[id].border_color);
		parse_hexstr(bg[id].border_color, config.color_sel_border);
		say("color_sep_fg     = %s", bg[id].border_color);
		parse_hexstr(bg[id].border_color, config.color_sep_fg);
		say("item_border      = %d", bg[id].border_width);
		config.item_border = bg[id].border_width;

	} else if (!strcmp(option, "task_font")) {
		say("font             = %s", value);
		xfree(font);
		font = xstrdup(value);
	} else if (!strcmp(option, "task_font_color")) {
		say("color_norm_fg    = %s", value);
		parse_hexstr(value, config.color_norm_fg);
	} else if (!strcmp(option, "task_active_font_color")) {
		say("color_sel_fg     = %s", value);
		parse_hexstr(value, config.color_sel_fg);
	} else if (!strcmp(option, "launcher_icon_theme")) {
		say("icon_theme       = %s", value);
		xfree(icon_theme);
		icon_theme = xstrdup(value);
	} else if (!strcmp(option, "launcher_icon_theme_override")) {
		say("override_xsetti. = %s", value);
		override_xsettings = atoi(value);
	} else if (!strcmp(option, "panel_position")) {
		field = strtok(value, DELIM);
		if (!field)
			return;
		if (!strcmp(field, "bottom"))
			valign = BOTTOM;
		else if (!strcmp(field, "top"))
			valign = TOP;
		else if (!strcmp(field, "center"))
			valign = CENTER;

		field = strtok(NULL, DELIM);
		if (!field)
			return;
		if (!strcmp(field, "left"))
			halign = LEFT;
		else if (!strcmp(field, "right"))
			halign = RIGHT;
		else if (!strcmp(field, "center"))
			halign = CENTER;

		field = strtok(NULL, DELIM);
		if (!field)
			return;
		if (!strcmp(field, "horizontal"))
			orientation = HORIZONTAL;
		else if (!strcmp(field, "vertical"))
			orientation = VERTICAL;

	} else if (!strcmp(option, "panel_size")) {
		/*
		 * For a vertical panel, panel_size's height/width are swapped
		 * We cannot calculate width/height at this point as we have
		 * to be able to resolve '%' and might not know alignment yet.
		 */
		field = strtok(value, DELIM);
		if (!field)
			return;
		panel_width = strdup(field);

		field = strtok(NULL, DELIM);
		if (!field)
			return;
		panel_height = strdup(field);

	} else if (!strcmp(option, "panel_margin")) {
		field = strtok(value, DELIM);
		if (!field)
			return;
		panel_margin_h = atoi(field);
		field = strtok(NULL, DELIM);
		if (!field)
			return;
		panel_margin_v = atoi(field);
	} else if (!strcmp(option, "taskbar_padding")) {
		field = strtok(value, DELIM);
		if (!field)
			return;
		taskbar_hpadding = atoi(field);
		field = strtok(NULL, DELIM);
		if (!field)
			return;
		taskbar_vpadding = atoi(field);
		field = strtok(NULL, DELIM);
		if (!field)
			return;
		taskbar_spacing = atoi(field);
	} else if (!strcmp(option, "panel_padding")) {
		field = strtok(value, DELIM);
		if (!field)
			return;
		panel_hpadding = atoi(field);
		field = strtok(NULL, DELIM);
		if (!field)
			return;
		panel_vpadding = atoi(field);
		field = strtok(NULL, DELIM);
		if (!field)
			return;
		panel_spacing = atoi(field);
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
	if (!fp) {
		warn("cannot open file '%s'", filename);
		return;
	}

	info("parsing tint2 config file '%s'", filename);
	read_file(fp);
	fclose(fp);
}

static void hpanel_set_margin_y(void)
{
	say("margin_y         = %d", parse_height(panel_height) +
	    panel_margin_v);
	config.menu_margin_y = parse_height(panel_height) + panel_margin_v;
	if (valign == TOP) {
		say("valign           = top");
		config.menu_valign = TOP;
	} else if (valign == BOTTOM) {
		say("valign           = bottom");
		config.menu_valign = BOTTOM;
	}
}

static void hpanel_set_margin_x(void)
{
	int x;

	say("halign           = left");
	config.menu_halign = LEFT;
	if (halign == CENTER)
		x = (g_screen_width - parse_width(panel_width) + panel_margin_h) / 2;
	else if (halign == RIGHT)
		x = g_screen_width - parse_width(panel_width);
	else
		x = panel_margin_h;
	say("margin_x         = %d", x);
	config.menu_margin_x = x;
}

static void vpanel_set_margin_x(void)
{
	say("margin_x         = %d", parse_width(panel_height) + panel_margin_h);
	config.menu_margin_x = parse_width(panel_height) + panel_margin_h;
	if (halign == LEFT) {
		say("halign           = left");
		config.menu_halign = LEFT;
	} else if (halign == RIGHT) {
		say("halign           = right");
		config.menu_halign = RIGHT;
	}
}

static void vpanel_set_margin_y(void)
{
	int y;

	say("valign           = top");
	config.menu_valign = TOP;
	/* panel_width here refers to panel "length" (=height!) */
	if (valign == CENTER)
		y = (g_screen_height - parse_height(panel_width) + panel_margin_v) / 2;
	else if (valign == BOTTOM)
		y = g_screen_height - parse_height(panel_width);
	else
		y = panel_margin_v;
	say("margin_y         = %d", y);
	config.menu_margin_y = y;
}

static int max2(int a, int b)
{
	return (a > b) ? a : b;
}

static int max3(int a, int b, int c)
{
	return ((a > b) & (a > c)) ? a :
	       ((b > a) & (b > c)) ? b : c;
}

static void set_padding_and_item_margin(void)
{
	int vspace = panel_vpadding + taskbar_vpadding;
	int padding = max3(taskbar_hpadding, panel_spacing, panel_hpadding);

	config.item_margin_y = vspace;
	config.item_margin_x = 0;
	config.menu_padding_right = padding;
	config.menu_padding_left = padding;
	config.menu_padding_top = max2(0, padding - vspace);
	config.menu_padding_bottom = max2(0, padding - vspace);
}

static void set_alignment_and_position(void)
{
	int i;

	if (use_taskbar_background) {
		for (i = 0; i < 4; i++)
			config.color_menu_bg[i] = taskbar_background_color[i];
	} else {
		for (i = 0; i < 4; i++)
			config.color_menu_bg[i] = panel_background_color[i];
	}

	if (DEBUG_PRINT_VARIABLES) {
		fprintf(stderr, "  - orientation      = ");
		if (orientation == HORIZONTAL)
			fprintf(stderr, "horizontal\n");
		else if (orientation == VERTICAL)
			fprintf(stderr, "vertical\n");
	}

	/*
	 * If "panel_shrink = 1" in tint2rc, one of menu_margin_{x,y} is not
	 * accurately set (i.e. x for horizontal and y for vertical).
	 * Also, the menu will always align to the edge of the panel.
	 * For more accurate positioning use IPC (ipc.c)
	 */
	if (orientation == HORIZONTAL) {
		hpanel_set_margin_y();
		hpanel_set_margin_x();
	}

	if (orientation == VERTICAL) {
		vpanel_set_margin_x();
		vpanel_set_margin_y();
	}

	set_padding_and_item_margin();
}

static void t2conf_cleanup(void)
{
	int i;

	for (i = 0; i < MAX_NR_BGS; i++) {
		if (bg[i].background_color)
			free(bg[i].background_color);
		if (bg[i].border_color)
			free(bg[i].border_color);
	}
	if (panel_width)
		free(panel_width);
	panel_width = NULL;
	if (panel_height)
		free(panel_height);
	panel_height = NULL;
}

void t2conf_atexit(void)
{
	xfree(font);
	xfree(icon_theme);
}

void t2conf_parse(const char *filename, int screen_width, int screen_height)
{
	struct sbuf tint2rc;
	struct stat sb;

	g_screen_width = screen_width;
	g_screen_height = screen_height;
	bg[0].background_color = strdup("#000000 00");
	bg[0].border_color = strdup("#000000 00");
	sbuf_init(&tint2rc);
	if (filename && filename[0] != '\0')
		sbuf_addstr(&tint2rc, filename);
	else
		sbuf_addstr(&tint2rc, DEFAULT_TINT2RC);
	sbuf_expand_tilde(&tint2rc);
	if (stat(tint2rc.buf, &sb) == -1)
		goto cleanup;
	parse_file(tint2rc.buf);
	set_alignment_and_position();
cleanup:
	free(tint2rc.buf);
	t2conf_cleanup();
}

void t2conf_get_font(char **f)
{
	*f = font;
}

int t2conf_get_override_xsettings(void)
{
	return override_xsettings;
}

void t2conf_get_icon_theme(char **t)
{
	*t = icon_theme;
}
