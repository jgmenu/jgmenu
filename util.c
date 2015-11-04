#include "util.h"

void die(const char *err, ...)
{
	va_list params;

	fputs("fatal: ", stderr);
	va_start(params, err);
	vfprintf(stderr, err, params);
	va_end(params);
	fputc('\n', stderr);

	exit(1);
}


void spawn(const char *arg)
{
	const char *shell = NULL;

	shell = getenv("SHELL");
	if (!shell)
		shell = "/bin/sh";
	if (!arg)
		return;

	/*
	 * Double-fork idea from dzen2 (https://github.com/robm/dzen)
	 * to avoid jgmenu being the parent of the new process.
	 */
	if (fork() == 0) {
		if (fork() == 0) {
			setsid();
			execl(shell, shell, "-c", arg, (char *)NULL);
		}
		exit(0);
	}
}


void *xmalloc(size_t size)
{
	void *ret = malloc(size);

	if (!ret)
		die("Out of memory, malloc failed");
	return ret;
}

void *xrealloc(void *ptr, size_t size)
{
	void *ret = realloc(ptr, size);

	if (!ret)
		die("Out of memory, realloc failed");
	return ret;
}

void *xcalloc(size_t nb, size_t size)
{
	void *ret = calloc(nb, size);

	if (!ret)
		die("Out of memory, calloc failed");
	return ret;
}


char *expand_tilde(char *s)
{
	char *tmp;

	tmp = (char *)malloc(strlen(s) + strlen(getenv("HOME")) + 1);
	strcpy(tmp, getenv("HOME"));
	strcat(tmp, s + 1);

	free(s);
	s = NULL;

	return tmp;
}

char *strstrip(char *s)
{
	size_t len;
	char *end;

	len = strlen(s);
	if (!len)
		return s;

	end = s + len - 1;
	while (end >= s && isspace(*end))
		end--;
	*(end + 1) = '\0';

	while (isspace(*s))
		s++;

	return s;
}


int parse_config_line(char *line, char **option, char **value)
{
	char *p;

	p = line;
	while ((p[0] == ' ') || (p[0] == '\t'))
		p++;
	if ((p[0] == '#') || (p[0] == '\n'))
		return 0;

	p = strchr(line, '=');
	if (!p)
		return 0;
	p[0] = 0;

	*option = strstrip(line);
	*value  = strstrip(++p);

	p = strchr(p, '\n');
	if (p)
		p[0] = 0;

	return 1;
}

