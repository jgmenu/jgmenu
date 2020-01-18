#ifndef JGMENU_LANG_H
#define JGMENU_LANG_H

/**
 * lang_name_ll - return "Name[ll]"
 *
 * Read $LANG and parse ll_CC.UTF8 format where
 *  - ‘ll’ is an ISO 639 two-letter language code (lowercase)
 *  - ‘CC’ is an ISO 3166 two-letter country code (uppercase)
 */
char *lang_name_ll(void);

/** name_llcc - return "Name[ll_CC]" */
char *lang_name_llcc(void);

/** gname_ll - return "GenericName[ll]" */
char *lang_gname_ll(void);

/** gname_llcc - return "GenericName[ll_CC]" */
char *lang_gname_llcc(void);

#endif /* JGMENU_LANG_H */
