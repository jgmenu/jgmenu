/*
 * Simple internationalisation library for jgmenu menu data.
 *
 * Copyright (C) Johan Malm 2018
 *
 * This implementation is inspired by gettext, but ignores everything in
 * po-files except the msgid and msgstr data. It does not support plural forms
 * or strings spanning multiple lines.
 */

#ifndef I18N_H
#define I18N_H

#include <stdio.h>
#include <stdlib.h>

#include "sbuf.h"

/**
 * i18n_open - open translation file and read msg{id,str} data into hashmap
 * @filename: filename of po file to be processed
 */
extern void i18n_open(const char *filename);

/**
 * i18n_tranlate - translate string
 * @s: string to be translated
 * If no translation is available, s is untouched.
 * The remainder (i.e. anything beyond the first field) is returned
 */
extern char *i18n_translate(struct sbuf *s);

#endif /* I18N_H */
