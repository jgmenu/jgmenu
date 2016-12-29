#include "sbuf.h"

void sbuf_init(struct sbuf *s)
{
	s->buf = xmalloc(1);
	s->buf[0] = 0;
	s->bufsiz = 1;
	s->len = 0;
}

void sbuf_addch(struct sbuf *s, char ch)
{
	if (s->bufsiz <= s->len + 1) {
		s->bufsiz = s->bufsiz * 2 + 16;
		s->buf = xrealloc(s->buf, s->bufsiz);
	}
	s->buf[s->len++] = ch;
	s->buf[s->len] = 0;
}

void sbuf_addstr(struct sbuf *s, const char *data)
{
	size_t len = strlen(data);

	if (s->bufsiz <= s->len + len + 1) {
		s->bufsiz = s->bufsiz + len;
		s->buf = xrealloc(s->buf, s->bufsiz);
	}
	memcpy(s->buf + s->len, data, len);
	s->len += len;
	s->buf[s->len] = 0;
}

void sbuf_cpy(struct sbuf *s, const char *data)
{
	s->len = 0;
	sbuf_addstr(s, data);
}

void sbuf_prepend(struct sbuf *s, const char *data)
{
	struct sbuf tmp;

	sbuf_init(&tmp);
	sbuf_addstr(&tmp, s->buf);
	sbuf_cpy(s, data);
	sbuf_addstr(s, tmp.buf);
	free(tmp.buf);
}

void sbuf_shift_left(struct sbuf *s, int n_bytes)
{
	char *data;

	data = strdup(s->buf);

	memcpy(s->buf, data + n_bytes, s->len - n_bytes);
	s->len -= n_bytes;
	s->buf[s->len] = 0;

	free(data);
}

void sbuf_split(struct list_head *sl, const char *data, char field_separator)
{
	char *p, *str;

	if (!data)
		return;

	str = strdup(data);

	for (;;) {
		p = strrchr(str, field_separator);
		if (!p)
			break;
		sbuf_list_prepend(sl, p + 1);
		*p = '\0';
	}
	sbuf_list_prepend(sl, str);
	free(str);
}

void sbuf_list_append(struct list_head *sl, const char *data)
{
	struct sbuf *new_string;

	new_string = xmalloc(sizeof(struct sbuf));
	sbuf_init(new_string);
	sbuf_addstr(new_string, data);

	list_add_tail(&new_string->list, sl);
}

void sbuf_list_prepend(struct list_head *sl, const char *data)
{
	struct sbuf *new_string;

	new_string = xmalloc(sizeof(struct sbuf));
	sbuf_init(new_string);
	sbuf_addstr(new_string, data);

	list_add(&new_string->list, sl);
}
