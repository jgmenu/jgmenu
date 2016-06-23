#include "sbuf.h"

void sbuf_init(struct String *s)
{
	s->buf = malloc(1);
	s->buf[0] = 0;
	s->bufsiz = 1;
	s->len = 0;
}

void sbuf_addch(struct String *s, char ch)
{
	if (s->bufsiz <= s->len + 1) {
		s->bufsiz = s->bufsiz * 2 + 16;
		s->buf = realloc(s->buf, s->bufsiz);
	}
	s->buf[s->len++] = ch;
	s->buf[s->len] = 0;
}

void sbuf_addstr(struct String *s, const char *data)
{
	size_t len = strlen(data);

	if (s->bufsiz <= s->len + len + 1) {
		s->bufsiz = s->bufsiz + len;
		s->buf = realloc(s->buf, s->bufsiz);
	}
	memcpy(s->buf + s->len, data, len);
	s->len += len;
	s->buf[s->len] = 0;
}

void sbuf_cpy(struct String *s, const char *data)
{
	s->len = 0;
	sbuf_addstr(s, data);
}

void sbuf_list_append(struct list_head *sl, const char *data)
{
	struct String *new_string;

	new_string = malloc(sizeof(struct String));
	sbuf_init(new_string);
	sbuf_addstr(new_string, data);

	list_add_tail(&(new_string->list), sl);
}
