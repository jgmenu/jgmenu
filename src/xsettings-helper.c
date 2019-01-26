/*
 * Copyright (C) 2016 Ovidiu M <mrovi9000@gmail.com>
 * Modified by Johan Malm
 */

#include <stdio.h>
#include <stdlib.h>

#include "sbuf.h"
#include "xsettings-helper.h"
#include "xsettings.h"

static void print_setting(struct xsetting *setting)
{
	printf("%s: ", setting->name);
	if (setting->type == XSETTINGS_TYPE_INT)
		printf("%d\n", setting->value.int_value);
	else if (setting->type == XSETTINGS_TYPE_STRING)
		printf("%s\n", setting->value.string_value);
	else if (setting->type == XSETTINGS_TYPE_COLOR)
		printf("%2x%2x%2x %d\n", setting->value.color_value.red,
		       setting->value.color_value.green,
		       setting->value.color_value.blue,
		       setting->value.color_value.alpha);
	else
		printf("??\n");
}

static void print_settings(struct xsetting *settings, size_t count, const char *key,
			   struct sbuf *s)
{
	size_t i;

	for (i = 0; i < count; i++) {
		if (key[0] == '*') {
			print_setting(&settings[i]);
		} else {
			if (settings[i].type == XSETTINGS_TYPE_STRING &&
			    !strcmp(settings[i].name, key)) {
				sbuf_cpy(s, settings[i].value.string_value);
				break;
			}
		}
	}
}

void xsettings_get(struct sbuf *s, const char *key)
{
	Display *display;
	size_t count;
	struct xsetting *settings;

	if (!key)
		return;
	display = XOpenDisplay(NULL);
	if (!display)
		return;

	settings = get_xsettings(display, &count);
	if (settings) {
		print_settings(settings, count, key, s);
		free_xsettings(settings, count);
	}

	XCloseDisplay(display);
}
