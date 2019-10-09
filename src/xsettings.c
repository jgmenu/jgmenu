/*
 * Copyright © 2001 Red Hat, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Red Hat not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  Red Hat makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * RED HAT DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING ALL
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL RED HAT
 * BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Author:  Owen Taylor, Red Hat, Inc.
 */

#include "xsettings.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

struct xsettings_buffer {
	char byte_order;
	size_t len;
	unsigned char *data;
	unsigned char *pos;
};

#define XSETTINGS_PAD(n, m) ((n + m - 1) & (~(m - 1)))
#define BYTES_LEFT(buffer) ((buffer)->data + (buffer)->len - (buffer)->pos)

static char byte_order(void)
{
	CARD32 myint = 0x01020304;

	return (*(char *)&myint == 1) ? MSBFirst : LSBFirst;
}

static int ignore_x11_errors(Display *display, XErrorEvent *event)
{
	return True;
}

static Bool fetch_card8(struct xsettings_buffer *buffer, CARD8 *result)
{
	if (BYTES_LEFT(buffer) < 1)
		return False;
	*result = *(CARD8 *)buffer->pos;
	buffer->pos += 1;
	return True;
}

static Bool fetch_card16(struct xsettings_buffer *buffer, CARD16 *result)
{
	CARD16 x;

	if (BYTES_LEFT(buffer) < 2)
		return False;
	x = *(CARD16 *)buffer->pos;
	buffer->pos += 2;
	if (buffer->byte_order == byte_order())
		*result = x;
	else
		*result = (x << 8) | (x >> 8);
	return True;
}

static Bool fetch_ushort(struct xsettings_buffer *buffer, unsigned short *result)
{
	CARD16 x;

	if (!fetch_card16(buffer, &x))
		return False;
	*result = x;
		return True;
}

static Bool fetch_card32(struct xsettings_buffer *buffer, CARD32 *result)
{
	CARD32 x;

	if (BYTES_LEFT(buffer) < 4)
		return False;

	x = *(CARD32 *)buffer->pos;

	buffer->pos += 4;
	if (buffer->byte_order == byte_order())
		*result = x;
	else
		*result = (x << 24) | ((x & 0xff00) << 8) |
			  ((x & 0xff0000) >> 8) | (x >> 24);
	return True;
}

#define XSETTINGS_PAD(n, m) ((n + m - 1) & (~(m - 1)))

static struct xsetting *parse_settings(unsigned char *data, size_t len, size_t *count)
{
	struct xsetting *result = NULL;
	struct xsettings_buffer buffer;
	CARD32 serial;
	CARD32 n_entries;
	size_t i;

	*count = 0;
	buffer.pos = data;
	buffer.data = data;
	buffer.len = len;
	buffer.byte_order = byte_order();

	if (!fetch_card8(&buffer, (CARD8 *)&buffer.byte_order))
		goto err;
	buffer.pos += 3;
	if (buffer.byte_order != MSBFirst && buffer.byte_order != LSBFirst) {
		fprintf(stderr, "Invalid byte order %x in XSETTINGS property\n",
			buffer.byte_order);
		goto err;
	}

	if (!fetch_card32(&buffer, &serial))
		goto err;

	if (!fetch_card32(&buffer, &n_entries) || !n_entries)
		goto err;

	result = calloc(n_entries, sizeof(struct xsetting));
	if (!result)
		goto err;
	*count = n_entries;

	for (i = 0; i < n_entries; i++) {
		CARD16 name_len;
		int pad_len;
		struct xsetting *setting;

		setting = &result[i];

		if (!fetch_card8(&buffer, &setting->type))
			goto err;
		buffer.pos += 1;

		if (!fetch_card16(&buffer, &name_len))
			goto err;

		pad_len = XSETTINGS_PAD(name_len, 4);

		if (BYTES_LEFT(&buffer) < pad_len)
			goto err;

		setting->name = calloc(name_len + 1, 1);
		if (!setting->name)
			goto err;
		memcpy(setting->name, buffer.pos, name_len);
		buffer.pos += pad_len;

		if (!fetch_card32(&buffer, &setting->serial))
			goto err;

		if (setting->type == XSETTINGS_TYPE_INT) {
			CARD32 v_int;

			if (!fetch_card32(&buffer, &v_int))
				goto err;
			setting->value.int_value = (INT32)v_int;
		} else if (setting->type == XSETTINGS_TYPE_STRING) {
			CARD32 v_int;

			if (!fetch_card32(&buffer, &v_int))
				goto err;

			pad_len = XSETTINGS_PAD(v_int, 4);
			if (v_int + 1 == 0 || BYTES_LEFT(&buffer) < pad_len)
				goto err;

			setting->value.string_value = calloc(v_int + 1, 1);
			if (!setting->value.string_value)
				goto err;
			memcpy(setting->value.string_value, buffer.pos, v_int);
			setting->value.string_value[v_int] = '\0';
			buffer.pos += pad_len;
		} else if (setting->type == XSETTINGS_TYPE_COLOR) {
			if (!fetch_ushort(&buffer, &setting->value.color_value.red))
				goto err;
			if (!fetch_ushort(&buffer, &setting->value.color_value.green))
				goto err;
			if (!fetch_ushort(&buffer, &setting->value.color_value.blue))
				goto err;
			if (!fetch_ushort(&buffer, &setting->value.color_value.alpha))
				goto err;
		} else {
			goto err;
		}
	}
	return result;
err:
	free_xsettings(result, *count);
	return NULL;
}

void free_xsettings(struct xsetting *settings, size_t count)
{
	size_t i;

	if (!settings)
		return;
	for (i = 0; i < count; i++) {
		struct xsetting *setting = &settings[i];

		if (setting->type == XSETTINGS_TYPE_STRING)
			free(setting->value.string_value);
		free(setting->name);
	}
	free(settings);
}

static Window get_xsettings_manager(Display *display)
{
	char screen_name[256];
	Atom _XSETTINGS_SCREEN;

	snprintf(screen_name, sizeof(screen_name),
		 "_XSETTINGS_S%d", DefaultScreen(display));
	_XSETTINGS_SCREEN = XInternAtom(display, screen_name, False);
	return XGetSelectionOwner(display, _XSETTINGS_SCREEN);
}

static unsigned char *read_xsettings(Display *display, Window manager,
				     unsigned long *total_size)
{
	int (*old_handler)(Display *, XErrorEvent *);
	Atom _XSETTINGS_SETTINGS;
	Atom type;
	int format;
	unsigned char *buffer;
	unsigned long buffer_size;
	unsigned long bytes_after;
	unsigned char *result = NULL;
	Bool done = False;
	Bool failed = False;

	old_handler = XSetErrorHandler(ignore_x11_errors);
	_XSETTINGS_SETTINGS = XInternAtom(display, "_XSETTINGS_SETTINGS", False);

	*total_size = 0;
	while (!done && !failed) {
		int status = XGetWindowProperty(display, manager, _XSETTINGS_SETTINGS, *total_size,
						LONG_MAX, False, _XSETTINGS_SETTINGS, &type, &format,
						&buffer_size, &bytes_after, &buffer);
		if (status == Success && type == _XSETTINGS_SETTINGS) {
			if (format != 8) {
				fprintf(stderr, "Invalid format for XSETTINGS property %d\n", format);
				failed = True;
			} else {
				if (buffer_size > 0) {
					unsigned long offset = *total_size;
					*total_size += buffer_size;
					result = xrealloc(result, *total_size);
					memcpy(result + offset, buffer, buffer_size);
					if (bytes_after == 0)
						done = True;
				} else {
				  failed = True;
				}
			}
			XFree(buffer);
		} else {
			fprintf(stderr, "Could not read XSETTINGS\n");
			failed = True;
		}
	}
	XSetErrorHandler(old_handler);
	if (failed) {
		free(result);
		result = NULL;
		*total_size = 0;
	}
	return result;
}

struct xsetting *get_xsettings(Display *display, size_t *count)
{
	Window manager = get_xsettings_manager(display);
	unsigned long size;
	unsigned char *buffer;
	struct xsetting *result;

	/* no xsettings daemon found */
	if (!manager)
		return NULL;

	buffer = read_xsettings(display, manager, &size);
	if (!buffer || !size) {
		fprintf(stderr, "warning: could not read xsettings\n");
		return NULL;
	}
	result = parse_settings(buffer, size, count);
	free(buffer);
	return result;
}
