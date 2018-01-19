#include <stdio.h>
#include <stdlib.h>

#include "sbuf.h"
#include "fmt.h"

/*
 * %n - application name
 * %g - application generic name in brackets
 */
static const char default_name_format[] = "%n %g";

void fmt_name(struct sbuf *buf, const char *name, const char *generic_name)
{
	static int inited;
	static char *format;
	char *p;

	if (!inited) {
		format = getenv("JGMENU_NAME_FORMAT");
		if (!format)
			format = (char *)default_name_format;
		inited = 1;
	}
	sbuf_cpy(buf, "");
	for (p = &format[0]; p; p++) {
		if (!p || *p == '\0')
			return;
		if (*p != '%') {
			sbuf_addch(buf, p[0]);
			continue;
		}
		switch (*++p) {
		case '\0':
			--p;
			break;
		case 'n':
			if (!name)
				continue;
			sbuf_addstr(buf, name);
			break;
		case 'g':
			if (!generic_name || !name)
				continue;
			if (!strcmp(name, generic_name))
				continue;
			sbuf_addstr(buf, " (");
			sbuf_addstr(buf, generic_name);
			sbuf_addstr(buf, ")");
		case '%':
		default:
			break;
		}
	}
}
