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
 * i18n_set_translation_file - set and open translation file and read
 *                             msg{id,str} data into hashmap
 * @filename: file or directory to open
 */
char *i18n_set_translation_file(const char *filename);

/**
 * i18n_tranlate - translate string
 * @s: string to be translated
 * Return translation, or NULL if none exists.
 */
char *i18n_translate(const char *s);

/**
 * i18n_tranlate_first_field - translate first comma separated field of string
 * @s: string to be translated
 * If no translation is available, s is untouched.
 */
void i18n_translate_first_field(struct sbuf *s);

/**
 * i18n_cleanup - free allocated memory
 */
void i18n_cleanup(void);

#endif /* I18N_H */
