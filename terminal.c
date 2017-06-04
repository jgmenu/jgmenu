#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "isprog.h"
#include "terminal.h"

static char *terminals[] = { "x-terminal-emulator", "terminator", "uxterm",
			     "xterm", "gnome-terminal", "lxterminal",
			     "qterminal", "uxterm", "urxvt", "rxvt",
			     "xfce4-terminal", "konsole", NULL };

static void strdup_terminal_from_array(char **term)
{
	int i;

	for (i = 0; terminals[i]; i++) {
		if (is_prog(terminals[i])) {
			*term = strdup(terminals[i]);
			break;
		}
	}
}

void term_build_terminal_cmd(struct sbuf *termcmd, const char *cmd,
			     const char *term, const char *args)
{
	char *terminal = NULL, *arguments = NULL;

	if (!termcmd->buf)
		warn("string buffer not initiated");
	if (!term || !term[0])
		strdup_terminal_from_array(&terminal);
	if (!terminal)
		fprintf(stderr, "warning: cannot find terminal\n");

	/* most use '-e' */
	if (!args || !args[0])
		arguments = strdup("-e");

	sbuf_cpy(termcmd, terminal);
	sbuf_addstr(termcmd, " ");
	sbuf_addstr(termcmd, arguments);
	sbuf_addstr(termcmd, " '");
	sbuf_addstr(termcmd, cmd);
	sbuf_addstr(termcmd, "'");
}
