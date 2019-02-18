#include <stdio.h>
#include <stdlib.h>

#include "xpm-loader.h"

int main(int argc, char **argv)
{
	goto inside;
	while (argc > 0) {
		char *filename = argv[0];
		char png_name[1024];
		cairo_surface_t *surface;
		fprintf(stderr, "info: convert %s\n", filename);
		surface = get_xpm_icon(filename);
		if (!surface) {
			fprintf(stderr, "fatal: xpm icon (%s) failed to load\n", filename);
			exit(EXIT_FAILURE);
		}
		snprintf(png_name, sizeof(png_name), "%s.png", filename);
		if (cairo_surface_write_to_png(surface, png_name)) {
			fprintf(stderr, "fatal: could not save XPM icon (%s) as png\n", filename);
			exit(EXIT_FAILURE);
		}
		cairo_surface_destroy(surface);
inside:
		argc--, argv++;
	}
	exit(EXIT_SUCCESS);
}
