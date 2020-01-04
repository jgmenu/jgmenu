#ifndef JGMENU_LANG_H
#define JGMENU_LANG_H

/*
 * lang_code - read $LANG and parse ll_CC.UTF8 format where
 *  - ‘ll’ is an ISO 639 two-letter language code (lowercase)
 *  - ‘CC’ is an ISO 3166 two-letter country code (uppercase)
 * @ll - pointer to 'll' string
 * @ll_cc - pointer to 'll_CC' string
 * Return -1 if $LANG not set
 */
int lang_code(char **ll, char **ll_cc);

/*
 * lang_localized_name_key - use lang_code() to produce "Name[]" keys
 * @name_ll - pointer to "Name[ll]"
 * @name_ll_cc - pointer to "Name[ll_CC]"
 */
void lang_localized_name_key(char **name_ll, char **name_ll_cc);
void lang_localized_gname_key(char **gname_ll, char **gname_ll_cc);

#endif /* JGMENU_LANG_H */
