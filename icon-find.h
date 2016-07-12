#ifndef ICON_FIND_H
#define ICON_FIND_H

#include <stdio.h>

#include "sbuf.h"

extern void icon_find_add_theme(const char *theme);
extern void icon_find_print_themes(void);
extern void icon_find_init(void);
extern void icon_find(struct String *name, int size);

#endif /* ICON_FIND_H */
