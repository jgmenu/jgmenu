/* Find BunsenLabs Linux tint2 session file */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bl.h"
#include "util.h"
#include "banned.h"

#define TINT2SESSIONFILE "~/.config/tint2/tint2-sessionfile"

void bl_tint2file(struct sbuf *tint2_file)
{
	struct sbuf blfile;
	FILE *fp;
	char line[2048], *p;

	sbuf_init(&blfile);
	sbuf_addstr(&blfile, TINT2SESSIONFILE);
	sbuf_expand_tilde(&blfile);
	fp = fopen(blfile.buf, "r");
	if (!fp)
		goto out;
	if (!fgets(line, sizeof(line), fp))
		goto out;
	p = strchr(line, '\n');
	if (p)
		*p = '\0';
	sbuf_cpy(tint2_file, line);
out:
	if (fp)
		fclose(fp);
	free(blfile.buf);
}
