#ifndef JGMENU_DIRS_H
#define JGMENU_DIRS_H

struct dir {
	char *name;
	char *icon;
	char *categories;
};

int dirs_nr(void);

/**
 * Return a vector of directories
 */
struct dir *dirs_read_schema(void);

#endif /* JGMENU_DIRS_H */
