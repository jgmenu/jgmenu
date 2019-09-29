#ifndef DESKTOP_H
#define DESKTOP_H

#include "list.h"

struct app {
	char *name;
	char *exec;
	char *icon;
	char *categories;
	int nodisplay;
	char *filename;
};

struct app *desktop_read_files(void);
int desktop_nr_apps(void);

#endif /* DESKTOP_H */
