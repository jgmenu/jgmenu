#include <stdio.h>

#include "../xdgicon.h"
#include "../sbuf.h"


static void test(struct String *name, const char *theme, int size)
{
	printf("INPUT: %s; %s; %d\n", name->buf, theme, size);
	icon_find(name, theme, size);
	printf("OUTPUT: %s\n", name->buf);
}

int main(int argc, char **argv)
{
	struct String a, b;

	sbuf_init(&a);
	sbuf_addstr(&a, "utilities-terminal");
	test(&a, "Adwaita", 48);

	sbuf_init(&b);
	sbuf_addstr(&b, "firefox");
	test(&b, "Adwaita", 48);
}
