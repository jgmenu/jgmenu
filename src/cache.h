#ifndef CACHE_H
#define CACHE_H

#include "sbuf.h"

extern void cache_set_icon_theme(const char *theme);
extern void cache_set_icon_size(int size);
extern int cache_touch(const char *name);
extern int cache_strdup_path(const char *name, struct sbuf *path);
extern int cache_create_symlink(char *path, char *name);
extern void cache_atexit_cleanup(void);

#endif /* CACHE_H */
