#ifndef JGMENU_DESKTOP_H
#define JGMENU_DESKTOP_H

#include <stdbool.h>

#include "list.h"

struct app {
	char *name;
	char *name_localized;
	char *generic_name;
	char *generic_name_localized;
	char *exec;
	char *icon;
	char *categories;
	bool nodisplay;
	char *filename;
	bool terminal;
	bool has_been_mapped;
};

/**
 * desktop_read_files - read and parse system .desktop files
 * Return null terminated app vector
 */
struct app *desktop_read_files(void);

#endif /* JGMENU_DESKTOP_H */
