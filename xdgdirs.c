/*
 * xdgdirs.c
 *
 * Simply provides a few linked list with "XDG" directories
 *
 * http://specifications.freedesktop.org/basedir-spec...
 *
 * $XDG_DATA_HOME
 * $HOME/.local/share/
 * $XDG_DATA_DIRS
 * /usr/local/share
 * /usr/share
 *
 */

#include <stdio.h>

#include "xdgdirs.h"
#include "sbuf.h"
#include "list.h"

/*
 * struct list_head xdgdirs_themes;
 * struct list_head xdgdirs_basedirs;
 * struct list_head xdgdirs_datadirs;
 */

void xdgdirs_init(void)
{
	/* if <list> != NULL, return */
}
