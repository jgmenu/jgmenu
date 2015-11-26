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

	char *normbgcol;
	char *normfgcol;
	char *selbgcol;
	char *selfgcol;
};

extern struct Config config;

void config_set_defaults(void);
void config_parse_file(char *filename);

#endif /* CONFIG_H */
