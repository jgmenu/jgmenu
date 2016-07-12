/*
 * Simple C string buffer implementatioun.
 *
 * Copyright Johan Malm 2016
 *
 * The buffer is a byte array which is at least (len + 1) bytes in size.
 * The sbuf functions are designed too maintain/add '\0' at the end,
 * allowing the buffer to be used be used as a C string
 * (i.e. buf[len] == 0).
 *
 * sbuf_init allocates one byte so that the buffer can always be safely:
 *	- assumed to be a valid C string (i.e. buf != NULL)
 *	- freed (i.e. buf must not to point to memory on the stack)
 *
 * Exampel life cycle:
 *	struct String s;
 *	sbuf_init(&s);
 *	sbuf_addch(&s, 'F');
 *	sbuf_addstr(&s, "oo");
 *	printf("%s\n", s.buf);
 *	free(s.buf);
 */

#ifndef SBUF_H
#define SBUF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"

struct String {
	char *buf;
	int bufsiz;
	int len;
	struct list_head list;
};

extern void sbuf_init(struct String *s);
extern void sbuf_addch(struct String *s, char ch);
extern void sbuf_addstr(struct String *s, const char *data);
extern void sbuf_cpy(struct String *s, const char *data);
extern void sbuf_prepend(struct String *s, const char *data);
extern void sbuf_shift_left(struct String *s, int n_bytes);
extern void sbuf_split(struct list_head *sl, const char *data, char field_separator);
extern void sbuf_list_append(struct list_head *sl, const char *data);
extern void sbuf_list_prepend(struct list_head *sl, const char *data);

#endif /* SBUF_H */
