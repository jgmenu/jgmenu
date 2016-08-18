/*
 * xdgapps.c creates cache for .desktop- and .directory-files
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include "xdgapps.h"
#include "util.h"
#include "sbuf.h"
#include "list.h"

#define APPLICATIONS "/usr/share/applications/"
#define DESKTOP_DIRECTORIES "/usr/share/desktop-directories/"

struct list_head desktop_files_all;
struct list_head desktop_files_filtered;

struct list_head directory_files;

static void parse_directory_file(FILE *fp, const char *filename)
{
	char line[8192];
	char *p;
	struct Directory_file_data *tmp;

	tmp = xcalloc(1, sizeof(struct Directory_file_data));

	tmp->filename = strdup(filename);

	while (fgets(line, sizeof(line), fp)) {
		p = strchr(line, '\n');
		if (p)
			*p = '\0';

		if (!strncmp("Name=", line, 5))
			tmp->name = strdup(line + 5);
		else if (!strncmp("Icon=", line, 5))
			tmp->icon = strdup(line + 5);
	}

	list_add_tail(&(tmp->list), &directory_files);
}

static void parse_desktop_file(FILE *fp)
{
	char line[8192];
	char *p;
	struct Desktop_file_data *tmp;

	tmp = xcalloc(1, sizeof(struct Desktop_file_data));

	while (fgets(line, sizeof(line), fp)) {
		p = strchr(line, '\n');
		if (p)
			*p = '\0';

		if (!strncmp("Name=", line, 5))
			tmp->name = strdup(line + 5);
		else if (!strncmp("Exec=", line, 5))
			tmp->exec = strdup(line + 5);
		else if (!strncmp("Icon=", line, 5))
			tmp->icon = strdup(line + 5);
		else if (!strncmp("Categories=", line, 11))
			tmp->categories = strdup(line + 11);
	}

	/* Remove %U, %f, etc at the end of Exec cmd */
	if (tmp->exec) {
		p = strchr(tmp->exec, '%');
		if (p)
			*p = '\0';
	}

	list_add_tail(&(tmp->full_list), &desktop_files_all);
}

static void process_file(char *filename, int isdir)
{
	FILE *fp;
	char fullname[4096];

	if (isdir) {
		strncpy(fullname, DESKTOP_DIRECTORIES, strlen(DESKTOP_DIRECTORIES));
		strncpy(fullname + strlen(DESKTOP_DIRECTORIES), filename, strlen(filename) + 1);
	} else {
		strncpy(fullname, APPLICATIONS, strlen(APPLICATIONS));
		strncpy(fullname + strlen(APPLICATIONS), filename, strlen(filename) + 1);
	}

	fp = fopen(fullname, "r");
	if (!fp)
		die("could not open file %s", filename);

	if (isdir)
		parse_directory_file(fp, filename);
	else
		parse_desktop_file(fp);

	fclose(fp);
}

/*
 * Set isdir to 1 to process .directory files
 */
static void init_list(const char *path, int isdir)
{
	struct dirent *entry;
	DIR *dp;

	dp = opendir(path);
	if (!dp)
		die("could not open dir %s", path);

	while ((entry = readdir(dp))) {
		if (!strncmp(entry->d_name, ".", 1) ||
		    !strncmp(entry->d_name, "..", 2))
			continue;
		process_file(entry->d_name, isdir);
	}

	closedir(dp);
}

void xdgapps_init_lists(void)
{
	INIT_LIST_HEAD(&desktop_files_all);
	INIT_LIST_HEAD(&desktop_files_filtered);
	INIT_LIST_HEAD(&directory_files);
	init_list(APPLICATIONS, 0);
	init_list(DESKTOP_DIRECTORIES, 1);
}

void xdgapps_filter_desktop_files_on_category(const char *category)
{
	struct Desktop_file_data *a, *pos;

	list_for_each_entry_safe(pos, a, &desktop_files_filtered, filtered_list)
		list_del(&pos->filtered_list);

	list_for_each_entry(a, &desktop_files_all, full_list) {
		if (!a->name || !a->categories)
			continue;

		if (strstr(a->categories, category))
			list_add_tail(&(a->filtered_list), &desktop_files_filtered);
	}
}
