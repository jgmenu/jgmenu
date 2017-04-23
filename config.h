#ifndef CONFIG_H
#define CONFIG_H

#define JGMENU_DEFAULT_FONT "Cantarell 10"

struct config {
	int spawn;		/* 1:execute commands  0:print to stdout */
	int stay_alive;
	int hide_on_startup;
	/* jgmenurc has a csv_cmd variable here */
	int tint2_look;
	int tint2_button;
	int tint2_rules;

	int menu_margin_x;
	int menu_margin_y;
	int menu_width;
	int menu_radius;
	int menu_border;
	char *menu_halign;
	char *menu_valign;
	int at_pointer;

	int item_margin_x;
	int item_margin_y;
	int item_height;	/* set to font height if greater */
	int item_padding_x;
	int item_radius;
	int item_border;
	int sep_height;

	char *font;
	int icon_size;		/* if set to zero, icons won't show */
	char *icon_theme;
	int ignore_xsettings;
	int ignore_icon_cache;
	int show_title;		/* will only show if ^tag() is set */

	char *arrow_string;
	int arrow_show;
	int search_all_items;

	double color_menu_bg[4];
	double color_menu_fg[4];
	double color_menu_border[4];
	double color_norm_bg[4];
	double color_norm_fg[4];
	double color_sel_bg[4];
	double color_sel_fg[4];
	double color_sel_border[4];
	double color_noprog_fg[4];
	double color_title_bg[4];
	double color_sep_fg[4];
};

extern struct config config;

void config_set_defaults(void);
void config_parse_file(char *filename);

#endif /* CONFIG_H */
