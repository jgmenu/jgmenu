#include <stdio.h>
#include <stdlib.h>

#include "../src/xpm-loader.h"

int main(int argc, char **argv)
{
  argc--, argv++;
  while (argc > 0) {
    char *filename = argv[0];
    char png_name[1024];
    cairo_surface_t *surface;
    fprintf(stderr, "Loading XPM icon %s...\n", filename);
    surface = get_xpm_icon(filename);
    if (!surface) {
      fprintf(stderr, "XPM icon %s failed to load!\n", filename);
      exit(EXIT_FAILURE);
    }
    sprintf(png_name, "%s.png", filename);
    if (cairo_surface_write_to_png(surface, png_name)) {
      fprintf(stderr, "Could not save XPM icon %s as png!\n", filename);
      exit(EXIT_FAILURE);
    }
    cairo_surface_destroy(surface);
    argc--, argv++;
  }
  exit(EXIT_SUCCESS);
}
