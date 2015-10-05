#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<string.h>

#include "config.h"

void config_set_defaults() {

	config.spawn = 1;

	menu.win_y0    = 0;
	menu.item_h    = 30;			/* code will set to font height if greater */
/*	menu.font      = strdup("Sans 18px");	*/
	menu.font      = strdup("Ubuntu condensed 18px");
	menu.title     = NULL;

	menu.normbgcol = strdup("#bbbbbb");
	menu.normfgcol = strdup("#222222");
	menu.selbgcol  = strdup("#3388cc");
	menu.selfgcol  = strdup("#eeeeee");

}
