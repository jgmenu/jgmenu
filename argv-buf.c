#include "argv-buf.h"

void argv_init(struct argv_buf *buf)
{
	int i;

	buf->argc = 0;
	for (i = 0; i < MAX_FIELDS; i++)
		buf->argv[i] = NULL;
}

void argv_set_delim(struct argv_buf *buf, char delim)
{
	buf->delim = delim;
}

void argv_strdup(struct argv_buf *buf, const char *s)
{
	buf->argv[0] = strdup(s);
	buf->argc = 1;
}

static char *next(char *str, char delim)
{
	char *tmp;

	tmp = strchr(str, delim);
	if (tmp)
		tmp++;
	return tmp;
}

void argv_parse(struct argv_buf *buf)
{
	char *q;
	int j;

	for (j = 0; j < MAX_FIELDS - 1; j++) {
		if (!buf->argv[j])
			break;
		buf->argv[j + 1] = next(buf->argv[j], buf->delim);
		if (buf->argv[j + 1])
			buf->argc++;
	}
	while ((q = strrchr(buf->argv[0], buf->delim)))
		*q = '\0';
}

void argv_free(struct argv_buf *buf)
{
	if (!buf)
		return;
	if (buf->argv[0])
		free(buf->argv[0]);
}
