#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"
#include "sbuf.h"
#include "util.h"
#include "banned.h"

static int ah = 40;
static int bw = 150;
static int cw = 300;
static int ch = 470;
static int pad = 4;
static int item_height = 35;

static char *categories[] = {
	"All:*",
	"Accessories:#Accessories #Util #Archiving #Compression #TextEditor ",
	"Development:#Development #Build #Debug #IDE #Profiling #RevisionControl ",
	"Games:#Game ",
	"Graphics:#Graphics #Scanning #Photography ",
	"Internet:#Internet #Network #Email #FileTransfer #WebBrowser ",
	"Multimedia:#Multimedia #Player #Audio #Video #Recorder ",
	"Office:#Office #Calendar #Contact #Dictionary #Chart #Finance #Presentation #Spreadsheet #WordProcessor #Publishing #Viewer ",
	"Settings:#Settings #Security #Preferences ",
	"System:#System #Emulator #FileManager #TerminalEmulator #Filesystem #Monitor ",
	NULL
};

static const char greeneye_usage[] =
"Usage: jgmenu_run greeneye [options]\n"
"Output config file and widget lines for a menu with the following layout:\n\n"
" +----------------+-----------------------+\n"
" |                |  a - search box       |\n"
" |                |-----------------------|\n"
" |                |                       |\n"
" | b - categories |                       |\n"
" |                |  c - applications     |\n"
" |                |                       |\n"
" |                |                       |\n"
" +----------------+-----------------------+\n\n"
"Options:\n"
"    --widgets     print widget lines\n"
"    --config      print config lines\n\n"
"Example:\n"
"    jgmenu_run greeneye --config  >$HOME/.config/jgmenu/jgmenurc\n"
"    jgmenu_run greeneye --widgets >$HOME/.config/jgmenu/prepend.csv\n";

static void usage(void)
{
	printf("%s", greeneye_usage);
	exit(0);
}

static void print_categories(void)
{
	int i, y;
	char buf[1024], *p;

	y = pad;
	for (i = 0; categories[i]; i++) {
		strlcpy(buf, categories[i], sizeof(buf));
		buf[1023] = '\0';
		p = strchr(buf, ':');
		if (!p)
			continue;
		*p = '\0';
		++p;
		printf("@rect,^filter(%s),%d,%d,%d,%d,2,left,top,#000000 0,#000000 0,\n",
		       p, pad, y, bw - pad * 2, item_height);
		printf("@text,,%d,%d,%d,%d,2,left,top,#e6e6e6 100,#000000 0,%s\n",
		       pad + 5, y, bw - pad * 2, item_height, buf);
		y += item_height;
	}
}

#define SEARCH_ICON "~/.config/jgmenu/greeneye-search.svg"
#define ICON_SIZE (22)
#define ICON_MARGINH (30)
#define ICON_MARGINV (8)
static void output_widgets(void)
{
	struct sbuf s;

	sbuf_init(&s);
	sbuf_addstr(&s, SEARCH_ICON);
	sbuf_expand_tilde(&s);
	/* Search box */
	printf("@rect,,%d,%d,%d,%d,2,left,top,#000000 0,#656565 50,\n",
	       bw + pad, pad, cw - pad * 2, ah - pad);
	printf("@search,,%d,%d,%d,%d,2,left,top,#eeeeee 80,#000000 0,\n",
	       bw + pad + 5, pad, cw - pad * 2 - ICON_SIZE - ICON_MARGINH,
	       ah - pad);
	printf("@icon,,%d,%d,%d,%d,2,left,top,#000000 50,#000000 50,%s\n",
	       bw + cw - pad - ICON_MARGINH, pad + ICON_MARGINV, ICON_SIZE,
	       ICON_SIZE, s.buf);

	/* Grey area for menus items */
	printf("@rect,,%d,%d,%d,%d,2,left,top,#000000 0,#282828 90,\n",
	       bw + pad, ah + pad, cw - pad * 2, ch - pad * 2);

	xfree(s.buf);
	print_categories();
}

static void output_config(void)
{
	printf("csv_cmd             = apps\n");
	printf("tint2_look          = 0\n");
	printf("menu_width          = %d\n", bw + cw);
	printf("menu_height_min     = %d\n", ah + ch);
	printf("menu_height_max     = %d\n", ah + ch);
	printf("menu_padding_top    = %d\n", ah);
	printf("menu_padding_right  = 2\n");
	printf("menu_padding_bottom = 1\n");
	printf("menu_padding_left   = %d\n", bw + 1);
	printf("menu_border         = 1\n");
	printf("item_height         = %d\n", item_height);
	printf("item_radius         = 2\n");
	printf("item_border         = 1\n");
	printf("color_menu_bg       = #212121 100\n");
	printf("color_menu_border   = #eeeeee 20\n");
	printf("#color_norm_fg       = #eeeeee 100\n");
	printf("#color_sel_bg        = #ffffff 20\n");
	printf("#color_sel_fg        = #eeeeee 100\n");
	printf("#color_sel_border    = #eeeeee 8\n");
	printf("color_scroll_ind    = #000000 0\n");
	printf("csv_name_format     = %%n\\n<span size=\"x-small\">%%g</span>\n");
	printf("csv_no_dirs         = 1\n");
}

int main(int argc, char **argv)
{
	int i;

	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "--widgets")) {
			output_widgets();
			exit(0);
		} else if (!strcmp(argv[i], "--config")) {
			output_config();
			exit(0);
		}
	}
	usage();
	return 0;
}
