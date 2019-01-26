#ifndef ICON_FIND_H
#define ICON_FIND_H

#include <stdio.h>

#include "sbuf.h"

struct icon_path {
	struct sbuf name;
	struct sbuf path;
	int smallest_match;
	int found;
	void *icon;
	struct list_head list;
};

extern void icon_find_add_theme(const char *theme);
extern void icon_find_print_themes(void);
extern void icon_find_init(void);
extern void icon_find_all(struct list_head *icons, int size);
extern void icon_find(struct sbuf *name, int size);
extern void icon_find_cleanup(void);

#endif /* ICON_FIND_H */
