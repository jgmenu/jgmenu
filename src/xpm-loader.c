/* Based on gdk-pixbuf/io-xpm.c
 * Copyright (C) 1999 Mark Crichton
 * Copyright (C) 1999 The Free Software Foundation
 *
 * Authors: Mark Crichton <crichton@gimp.org>
 *          Federico Mena-Quintero <federico@gimp.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>

#include "xpm-loader.h"
#include "util.h"

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b) ((a) < (b) ? (b) : (a))
#endif

#ifndef G_N_ELEMENTS
#define G_N_ELEMENTS(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

enum buf_op { op_header, op_cmap, op_body };

struct xpm_color {
	char *color_string;
	u_int16_t red;
	u_int16_t green;
	u_int16_t blue;
	int transparent;
};

struct file_handle {
	FILE *infile;
	char *buffer;
	uint buffer_size;
};

/* The following 2 routines (parse_color, find_color) come from Tk, via the Win32
 * port of GDK. The licensing terms on these (longer than the functions) is:
 *
 * This software is copyrighted by the Regents of the University of
 * California, Sun Microsystems, Inc., and other parties.  The following
 * terms apply to all files associated with the software unless explicitly
 * disclaimed in individual files.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions. No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 *
 * IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY PARTY
 * FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE, ITS DOCUMENTATION, OR ANY
 * DERIVATIVES THEREOF, EVEN IF THE AUTHORS HAVE BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.  THIS SOFTWARE
 * IS PROVIDED ON AN "AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS HAVE
 * NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR
 * MODIFICATIONS.
 *
 * GOVERNMENT USE: If you are acquiring this software on behalf of the
 * U.S. government, the Government shall have only "Restricted Rights"
 * in the software and related documentation as defined in the Federal
 * Acquisition Regulations (FARs) in Clause 52.227.19 (c) (2).  If you
 * are acquiring the software on behalf of the Department of Defense, the
 * software shall be classified as "Commercial Computer Software" and the
 * Government shall have only "Restricted Rights" as defined in Clause
 * 252.227-7013 (c) (1) of DFARs.  Notwithstanding the foregoing, the
 * authors grant the U.S. Government and others acting in its behalf
 * permission to use and distribute the software in accordance with the
 * terms specified in this license.
 */

#include "xpm-color-table.h"

/*
 *----------------------------------------------------------------------
 *
 * find_color --
 *
 *	This routine finds the color entry that corresponds to the
 *	specified color.
 *
 * Results:
 *	Returns non-zero on success.  The RGB values of the XColor
 *	will be initialized to the proper values on success.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int compare_xcolor_entries(const void *a, const void *b)
{
	return strcasecmp((const char *)a, color_names + ((const struct xpm_color_entry *)b)->name_offset);
}

static int find_color(const char *name, struct xpm_color *color_ptr)
{
	struct xpm_color_entry *found;

	found = bsearch(name, xcolors, G_N_ELEMENTS(xcolors), sizeof(struct xpm_color_entry), compare_xcolor_entries);
	if (!found)
		return 0;

	color_ptr->red = (found->red * 65535) / 255;
	color_ptr->green = (found->green * 65535) / 255;
	color_ptr->blue = (found->blue * 65535) / 255;

	return 1;
}

/*
 *----------------------------------------------------------------------
 *
 * parse_color --
 *
 *	Partial implementation of X color name parsing interface.
 *
 * Results:
 *	Returns 1 on success.
 *
 * Side effects:
 *	None.
 *
 *----------------------------------------------------------------------
 */

static int parse_color(const char *spec, struct xpm_color *color_ptr)
{
	if (spec[0] == '#') {
		int i;
		unsigned int red, green, blue;

		i = strlen(spec + 1);
		if (i % 3)
			return 0;
		i /= 3;

		if (i == 4) {
			if (sscanf(spec + 1, "%4x%4x%4x", &red, &green, &blue) != 3)
				return 0;
			color_ptr->red = red;
			color_ptr->green = green;
			color_ptr->blue = blue;
		} else if (i == 1) {
			if (sscanf(spec + 1, "%1x%1x%1x", &red, &green, &blue) != 3)
				return 0;
			color_ptr->red = (red * 65535) / 15;
			color_ptr->green = (green * 65535) / 15;
			color_ptr->blue = (blue * 65535) / 15;
		} else if (i == 2) {
			if (sscanf(spec + 1, "%2x%2x%2x", &red, &green, &blue) != 3)
				return 0;
			color_ptr->red = (red * 65535) / 255;
			color_ptr->green = (green * 65535) / 255;
			color_ptr->blue = (blue * 65535) / 255;
		} else /* if (i == 3) */ {
			if (sscanf(spec + 1, "%3x%3x%3x", &red, &green, &blue) != 3)
				return 0;
			color_ptr->red = (red * 65535) / 4095;
			color_ptr->green = (green * 65535) / 4095;
			color_ptr->blue = (blue * 65535) / 4095;
		}
	} else {
		if (!find_color(spec, color_ptr))
			return 0;
	}
	return 1;
}

static int xpm_seek_string(FILE *infile, const char *str)
{
	char instr[1024];

	while (!feof(infile)) {
		if (fscanf(infile, "%1023s", instr) < 0)
			return 0;
		if (strcmp(instr, str) == 0)
			return 1;
	}

	return 0;
}

static int xpm_seek_char(FILE *infile, char c)
{
	int b, oldb;

	while ((b = getc(infile)) != EOF) {
		if (c != b && b == '/') {
			b = getc(infile);
			if (b == EOF) {
				return 0;
			} else if (b == '*') {
				/* we have a comment */
				b = -1;
				do {
					oldb = b;
					b = getc(infile);
					if (b == EOF)
						return 0;
				} while (!(oldb == '*' && b == '/'));
			}
		} else if (c == b) {
			return 1;
		}
	}

	return 0;
}

static int xpm_read_string(FILE *infile, char **buffer, uint *buffer_size)
{
	int c;
	uint cnt = 0, bufsiz, ret = 0;
	char *buf;

	buf = *buffer;
	bufsiz = *buffer_size;
	if (!buf) {
		bufsiz = 10 * sizeof(char);
		buf = (char *)calloc(bufsiz, sizeof(char));
	}

	do {
		c = getc(infile);
	} while (c != EOF && c != '"');

	if (c != '"')
		goto out;

	while ((c = getc(infile)) != EOF) {
		if (cnt == bufsiz) {
			uint new_size = bufsiz * 2;

			if (new_size > bufsiz)
				bufsiz = new_size;
			else
				goto out;

			buf = (char *)xrealloc(buf, bufsiz);
			buf[bufsiz - 1] = '\0';
		}

		if (c != '"') {
			buf[cnt++] = c;
		} else {
			buf[cnt] = 0;
			ret = 1;
			break;
		}
	}

out:
	buf[bufsiz - 1] = '\0'; /* ensure null termination for errors */
	*buffer = buf;
	*buffer_size = bufsiz;
	return ret;
}

/* Unlike the standard C library isspace() function, this only recognizes standard ASCII white-space
 * and ignores the locale, returning FALSE for all non-ASCII characters.
 * Also, unlike the standard library function, this takes a char, not an int,
 * so don't call it on EOF, but no need to cast to unsigned char before passing a possibly non-ASCII character in.
 * Similar to g_ascii_isspace.
 */
static int xpm_isspace(char c)
{
	return c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v';
}

static char *xpm_extract_color(const char *buffer)
{
	const char *p = &buffer[0];
	int new_key = 0;
	int key = 0;
	int current_key = 1;
	int space = 128;
	char word[129], color[129], current_color[129];
	char *r;

	word[0] = '\0';
	color[0] = '\0';
	current_color[0] = '\0';
	while (1) {
		/* skip whitespace */
		for (; *p != '\0' && xpm_isspace(*p); p++)
			; /* nothing to do */
		/* copy word */
		for (r = word; *p != '\0' && !xpm_isspace(*p) && r - word < (int)sizeof(word) - 1; p++, r++)
			*r = *p;
		*r = '\0';
		if (*word == '\0') {
			if (color[0] == '\0') /* incomplete colormap entry */
				return NULL;
			/* end of entry, still store the last color */
			new_key = 1;
		} else if (key > 0 && color[0] == '\0') {
			/* next word must be a color name part */
			new_key = 0;
		} else {
			if (strcmp(word, "c") == 0)
				new_key = 5;
			else if (strcmp(word, "g") == 0)
				new_key = 4;
			else if (strcmp(word, "g4") == 0)
				new_key = 3;
			else if (strcmp(word, "m") == 0)
				new_key = 2;
			else if (strcmp(word, "s") == 0)
				new_key = 1;
			else
				new_key = 0;
		}
		if (new_key == 0) { /* word is a color name part */
			if (key == 0)   /* key expected */
				return NULL;
			/* accumulate color name */
			if (color[0] != '\0') {
				strncat(color, " ", space);
				space -= MIN(space, 1);
			}
			strncat(color, word, space);
			space -= MIN(space, (int)strlen(word));
		} else { /* word is a key */
			if (key > current_key) {
				current_key = key;
				strcpy(current_color, color);
			}
			space = 128;
			color[0] = '\0';
			key = new_key;
			if (*p == '\0')
				break;
		}
	}
	if (current_key > 1)
		return strdup(current_color);
	else
		return NULL;
}

/* (almost) direct copy from gdkpixmap.c... loads an XPM from a file */

static const char *file_buffer(enum buf_op op, void *handle)
{
	struct file_handle *h = (struct file_handle *)handle;

	if (op == op_header) {
		if (xpm_seek_string(h->infile, "XPM") != 1)
			goto out;
		if (xpm_seek_char(h->infile, '{') != 1)
			goto out;
	}
	if (op == op_cmap || op == op_header) {
		xpm_seek_char(h->infile, '"');
		fseek(h->infile, -1, SEEK_CUR);
	}
	if (xpm_read_string(h->infile, &h->buffer, &h->buffer_size))
		return h->buffer;
out:
	return NULL;
}

static struct xpm_color *lookup_color(struct xpm_color *colors, int n_colors, const char *name)
{
	int i;

	for (i = 0; i < n_colors; i++) {
		struct xpm_color *color = &colors[i];

		if (strcmp(name, color->color_string) == 0)
			return color;
	}
	return NULL;
}

/* This function does all the work. */
static u_int32_t *pixbuf_create_from_xpm(const char *(*get_buf)(enum buf_op op, void *handle),
					 void *handle,
					 int *width,
					 int *height)
{
	int w, h, n_col, cpp, x_hot, y_hot, items;
	int cnt, xcnt, ycnt, wbytes, n;
	int is_trans = 0;
	const char *buffer;
	char *name_buf;
	char pixel_str[32];
	struct xpm_color *colors, *color, *fallbackcolor;
	u_int32_t *data = NULL;

	fallbackcolor = NULL;

	buffer = (*get_buf)(op_header, handle);
	if (!buffer)
		return NULL;
	items = sscanf(buffer, "%d %d %d %d %d %d", &w, &h, &n_col, &cpp, &x_hot, &y_hot);

	if (items != 4 && items != 6)
		return NULL;
	if (w <= 0)
		return NULL;
	if (h <= 0)
		return NULL;
	if (cpp <= 0 || cpp >= 32)
		return NULL;
	if (n_col <= 0 || n_col >= INT_MAX / (cpp + 1) || n_col >= INT_MAX / (int)sizeof(struct xpm_color))
		return NULL;

	name_buf = (char *)calloc(n_col, cpp + 1);
	if (!name_buf)
		return NULL;
	colors = (struct xpm_color *)calloc(n_col, sizeof(struct xpm_color));
	if (!colors) {
		free(name_buf);
		return NULL;
	}

	for (cnt = 0; cnt < n_col; cnt++) {
		char *color_name;

		buffer = (*get_buf)(op_cmap, handle);
		if (!buffer) {
			free(colors);
			free(name_buf);
			return NULL;
		}

		color = &colors[cnt];
		color->color_string = &name_buf[cnt * (cpp + 1)];
		strncpy(color->color_string, buffer, cpp);
		color->color_string[cpp] = 0;
		buffer += strlen(color->color_string);
		color->transparent = 0;

		color_name = xpm_extract_color(buffer);

		if (!color_name || (strcasecmp(color_name, "None") == 0) ||
		    (parse_color(color_name, color) == 0)) {
			color->transparent = 1;
			color->red = 0;
			color->green = 0;
			color->blue = 0;
			is_trans = 1;
		}

		free(color_name);

		if (cnt == 0)
			fallbackcolor = color;
	}

	data = (u_int32_t *)calloc(w * h, sizeof(u_int32_t));
	if (!data) {
		free(colors);
		free(name_buf);
		return NULL;
	}

	wbytes = w * cpp;

	for (ycnt = 0; ycnt < h; ycnt++) {
		buffer = (*get_buf)(op_body, handle);
		if ((!buffer) || ((int)strlen(buffer) < wbytes))
			continue;

		for (n = 0, xcnt = 0; n < wbytes; n += cpp, xcnt++) {
			uint a, r, g, b;

			strncpy(pixel_str, &buffer[n], cpp);
			pixel_str[cpp] = 0;

			color = lookup_color(colors, n_col, pixel_str);

			/* Bad XPM...punt */
			if (!color)
				color = fallbackcolor;

			a = 0xFF;
			if (is_trans && color->transparent)
				a = 0;
			r = color->red >> 8;
			g = color->green >> 8;
			b = color->blue >> 8;
			data[ycnt * w + xcnt] = (a << 24) | (r << 16) | (g << 8) | b;
		}
	}

	free(colors);
	free(name_buf);

	*width = w;
	*height = h;
	return data;
}

cairo_surface_t *get_xpm_icon(const char *filename)
{
	struct file_handle handle;
	int w, h;
	u_int32_t *data;
	cairo_surface_t *surface;
	unsigned char *surface_data;

	memset(&h, 0, sizeof(h));
	handle.infile = fopen(filename, "r");
	if (!handle.infile) {
		fprintf(stderr, "Failed to load XPM file: %s\n", filename);
		return NULL;
	}
	handle.buffer_size = 4096;
	handle.buffer = calloc(4096, 1);
	data = pixbuf_create_from_xpm(file_buffer, &handle, &w, &h);
	free(handle.buffer);
	fclose(handle.infile);
	if (!data) {
		fprintf(stderr, "Failed to load XPM file: %s\n", filename);
		return NULL;
	}

	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
	if (!surface) {
		fprintf(stderr, "Failed to load XPM file: %s\n", filename);
		free(data);
		return NULL;
	}
	surface_data = cairo_image_surface_get_data(surface);
	cairo_surface_flush(surface);
	memcpy(surface_data, data, w * h * 4);
	free(data);
	cairo_surface_mark_dirty(surface);
	return surface;
}
