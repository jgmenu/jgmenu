/*
 * i18n.c
 *
 * Copyright (C) Johan Malm 2018
 */

#include <string.h>
#include <sys/stat.h>

#include "i18n.h"
#include "util.h"
#include "hashmap.h"
#include "banned.h"

static struct hashmap map;
static char *translation_file;

struct map_entry {
	struct hashmap_entry ent;
	char key[FLEX_ARRAY];
};

static int cmp(const struct map_entry *e1, const struct map_entry *e2,
	       const char *key)
{
	return strcasecmp(e1->key, key ? key : e2->key);
}

static struct map_entry *alloc_entry(int hash, char *key, int klen,
				     char *value, int vlen)
{
	struct map_entry *entry = malloc(sizeof(struct map_entry) + klen
			+ vlen + 2);
	hashmap_entry_init(entry, hash);
	memcpy(entry->key, key, klen + 1);
	memcpy(entry->key + klen + 1, value, vlen + 1);
	return entry;
}

static const char *get_value(const struct map_entry *e)
{
	return e->key + strlen(e->key) + 1;
}

static void hashput(char *id, char *str)
{
	int hash = 0;
	struct map_entry *entry;

	hash = strihash(id);
	entry = alloc_entry(hash, id, strlen(id), str, strlen(str));
	entry = hashmap_put(&map, entry);

	/* If an entry has been replaced, free it */
	xfree(entry);
}

static char *stripquotes(char *s)
{
	char *start, *end;

	start = strchr(s, '"');
	if (start)
		++start;
	else
		start = s;
	end = strrchr(start, '"');
	if (end)
		*(end) = '\0';
	return start;
}

static void process_line(char *line)
{
	char *p;
	static char *msgid;

	BUG_ON(!line);
	p = strrchr(line, '\n');
	if (p)
		*p = '\0';
	if (!strncmp(line, "msgid", 5)) {
		if (msgid)
			warn("last msgid had no corresponding msgstr");
		msgid = strdup(stripquotes(line + 5));
	} else if (!strncmp(line, "msgstr", 6)) {
		if (!msgid)
			warn("msgstr without msgid");
		hashput(msgid, stripquotes(line + 6));
		xfree(msgid);
	}
}

static void read_file(FILE *fp)
{
	char line[1024];

	while (fgets(line, sizeof(line), fp))
		process_line(line);
}

static void parse_file(char *filename)
{
	FILE *fp;

	fp = fopen(filename, "r");
	if (!fp)
		return;
	read_file(fp);
	fclose(fp);
}

static void i18n_open(void)
{
	struct stat sb;
	struct sbuf s;
	static int has_run;

	if (!has_run) {
		hashmap_init(&map, (hashmap_cmp_fn) cmp, 0);
		has_run = 1;
	}
	BUG_ON(!translation_file);
	if (translation_file[0] == '\0')
		return;
	sbuf_init(&s);
	sbuf_cpy(&s, translation_file);
	sbuf_expand_tilde(&s);
	if (stat(s.buf, &sb) < 0)
		die("%s: stat()", __func__);
	parse_file(s.buf);
	xfree(s.buf);
}

/*
 * Read $LANG and parse ll_CC.UTF8 format where
 *  - ‘ll’ is an ISO 639 two-letter language code (lowercase).
 *  - ‘CC’ is an ISO 3166 two-letter country code (uppercase).
 */
static void find_translation_file_within_dir(struct sbuf *s)
{
	char *p;
	struct stat st;

	p = getenv("LANG");
	if (!p) {
		warn("LANG not set - cannot not find translation file");
		sbuf_cpy(s, "");
		return;
	}
	if (s->buf[s->len] != '/')
		sbuf_addch(s, '/');
	sbuf_addstr(s, p);

	/* try ll_CC */
	if (strchr(getenv("LANG"), '.')) {
		p = strrchr(s->buf, '.');
		BUG_ON(!p);
		*p = '\0';
	}
	if (!stat(s->buf, &st))
		return;

	/* try ll */
	if (strchr(getenv("LANG"), '_')) {
		p = strrchr(s->buf, '_');
		BUG_ON(!p);
		*p = '\0';
	}
	if (stat(s->buf, &st) < 0)
		sbuf_cpy(s, "");
}

char *i18n_set_translation_file(const char *filename)
{
	struct stat st;
	struct sbuf s;

	sbuf_init(&s);
	sbuf_cpy(&s, filename);
	sbuf_expand_tilde(&s);

	if (stat(s.buf, &st) < 0) {
		warn("i18n: file '%s' does not exist", s.buf);
		translation_file = NULL;
		xfree(s.buf);
		return NULL;
	}
	if (S_ISDIR(st.st_mode)) {
		find_translation_file_within_dir(&s);
		if (!s.len) {
			warn("i18n: could not find translation file in dir '%s'",
			     filename);
			translation_file = NULL;
			xfree(s.buf);
			return NULL;
		}
	}
	translation_file = s.buf;
	info("i18n: translation file '%s' loaded", translation_file);
	i18n_open();
	return translation_file;
}

char *i18n_translate(const char *s)
{
	struct map_entry *entry;
	int hash = 0;

	hash = strihash(s);
	entry = hashmap_get_from_hash(&map, hash, s);
	return entry ? (char *)get_value(entry) : NULL;
}

void i18n_translate_first_field(struct sbuf *s)
{
	char *tmp, *remainder = NULL, *translation = NULL;

	BUG_ON(!s || !s->buf);
	if (s->buf[0] == '\0')
		return;
	/* Make a copy as we leave 's' untouched if no translation exists */
	tmp = xstrdup(s->buf);
	remainder = strchr(tmp, ',');
	if (remainder) {
		*remainder = '\0';
		++remainder;
	}
	/* tmp now contains the first field, which we want to translate */
	translation = i18n_translate(tmp);
	if (translation) {
		sbuf_cpy(s, translation);
		if (remainder) {
			sbuf_addch(s, ',');
			sbuf_addstr(s, remainder);
		}
	}
	xfree(tmp);
}

void i18n_cleanup(void)
{
	hashmap_free(&map, 1);
}
