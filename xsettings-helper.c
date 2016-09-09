/*
 * Copyright (C) 2016 Ovidiu M <mrovi9000@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>

#include "xsettings.h"

void print_setting(XSetting *setting)
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

void print_settings(XSetting *settings, size_t count)
{
	for (size_t i = 0; i < count; i++)
		print_setting(&settings[i]);
}

int main(void)
{
	Display *display = XOpenDisplay(NULL);
	size_t count;
	XSetting *settings = get_xsettings(display, &count);

	if (!display) {
		fprintf(stderr, "Could not open display\n");
		return 1;
	}
	if (settings) {
		print_settings(settings, count);
		free_xsettings(settings, count);
	} else {
		fprintf(stderr, "Could not read settings\n");
	}

	XCloseDisplay(display);
	return 0;
}
