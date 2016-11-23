#ifndef FILTER_H
#define FILTER_H

#include "compat.h"

void filter_init(void);
void filter_addstr(const char *str, size_t n);
void filter_backspace(void);
int filter_needle_length(void);
int filter_ismatch(const char *haystack);

#endif /* FILTER_H */
