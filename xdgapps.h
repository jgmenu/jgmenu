#ifndef XDGAPPS_H
#define XDGAPPS_H

#include "list.h"

struct desktop_file_data {
	char *name;
	char *exec;
	char *icon;
	char *categories;
	struct list_head full_list;
	struct list_head filtered_list;
};

struct directory_file_data {
	char *filename;
	char *name;
	char *icon;
	struct list_head list;
};

extern struct list_head desktop_files_all;
extern struct list_head desktop_files_filtered;
extern struct list_head directory_files;

void xdgapps_init_lists(void);
void xdgapps_filter_desktop_files_on_category(const char *category);

#endif /* XDGAPPS_H */
