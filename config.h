#ifndef CONFIG_H
#define CONFIG_H

#define MAX_FIELDS 3		/* nr fields to parse for each stdin line */

struct Item {
 	char *t[MAX_FIELDS];	/* pointers name, cmd */
	char *tag;		/* used to tag the start of a submenu */
	struct Item *prev;
	struct Item *next;			
};


struct Config {
	int spawn;		
};
struct Config config;


struct Menu {
	int screen_x0;
	int screen_y0;
	int screen_w;
	int screen_h;

	int win_x0;
	int win_y0;

	int menu_h;
	int menu_w;
	int item_h;

	char *normbgcol;
	char *normfgcol;
	char *selbgcol;
	char *selfgcol;

	char *font;
	int font_height;	/* logical font height calculated by pango */

	struct Item *head;	/* pointer to the first menu item		*/
	struct Item *tail;	/* end of dynamic array				*/
	int end;		/* number of items in dynamic array		*/

	struct Item *sel;	/* pointer to the currently selected item	*/
	struct Item *first;	/* pointer to the first item in submenu		*/ 
	struct Item *last;	/* pointer to the first item in submenu		*/ 
	int nr_items;		/* number of items in menu/submenu		*/

	char *title;	

/*
 *	margin_x		between screen and menu
 *	margin_y		between screen and menu
 *	padding_x		between items and menu edge
 *	padding_y		between items
 *	item_w
 *	align_bottom
 *	align_left
 */
};

struct Menu menu;

void config_populate();

#endif /* CONFIG_H */
