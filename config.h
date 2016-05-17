#ifndef CONFIG_H
#define CONFIG_H

struct Config {
	int spawn;		/* 1:execute commands  0:print to stdout */
	int debug_mode;		/* print debug messages */
	int item_height;	/* set to font height if greater */
	char *font;
	int menu_margin_x;
	int menu_margin_y;
	int menu_width;
	int menu_radius;
	int menu_border;
	int item_margin_x;
	int item_margin_y;
	int item_radius;
	int item_border;
	int item_padding_x;
	int max_items;		/* max number of items to show in menu */
				/* if more items present, user has to scroll */
	int min_items;

	double color_menu_bg[4];
	double color_menu_fg[4];
	double color_norm_bg[4];
	double color_norm_fg[4];
	double color_sel_bg[4];
	double color_sel_fg[4];
	double color_noprog_fg[4];
	double color_title_bg[4];
};

extern struct Config config;

void config_set_defaults(void);
void config_parse_file(char *filename);

#endif /* CONFIG_H */
