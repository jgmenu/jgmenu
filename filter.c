#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "filter.h"
#include "sbuf.h"
#include "util.h"

static struct sbuf needle;
static int has_been_inited;

void filter_init(void)
{
	sbuf_init(&needle);
	has_been_inited = 1;
}

char *filter_strdup_needle(void)
{
	return xstrdup(needle.buf);
}

void filter_addstr(const char *str, size_t n)
{
	size_t i;

	if (!has_been_inited)
		die("filter has not been initiated");

	for (i = 0; i < n; i++)
		sbuf_addch(&needle, str[i]);

	printf("%s\n", needle.buf);
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
		goto out;
	needle.len -= 1;
	if (utf8_is_byte1(needle.buf[needle.len]))
		byte1 = 1;
	needle.buf[needle.len] = '\0';
	if (byte1 || !needle.len)
		goto out;
	/* keep deleting if it's not byte1 of a UTF-8 sequence */
	filter_backspace();
out:
	if (byte1)
		printf("%s\n", needle.buf);
}

void filter_reset(void)
{
	if (!has_been_inited)
		die("filter has not been initiated");
	sbuf_cpy(&needle, "");
}

int filter_needle_length(void)
{
	return needle.len;
}

int filter_ismatch(const char *haystack)
{
	int ret = 0;

	if (!needle.len || strcasestr(haystack, needle.buf))
		ret = 1;

	return ret;
}

void filter_cleanup(void)
{
	xfree(needle.buf);
}
