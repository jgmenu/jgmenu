#include <stdio.h>

#include "../xdgapps.h"
#include "../sbuf.h"
#include "../list.h"

int main(int argc, char **argv)
{
	struct desktop_file_data *desktop_file;

	xdgapps_init_lists();

/*	list_for_each_entry(desktop_file, &desktop_files_all, full_list) */

	printf("Development:\n");
	xdgapps_filter_desktop_files_on_category("Development");
	list_for_each_entry(desktop_file, &desktop_files_filtered, filtered_list)
		printf("%s,%s,%s\n", desktop_file->name, desktop_file->exec, desktop_file->icon);

}
