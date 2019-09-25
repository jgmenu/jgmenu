#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "util.h"
#include "back.h"

static char *back_str;

struct back_lookup_table {
	const char *lang;
	const char *back_string;
};

/* clang-format off */
static struct back_lookup_table back_lookup_table[] = {
	{ "am", "ወደ ኋላ"},
	{ "ar", "إلى الخلف" },
	{ "ast", "Atrás" },
	{ "be", "Назад" },
	{ "bg", "Назад" },
	{ "bn", "পূর্ববর্তী" },
	{ "ca", "Endarrere" },
	{ "cs", "Zpět" },
	{ "da", "Tilbage" },
	{ "de", "Zurück" },
	{ "el", "Πίσω" },
	{ "eo", "Malantaŭen" },
	{ "es", "Atrás" },
	{ "et", "Tagasi" },
	{ "eu", "Atzera" },
	{ "fa", "بازگشت" },
	{ "fi", "Edellinen" },
	{ "fr", "Précédent" },
	{ "gl", "Recuar" },
	{ "he", "אחורה" },
	{ "hr", "Natrag" },
	{ "hu", "Vissza" },
	{ "id", "Kembali" },
	{ "is", "Til baka" },
	{ "it", "Indietro" },
	{ "ja", "戻る" },
	{ "kk", "Артқа" },
	{ "ko", "뒤로" },
	{ "lt", "Atgal" },
	{ "lv", "Atpakaļ" },
	{ "ms", "Undur" },
	{ "nb", "Tilbake" },
	{ "nl", "Terug" },
	{ "nn", "Tilbake" },
	{ "oc", "Precedent" },
	{ "pa", "ਪਿੱਛੇ"  },
	{ "pl", "Wstecz" },
	{ "pt", "Recuar" },
	{ "ro", "Înapoi" },
	{ "ru", "Назад" },
	{ "sk", "Späť" },
	{ "sq", "Prapa" },
	{ "sr", "Назад" },
	{ "sv", "Bakåt" },
	{ "te", "వెనుకకు" },
	{ "th", "ถอยกลับ" },
	{ "tr", "Geri" },
	{ "ug", "ئارقىسىغا" },
	{ "uk", "Назад" },
	{ "ur", "پیچھے" },
	{ "vi", "Quay lui" },
	{ "zh", "后退" },
	{ NULL, NULL }
};
/* clang-format on */

static int get_lang(char *b)
{
	char *s;

	s = getenv("LANG");
	if (!s || strlen(s) < 2)
		return -1;
	b[0] = s[0];
	b[1] = s[1];
	b[2] = '\0';
	return 0;
}

/* Localized support for "Back" */
static void init_back_string(void)
{
	static char *language;
	struct back_lookup_table *p;

	language = xmalloc(3 * sizeof(char));
	get_lang(language);
	for (p = back_lookup_table; p->lang; p++) {
		if (!strcmp(p->lang, language)) {
			back_str = strdup(p->back_string);
			break;
		}
	}
	if (!back_str)
		back_str = strdup("Back");
	free(language);
}

char *back_string(void)
{
	if (!back_str)
		init_back_string();

	return back_str;
}
