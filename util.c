#include "util.h"

void die(const char *err, ...)
{
	va_list params;

	fputs("fatal: ",stderr);
	va_start(params, err);
	vfprintf(stderr, err, params);
	va_end(params);
	fputc('\n',stderr);

	exit(1);
}


/*
 * spawn() is copied from dzen2 (https://github.com/robm/dzen)
 *
 * (C)opyright MMVI-MMVII Anselm R. Garbe <garbeam at gmail dot com>
 * (C)opyright MMVII Robert Manea <rob dot manea  at gmail dot com>
 * See LICENSE file for license details (MIT/X Consortium License)
 */
void spawn(const char *arg)
{
	static const char *shell = NULL;

	if(!shell && !(shell = getenv("SHELL")))
		shell = "/bin/sh";
	if(!arg)
		return;
	/* The double-fork construct avoids zombie processes and keeps the code
	 * clean from stupid signal handlers. */
	if(fork() == 0) {
		if(fork() == 0) {
			setsid();
			execl(shell, shell, "-c", arg, (char *)NULL);
			fprintf(stderr, "execl '%s -c %s'", shell, arg);
		}
		exit(0);
	}
}
