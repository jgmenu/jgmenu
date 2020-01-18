#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "compat.h"
#include "util.h"

static char ll[24] = { 0 };
static char llcc[24] = { 0 };

/*
 * Static name keys for GenericName[ll_CC], etc
 */
static char name_ll[64] = { 0 };
static char name_llcc[64] = { 0 };
static char gname_ll[64] = { 0 };
static char gname_llcc[64] = { 0 };

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
	strlcpy(llcc, p, sizeof(llcc));
	p = strrchr(llcc, '.');
	if (p)
		*p = '\0';

	/* ll */
	strlcpy(ll, llcc, sizeof(ll));
	p = strrchr(ll, '_');
	if (p)
		*p = '\0';

	snprintf(name_ll, sizeof(name_ll), "Name[%s]", ll);
	snprintf(name_llcc, sizeof(name_llcc), "Name[%s]", llcc);
	snprintf(gname_ll, sizeof(gname_ll), "GenericName[%s]", ll);
	snprintf(gname_llcc, sizeof(gname_llcc), "GenericName[%s]", llcc);
}

char *lang_name_ll(void)
{
	lang_init();
	return name_ll;
}

char *lang_name_llcc(void)
{
	lang_init();
	return name_llcc;
}

char *lang_gname_ll(void)
{
	lang_init();
	return gname_ll;
}

char *lang_gname_llcc(void)
{
	lang_init();
	return gname_llcc;
}
