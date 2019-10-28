#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "util.h"
#include "sbuf.h"
#include "banned.h"

static int info_muted;

void mute_info(void)
{
	info_muted = 1;
}

void info(const char *err, ...)
{
	va_list params;

	if (info_muted)
		return;
	fprintf(stderr, "info: ");
	va_start(params, err);
	vfprintf(stderr, err, params);
	va_end(params);
	fprintf(stderr, "\n");
}

void warn(const char *err, ...)
{
	va_list params;

	fprintf(stderr, "warning: ");
	va_start(params, err);
	vfprintf(stderr, err, params);
	va_end(params);
	fprintf(stderr, "\n");
}

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

void safe_free(void **ptr)
{
	if (!ptr || !*ptr)
		return;
	free(*ptr);
	*ptr = NULL;
}

char *xstrdup(const char *s)
{
	char *ret = NULL;

	if (!s)
		return NULL;
	ret = strdup(s);
	if (!ret)
		die("Out of memory; strdup failed");
	return ret;
}

void *xmalloc(size_t size)
{
	void *ret = malloc(size);

	if (!ret)
		die("Out of memory; malloc failed");
	return ret;
}

void *xrealloc(void *ptr, size_t size)
{
	void *ret = realloc(ptr, size);

	if (!ret)
		die("Out of memory; realloc failed");
	return ret;
}

void *xcalloc(size_t nb, size_t size)
{
	void *ret = calloc(nb, size);

	if (!ret)
		die("Out of memory; calloc failed");
	return ret;
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
	int iscomment = 0;

	p = line;
	while ((p[0] == ' ') || (p[0] == '\t'))
		p++;
	if (p[0] == '\n')
		return 0;
	if (p[0] == '#')
		iscomment = 1;
	p = strchr(line, '=');
	if (!p)
		return 0;
	p[0] = '\0';
	*option = strstrip(line);
	if (*option[0] == '#')
		++(*option);
	*value  = strstrip(++p);
	p = strchr(p, '\n');
	if (p)
		p[0] = '\0';
	return iscomment ? 0 : 1;
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
	if (!hex || hex[0] != '#' || strlen(hex) < 7)
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
	int i = 0, num = 0, has_found_number = 0;

	if (!s)
		return 0;

	for (;;) {
		unsigned char c = s[i++];

		if (!c)
			return num;

		if (!has_found_number && isdigit(c)) {
			num = c - '0';
			has_found_number = 1;
			continue;
		}

		if (has_found_number && isdigit(c))
			num = 10 * num + c - '0';

		if (has_found_number && !isdigit(c))
			return num;
	}
}

static void xatoi_warn(const char *msg, const char *val, const char *key)
{
	BUG_ON(!msg || !val || !key);
	fprintf(stderr, "warning: %s; value='%s'; key='%s'\n", msg, val, key);
}

/*
 * xatoi() is an alternative to 'var = atoi(value)'. It was inspired by
 * http://man7.org/tlpi/code/online/dist/lib/get_num.c.html
 *
 * Variable 'var' is only touched if 'value' is a valid integer.
 *
 * The variable 'key' is only there for debugging purposes; it would typically
 * contain the variable name;
 *
 * The flags are defined in the header-file (XATOI_*)
 */
void xatoi(int *var, const char *value, int flags, const char *key)
{
	long res;
	char *endptr;

	if (!value || *value == '\0') {
		xatoi_warn("null or empty string", "", key);
		return;
	}

	errno = 0;
	res = strtol(value, &endptr, 10);
	if (errno != 0)
		xatoi_warn("strtol() failed", value, key);
	else if (*endptr != '\0')
		xatoi_warn("nonnumeric characters", value, key);
	else if ((flags & XATOI_NONNEG) && res < 0)
		xatoi_warn("negative value not allowed", value, key);
	else if ((flags & XATOI_GT_0) && res <= 0)
		xatoi_warn("value must be > 0", value, key);
	else if (res > INT_MAX || res < INT_MIN)
		xatoi_warn("integer out of range", value, key);
	else
		*var = (int)res;
}

void cat(const char *filename)
{
	FILE *fp;
	char line[4096];
	struct sbuf f;

	sbuf_init(&f);
	sbuf_cpy(&f, filename);
	sbuf_expand_tilde(&f);
	fp = fopen(f.buf, "r");
	if (!fp)
		goto cleanup;
	while (fgets(line, sizeof(line), fp))
		printf("%s", line);
	printf("\n");
	fclose(fp);
cleanup:
	xfree(f.buf);
}

void remove_caret_markup_closing_bracket(char *s)
{
	char *q;

	if (!s)
		return;
	if (s[0] == '^') {
		q = strrchr(s, ')');
		if (q)
			*q = '\0';
	}
}

void mkdir_p(const char *path)
{
	struct sbuf s;
	char *p;

	sbuf_init(&s);
	if (strlen(path) > PATH_MAX) {
		warn("mkdir_p: path too long");
		return;
	}
	sbuf_cpy(&s, path);
	if (!s.len)
		return;
	sbuf_expand_tilde(&s);
	if (s.buf[s.len - 1] != '/')
		sbuf_addch(&s, '/');
	for (p = s.buf + 1; *p; p++) {
		if (*p != '/')
			continue;
		*p = '\0';
		if (mkdir(s.buf, S_IRWXU) != 0 && errno != EEXIST)
			warn("mkdir_p: could not create '%s'", s.buf);
		*p = '/';
	}
}

void msleep(unsigned int duration)
{
	struct timespec ts;
	unsigned int sec = duration / 1000;
	unsigned int msec = duration % 1000;

	ts.tv_sec  = sec;
	ts.tv_nsec =  msec * 1000000;
	nanosleep(&ts, NULL);
}
