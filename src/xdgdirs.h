#ifndef XDGDIRS_H
#define XDGDIRS_H

#include "list.h"
#include "sbuf.h"

void xdgdirs_get_basedirs(struct list_head *dir_list);
void xdgdirs_get_configdirs(struct list_head *dir_list);
void xdgdirs_find_menu_file(struct sbuf *filename);

#endif /* XDGDIRS */
