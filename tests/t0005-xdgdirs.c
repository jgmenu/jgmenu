#include <stdio.h>

#include "../xdgdirs.h"
#include "../sbuf.h"
#include "../list.h"


int main(int argc, char **argv)
{
	struct list_head foo;
	struct String *dir;

	INIT_LIST_HEAD(&foo);

	xdgdirs_get_basedirs(&foo);

	list_for_each_entry(dir, &foo, list)
		printf("%s\n", dir->buf);
}
