#include <stdio.h>
#include <stdlib.h>

#include "dirs.h"

int main(int argc, char **argv)
{
	struct dir *dir, *dirs;

	dirs_read_schema(&dirs);
	for (dir = dirs; dir->name; dir += 1)
		printf("%s,%s\n", dir->name, dir->icon);
	return 0;
}
