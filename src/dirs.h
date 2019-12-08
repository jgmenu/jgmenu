#ifndef JGMENU_DIRS_H
#define JGMENU_DIRS_H

#include "argv-buf.h"

struct dir {
	char *name;
	char *name_localized;
	char *icon;
	char *categories;
};

/**
 * dirs_read_schema - read and parse schema file to provide directory structure
 * @vector: pointer to null terminated directory vector
 */
void dirs_read_schema(struct dir **vector);

#endif /* JGMENU_DIRS_H */
