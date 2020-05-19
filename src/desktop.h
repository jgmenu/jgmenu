#ifndef JGMENU_DESKTOP_H
#define JGMENU_DESKTOP_H

#include <stdbool.h>

#include "list.h"

struct app {
	char name[128];
	char name_localized[128];
	char generic_name[128];
	char generic_name_localized[128];
	char exec[128];
	char icon[128];
	char categories[512];
	bool nodisplay;
	char filename[128];
	bool terminal;
	bool has_been_mapped;
	bool end;
};

/**
 * desktop_read_files - read and parse system .desktop files
 * Return null terminated app vector
 */
struct app *desktop_read_files(void);

#endif /* JGMENU_DESKTOP_H */
