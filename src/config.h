#ifndef CONFIG_H
#define CONFIG_H

#include "align.h"

#define CONFIG_AUTO (-9999)
#define CONFIG_STATIC (-9998)
#define CONFIG_DYNAMIC (-9997)

enum position_mode {POSITION_MODE_FIXED, POSITION_MODE_IPC, POSITION_MODE_PTR,
		    POSITION_MODE_CENTER, POSITION_MODE_UNKNOWN};

struct config {
	int spawn;		/* 1:execute commands  0:print to stdout */
	int verbosity;
	int stay_alive;
	int hide_on_startup;
	char *csv_cmd;
	int tint2_look;
	enum position_mode position_mode;
	int respect_workarea;	/* set with position_mode */
	int edge_snap_x;
	char *terminal_exec;
	char *terminal_args;
	int monitor;
	int hover_delay;
	int hide_back_items;
	int columns;
	int tabs;

	int menu_margin_x;
	int menu_margin_y;
	int menu_width;
	int menu_height_min;
	int menu_height_max;
	int menu_height_mode;
	int menu_padding_top;
	int menu_padding_right;
	int menu_padding_bottom;
	int menu_padding_left;
	int menu_radius;
	int menu_border;
	enum alignment menu_halign;
	enum alignment menu_valign;

	int sub_spacing;
	int sub_padding_top;
	int sub_padding_right;
	int sub_padding_bottom;
	int sub_padding_left;
	int sub_hover_action;

	int item_margin_x;
	int item_margin_y;
	int item_height;	/* set to font height if greater */
	int item_padding_x;
	int item_radius;
	int item_border;
	enum alignment item_halign;
	int sep_height;
	char *sep_markup;
	enum alignment sep_halign;

	char *font;
	char *font_fallback;
	int icon_size;		/* if set to zero, icons won't show */
	int icon_text_spacing;
	int icon_norm_alpha;
	int icon_sel_alpha;
	char *icon_theme;
	char *icon_theme_fallback;

	char *arrow_string;
	int arrow_width;

	double color_menu_bg[4];
	double color_menu_border[4];
	double color_norm_bg[4];
	double color_norm_fg[4];
	double color_sel_bg[4];
	double color_sel_fg[4];
	double color_sel_border[4];
	double color_sep_fg[4];
	double color_title_fg[4];
	double color_title_bg[4];
	double color_title_border[4];
	double color_scroll_ind[4];

	char *csv_name_format;
	int csv_single_window;
	int csv_no_dirs;
	char *csv_i18n;
	int csv_no_duplicates;
};

extern struct config config;

void config_set_defaults(void);
void config_cleanup(void);
void config_read_jgmenurc(const char *filename);
void config_post_process(void);

#endif /* CONFIG_H */
