#ifndef FILTER_H
#define FILTER_H

#include "compat.h"

void filter_init(void);
char *filter_strdup_needle(void);
void filter_addstr(const char *str, size_t n);
void filter_backspace(void);
void filter_reset(void);
int filter_needle_length(void);
int filter_ismatch(const char *haystack);
void filter_cleanup(void);

#endif /* FILTER_H */
