#ifndef TERMINAL_H
#define TERMINAL_H

#include "sbuf.h"

/**
 * term_build_terminal_cmd - build string to launch program in terminal
 * @termcmd: the return string
 * @cmd: program/command to be run in the terminal
 * @term: terminal (e.g. xterm)
 * @args: terminal args; the last of which must be '-e' or equivalent
 *        as it will be followed by the program/command to be run
 */
void term_build_terminal_cmd(struct sbuf *termcmd, const char *cmd,
			     const char *term, const char *args);

#endif /* TERMINAL_H */
