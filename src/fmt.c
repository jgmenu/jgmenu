#include <stdio.h>
#include <stdlib.h>

#include "sbuf.h"
#include "fmt.h"
#include "banned.h"

/*
 * %n - application name
 * %g - application generic name
 *
 * Note: If a 'generic name' does not exist or is the same as the 'name',
 *       %n will be returned without formatting
 */
static const char default_name_format[] = "%n (%g)";

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
	if (!name)
		return;
	if (!generic_name || generic_name[0] == '\0' ||
	    !strcasecmp(name, generic_name)) {
		sbuf_cpy(buf, name);
		return;
	}
	for (p = &format[0]; p && *p; p++) {
		if (*p != '%') {
			sbuf_addch(buf, p[0]);
			continue;
		}
		switch (*++p) {
		case '\0':
			--p;
			break;
		case 'n':
			sbuf_addstr(buf, name);
			break;
		case 'g':
			sbuf_addstr(buf, generic_name);
			break;
		case '%':
		default:
			break;
		}
	}
}
