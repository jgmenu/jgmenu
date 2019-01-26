/*
 * Simple argv style field parser implementation
 *
 * Copyright (C) Johan Malm 2017
 *
 *   - buf points to the beginning of the buffer
 *   - argv[0..MAX_FIELDS-1] point to each 'delim' separated field
 *
 * It understands the following syntaxes:
 *
 * 1. a,b,c where no fields contain commas.
 *    This covers most cases.
 *
 * 2. a,"""b""",c where b can contain commas but not triple quotes.
 *    This is useful for scripting (paricularly for pipes) where commas and quotes
 *    are used.
 */

#ifndef ARGV_BUF_H
#define ARGV_BUF_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FIELDS (16)

struct argv_buf {
	char *buf;
	int argc;
	char *argv[MAX_FIELDS];
	char delim;
};

void argv_init(struct argv_buf *buf);
void argv_set_delim(struct argv_buf *buf, char delim);
void argv_strdup(struct argv_buf *buf, const char *s);
void argv_parse(struct argv_buf *buf);
void argv_free(struct argv_buf *buf);

#endif /* ARGV_BUF_H */
