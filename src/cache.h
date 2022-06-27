#ifndef CACHE_H
#define CACHE_H

#include "sbuf.h"

void cache_set_icon_theme(const char *theme);
void cache_set_icon_size(int size);
char *cache_icon_get_dir(void);
int cache_touch(const char *name);
int cache_strdup_path(const char *name, struct sbuf *path);
int cache_create_symlink(char *path, char *name);
void cache_atexit_cleanup(void);

#endif /* CACHE_H */
