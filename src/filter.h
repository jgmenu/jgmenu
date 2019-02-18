#ifndef FILTER_H
#define FILTER_H

#include "compat.h"

void filter_init(void);
char *filter_strdup_needle(void);
int filter_get_clear_on_keyboard_input(void);
void filter_set_clear_on_keyboard_input(int clear);
void filter_addstr(const char *str, size_t n);
void filter_backspace(void);
void filter_reset(void);
int filter_needle_length(void);
int filter_ismatch(const char *haystack);
void filter_cleanup(void);

#endif /* FILTER_H */
