#include <stdio.h>
#include <stdlib.h>

#include "dirs.h"

int main(int argc, char **argv)
{
	struct dir *dirs = dirs_read_schema();

	for (int i = 0; i < dirs_nr(); i++) {
		struct dir *dir = dirs + i;
		printf("%s,%s\n", dir->name, dir->icon);
	}
	return 0;
}
