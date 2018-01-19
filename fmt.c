#include <stdio.h>
#include <stdlib.h>

#include "sbuf.h"
#include "fmt.h"

void fmt_name(struct sbuf *buf, const char *name, const char *generic_name)
{
	sbuf_cpy(buf, "");
	if (!name)
		return;
	sbuf_addstr(buf, name);
	if (!generic_name)
		return;
	if (!strcmp(name, generic_name))
		return;
	sbuf_addstr(buf, " (");
	sbuf_addstr(buf, generic_name);
	sbuf_addstr(buf, ")");
}
