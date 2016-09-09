#ifndef XSETTINGS_H
#define XSETTINGS_H

#include <X11/Xlib.h>
#include <X11/Xmd.h>

/* Types of settings possible. Enum values correspond to protocol values. */
#define XSETTINGS_TYPE_INT 0
#define XSETTINGS_TYPE_STRING 1
#define XSETTINGS_TYPE_COLOR 2
#define XSETTINGS_TYPE_NONE 0xff

typedef struct XSettingsColor {
	unsigned short red, green, blue, alpha;
} XSettingsColor;

typedef struct XSetting {
	unsigned char type;
	char *name;
	union {
		int int_value;
		char *string_value;
		XSettingsColor color_value;
	} value;
	CARD32 serial;
} XSetting;

XSetting *get_xsettings(Display *display, size_t *count);
void free_xsettings(XSetting *settings, size_t count);

#endif /* XSETTINGS_H */
