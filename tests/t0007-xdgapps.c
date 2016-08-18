#include <stdio.h>

#include "../xdgapps.h"
#include "../sbuf.h"
#include "../list.h"

int main(int argc, char **argv)
{
	struct Desktop_file_data *desktop_file;

	xdgapps_init_lists();

	xdgapps_filter_desktop_files_on_category("xxxxxxxxxxxxxxxxx");

/*	list_for_each_entry(desktop_file, &desktop_files_all, full_list) */

	printf("Audio:\n");
	xdgapps_filter_desktop_files_on_category("Audio");
	list_for_each_entry(desktop_file, &desktop_files_filtered, filtered_list)
		printf("%s,%s,%s\n", desktop_file->name, desktop_file->exec, desktop_file->name);

	printf("Graphics:\n");
	xdgapps_filter_desktop_files_on_category("Graphics");
	list_for_each_entry(desktop_file, &desktop_files_filtered, filtered_list)
		printf("%s,%s,%s\n", desktop_file->name, desktop_file->exec, desktop_file->name);

}
