#ifndef CONFIG_H
#define CONFIG_H

#define JGMENU_DEFAULT_FONT "Cantarell 10"

struct config {
	int spawn;		/* 1:execute commands  0:print to stdout */

	int menu_margin_x;
	int menu_margin_y;
	int menu_width;
	int menu_radius;
	int menu_border;

	int item_margin_x;
	int item_margin_y;
	int item_height;	/* set to font height if greater */
	int item_padding_x;
	int item_radius;
	int item_border;

	int max_items;		/* max number of items to show in menu */
				/* if more items present, user has to scroll */
	int min_items;
	char *font;
	int icon_size;		/* if set to zero, icons won't show */
	char *icon_theme;
	int ignore_xsettings;
	int ignore_icon_cache;
	int show_title;		/* will only show if ^tag() is set */

	double color_menu_bg[4];
	double color_menu_fg[4];
	double color_norm_bg[4];
	double color_norm_fg[4];
	double color_sel_bg[4];
	double color_sel_fg[4];
	double color_noprog_fg[4];
	double color_title_bg[4];
};

extern struct config config;

void config_set_defaults(void);
void config_parse_file(char *filename);

#endif /* CONFIG_H */
