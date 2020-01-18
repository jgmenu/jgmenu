#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "compat.h"
#include "util.h"

static char _ll[32] = { 0 };
static char _llcc[32] = { 0 };
static char _name_ll[32] = { 0 }; /* Name[ll] */
static char _name_llcc[32] = { 0 }; /* Name[ll_CC] */
static char _gname_ll[32] = { 0 }; /* GenericName[ll] */
static char _gname_llcc[32] = { 0 }; /* GenericName[ll_CC] */

static void lang_init(void)
{
	static bool has_been_initialized;
	char *p;

	if (has_been_initialized)
		return;
	has_been_initialized = true;

	p = getenv("LANG");
	if (!p) {
		warn("$LANG not set");
		return;
	}

	/* ll_CC */
	strlcpy(_llcc, p, sizeof(_llcc));
	p = strrchr(_llcc, '.');
	if (p)
		*p = '\0';

	/* ll */
	strlcpy(_ll, _llcc, sizeof(_ll));
	p = strrchr(_ll, '_');
	if (p)
		*p = '\0';

	snprintf(_name_ll, sizeof(_name_ll), "Name[%s]", _ll);
	snprintf(_name_llcc, sizeof(_name_llcc), "Name[%s]", _llcc);
	snprintf(_gname_ll, sizeof(_gname_ll), "GenericName[%s]", _ll);
	snprintf(_gname_llcc, sizeof(_gname_llcc), "GenericName[%s]", _llcc);
}

char *lang_name_ll(void)
{
	lang_init();
	return _name_ll;
}

char *lang_name_llcc(void)
{
	lang_init();
	return _name_llcc;
}

char *lang_gname_ll(void)
{
	lang_init();
	return _gname_ll;
}

char *lang_gname_llcc(void)
{
	lang_init();
	return _gname_llcc;
}
