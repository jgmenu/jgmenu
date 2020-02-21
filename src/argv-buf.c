#include "argv-buf.h"
#include "util.h"
#include "banned.h"

void argv_init(struct argv_buf *buf)
{
	int i;

	buf->argc = 0;
	buf->buf = NULL;
	for (i = 0; i < MAX_FIELDS; i++)
		buf->argv[i] = NULL;
}

void argv_set_delim(struct argv_buf *buf, char delim)
{
	buf->delim = delim;
}

void argv_strdup(struct argv_buf *buf, const char *s)
{
	buf->buf = strdup(s);
	buf->argv[0] = buf->buf;
	buf->argc = 1;
}

static int is_triple_quote(char **s)
{
	if (!s || !*s)
		return 0;
	if (**s == '\0' || *(*s + 1) == '\0' || *(*s + 2) == '\0')
		return 0;
	return ((*s)[0] == '"' && (*s)[1] == '"' && (*s)[2] == '"');
}

static void traverse_triple_quote(char **s)
{
	(*s) += 3;
	while (!is_triple_quote(s)) {
		if (**s == '\0')
			die("no closing triple quote");
		(*s)++;
	}
	(*s) += 3;
}

static void rm_next_triple_quote(char **s)
{
	while (!is_triple_quote(s)) {
		if (**s == '\0')
			die("no closing triple quote");
		(*s)++;
	}
	**s = '\0';
	(*s) += 3;
}

static char *next(char *str, char delim)
{
	char *tmp;

	if (is_triple_quote(&str))
		traverse_triple_quote(&str);
	tmp = strchr(str, delim);
	if (tmp)
		tmp++;
	return tmp;
}

static void trim_all_fields(struct argv_buf *buf)
{
	int i;
	char *p;

	for (i = 0; i < buf->argc; i++) {
		p = buf->argv[i];
		rtrim(&p);
		while (isspace(*p))
			p++;
		buf->argv[i] = p;
	}
}

void argv_parse(struct argv_buf *buf)
{
	char *q;
	int j;

	if (!buf->argv[0]) {
		warn("cannot parse empty argv buffer");
		return;
	}
	for (j = 0; j < MAX_FIELDS - 1; j++) {
		if (!buf->argv[j])
			break;
		buf->argv[j + 1] = next(buf->argv[j], buf->delim);
		if (buf->argv[j + 1])
			buf->argc++;
	}

	/* Change delims to '\0' */
	for (j = buf->argc - 1; j >= 0; j--) {
		q = buf->argv[j];
		while (*q != '\0') {
			if (is_triple_quote(&q)) {
				buf->argv[j] += 3;
				q += 3;
				rm_next_triple_quote(&q);
			}
			if (*q == '\0')
				break;
			if (*q == buf->delim) {
				*q = '\0';
				break;
			}
			q++;
		}
	}
	trim_all_fields(buf);
}

void argv_free(struct argv_buf *buf)
{
	if (!buf)
		return;
	if (buf->buf)
		free(buf->buf);
}
