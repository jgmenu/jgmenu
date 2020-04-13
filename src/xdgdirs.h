#ifndef JGMENU_XDGDIRS_H
#define JGMENU_XDGDIRS_H

#include "list.h"
#include "sbuf.h"

/**
 * xdgdirs_get_datadirs - populate list with XDG data directories
 * @dir_list: pointer to the directory list
 */
void xdgdirs_get_datadirs(struct list_head *dir_list);

/**
 * xdgdirs_get_configdirs - populate list with XDG config directories
 * @dir_list: pointer to the directory list
 */
void xdgdirs_get_configdirs(struct list_head *dir_list);

/**
 * xdgdirs_find_menu_file - find XDG applications-*.menu file
 * @filename: pointer to filename
 */
void xdgdirs_find_menu_file(struct sbuf *filename);

#endif /* JGMENU_XDGDIRS_H */
