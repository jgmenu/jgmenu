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

	tmp = xmalloc(strlen(s) + strlen(getenv("HOME")) + 1);
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

int hex_to_dec(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return 0;
}

int parse_hexstr(char *hex, double *rgba)
{
	if (hex == NULL || hex[0] != '#' || strlen(hex) < 7)
		return 0;

	rgba[0] = (hex_to_dec(hex[1]) * 16 + hex_to_dec(hex[2])) / 255.0;
	rgba[1] = (hex_to_dec(hex[3]) * 16 + hex_to_dec(hex[4])) / 255.0;
	rgba[2] = (hex_to_dec(hex[5]) * 16 + hex_to_dec(hex[6])) / 255.0;

	if (strlen(hex) > 7)
		rgba[3] = atoi(hex + 7) / 100.0;
	else
		rgba[3] = 1.0;

	return 1;
}

int get_first_num_from_str(const char *s)
{
	char *str, *a, *b;
	int num;

	str = strdup(s);
	a = str;

	while (!isdigit(*a) && a != '\0')
		++a;

	b = a;
	while (isdigit(*b))
		++b;

	*b = '\0';
	num = atoi(a);
	free(str);

	return num;
}
