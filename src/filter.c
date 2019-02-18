#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filter.h"
#include "sbuf.h"
#include "util.h"
#include "argv-buf.h"

static struct sbuf needle;
static int has_been_inited;
static int clear_on_keyboard_input;

void filter_init(void)
{
	sbuf_init(&needle);
	has_been_inited = 1;
}

char *filter_strdup_needle(void)
{
	return xstrdup(needle.buf);
}

/**
 * filter_clear_on_keyboard_input - should we clear needle on key stroke?
 * Note this handles relationship bewteen type-to-search and ^filter()
 */
int filter_get_clear_on_keyboard_input(void)
{
	return clear_on_keyboard_input;
}

void filter_set_clear_on_keyboard_input(int clear)
{
	clear_on_keyboard_input = clear;
}

void filter_addstr(const char *str, size_t n)
{
	size_t i;

	BUG_ON(!has_been_inited);
	/* Byte-by-byte to handle XKeyEvent buf properly */
	for (i = 0; i < n; i++)
		sbuf_addch(&needle, str[i]);
}

/* byte1 refers to the first byte in a UTF-8 sequence of 1-4 bytes */
static int utf8_is_byte1(unsigned char c)
{
	/* When only 1 byte is allocated, we should see 0xxxxxx */
	if (!(c & 0x80))
		return 1;

	/*
	 * In the case of a 2-4 byte UTF-8 sequence, the bytes will take the
	 * following format
	 *   - first byte: 11xxxxx
	 *   - any other:  10xxxxx
	 */
	return (c & 0xc0) != 0x80;
}

void filter_backspace(void)
{
	int byte1 = 0;

	if (!has_been_inited)
		die("filter has not been initiated");
	if (!needle.len)
		return;
	needle.len -= 1;
	if (utf8_is_byte1(needle.buf[needle.len]))
		byte1 = 1;
	needle.buf[needle.len] = '\0';
	if (byte1 || !needle.len)
		return;
	/* keep deleting if it's not byte1 of a UTF-8 sequence */
	filter_backspace();
}

void filter_reset(void)
{
	BUG_ON(!has_been_inited);
	filter_set_clear_on_keyboard_input(0);
	sbuf_cpy(&needle, "");
}

int filter_needle_length(void)
{
	return needle.len;
}

int filter_ismatch(const char *haystack)
{
	struct argv_buf a;
	int i, ret = 1;

	if (!haystack)
		return 0;
	if (!needle.len)
		return 1;
	argv_init(&a);
	argv_set_delim(&a, ' ');
	argv_strdup(&a, needle.buf);
	argv_parse(&a);
	for (i = 0; i < a.argc; i++) {
		if (a.argv[i][0] == '\0')
			continue;
		if (!strcmp(a.argv[i], "*"))
			goto out;
		if (strcasestr(haystack, a.argv[i]))
			goto out;
	}
	ret = 0;
out:
	argv_free(&a);
	return ret;
}

void filter_cleanup(void)
{
	xfree(needle.buf);
}
