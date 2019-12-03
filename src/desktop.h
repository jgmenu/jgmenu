#ifndef JGMENU_DESKTOP_H
#define JGMENU_DESKTOP_H

#include <stdbool.h>

#include "list.h"

struct app {
	char *name;
	char *exec;
	char *icon;
	char *categories;
	int nodisplay;
	char *filename;
	bool has_been_mapped;
};

/**
 * desktop_read_files - read and parse system .desktop files
 * Return null terminated app vector
 */
struct app *desktop_read_files(void);

#endif /* JGMENU_DESKTOP_H */
