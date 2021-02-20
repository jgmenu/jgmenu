#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include <sys/stat.h>

#define xfree(pointer) safe_free((void **)&(pointer))

#define XATOI_NONNEG (01)
#define XATOI_GT_0   (02)

#define BUG_ON(condition) do { \
	if ((condition) != 0) { \
		fprintf(stderr, "Badness in %s() at %s:%d\n", __func__, __FILE__, \
			__LINE__); \
	} \
} while (0)

void mute_info(void);
void info(const char *err, ...);
void warn(const char *err, ...);
void die(const char *err, ...);
void safe_free(void **ptr);
char *xstrdup(const char *s);
void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);
void *xcalloc(size_t nb, size_t size);
void rtrim(char **s);
char *strstrip(char *s);
int parse_config_line(char *line, char **option, char **value);
int hex_to_dec(char c);
int parse_hexstr(char *hex, double *rgba);
int get_first_num_from_str(const char *s);
void xatoi(int *var, const char *value, int flags, const char *key);
void cat(const char *filename);
void remove_caret_markup_closing_bracket(char *s);
void mkdir_p(const char *path);
void msleep(unsigned int duration);

#endif /* UTIL_H */
