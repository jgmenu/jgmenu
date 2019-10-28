#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "isprog.h"
#include "terminal.h"
#include "banned.h"

static char *terminals[] = { "x-terminal-emulator", "terminator", "uxterm",
			     "xterm", "gnome-terminal", "lxterminal",
			     "qterminal", "uxterm", "urxvt", "rxvt",
			     "xfce4-terminal", "konsole", "sakura", "st",
			     NULL };

static void strdup_terminal_from_array(char **term)
{
	int i;

	for (i = 0; terminals[i]; i++) {
		if (isprog(terminals[i])) {
			*term = xstrdup(terminals[i]);
			break;
		}
	}
}

void term_build_terminal_cmd(struct sbuf *termcmd, const char *cmd,
			     const char *term, const char *args)
{
	char *terminal = NULL;

	if (!termcmd->buf)
		die("string buffer not initiated");
	if (!args || !args[0])
		warn("terminal_args must be set");
	if (term && term[0] && isprog(term))
		terminal = xstrdup(term);
	else
		strdup_terminal_from_array(&terminal);
	if (!terminal)
		fprintf(stderr, "warning: cannot find terminal\n");

	sbuf_cpy(termcmd, terminal);
	sbuf_addstr(termcmd, " ");
	sbuf_addstr(termcmd, args);
	sbuf_addstr(termcmd, " '");
	sbuf_addstr(termcmd, cmd);
	sbuf_addstr(termcmd, "'");
	xfree(terminal);
}
