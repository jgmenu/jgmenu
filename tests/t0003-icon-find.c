#include <stdio.h>

#include "../icon-find.h"
#include "../sbuf.h"
#include "../util.h"


static void test(struct sbuf *name, int size)
{
	printf("INPUT: %s; %d\n", name->buf, size);
	icon_find(name, size);
	printf("OUTPUT: %s\n", name->buf);
}

int main(int argc, char **argv)
{
	struct sbuf s;

	sbuf_init(&s);

	icon_find_init();

	icon_find_add_theme("Adwaita");
	sbuf_cpy(&s, "folder-documents");
	test(&s, 22);

	icon_find_add_theme("Adwaita");
	sbuf_cpy(&s, "utilities-terminal");
	test(&s, 48);

	sbuf_cpy(&s, "firefox");
	test(&s, 48);

	sbuf_cpy(&s, "apps/firefox");
	test(&s, 48);
}
