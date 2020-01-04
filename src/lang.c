#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compat.h"

static char short_lang_code[32];	/* e.g. "sv" */
static char long_lang_code[32];		/* e.g. "sv_SE */
static char name_key_ll[32];		/* e.g. "Name[sv]" */
static char name_key_ll_cc[32];		/* e.g. "Name[sv_SE]" */
static char generic_name_key_ll[32];	/* e.g. "Name[sv]" */
static char generic_name_key_ll_cc[32];	/* e.g. "Name[sv_SE]" */

int lang_code(char **ll, char **ll_cc)
{
	char *p;

	p = getenv("LANG");
	if (!p)
		return -1;

	/* ll_CC */
	strlcpy(long_lang_code, p, sizeof(long_lang_code));
	p = strrchr(long_lang_code, '.');
	if (p)
		*p = '\0';

	/* ll */
	strlcpy(short_lang_code, long_lang_code, sizeof(short_lang_code));
	p = strrchr(short_lang_code, '_');
	if (p)
		*p = '\0';

	*ll = short_lang_code;
	*ll_cc = long_lang_code;
	return 0;
}

void lang_localized_name_key(char **name_ll, char **name_ll_cc)
{
	char *ll, *ll_cc;

	lang_code(&ll, &ll_cc);
	snprintf(name_key_ll, sizeof(name_key_ll), "Name[%s]", ll);
	snprintf(name_key_ll_cc, sizeof(name_key_ll_cc), "Name[%s]", ll_cc);
	*name_ll = name_key_ll;
	*name_ll_cc = name_key_ll_cc;
}

void lang_localized_gname_key(char **gname_ll, char **gname_ll_cc)
{
	char *ll, *ll_cc;

	lang_code(&ll, &ll_cc);
	snprintf(generic_name_key_ll, sizeof(generic_name_key_ll),
		 "GenericName[%s]", ll);
	snprintf(generic_name_key_ll_cc, sizeof(generic_name_key_ll_cc),
		 "GenericName[%s]", ll_cc);
	*gname_ll = generic_name_key_ll;
	*gname_ll_cc = generic_name_key_ll_cc;
}
