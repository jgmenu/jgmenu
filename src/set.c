/*
 * set.c - set key/value pairs on config file
 *
 * Copyright (C) Johan Malm 2019
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "sbuf.h"
#include "compat.h"
#include "set.h"
#include "banned.h"

#define strsiz (1000)
struct entry {
	char line[strsiz];
	char key[strsiz];
	char value[strsiz];
	int is_commented_out;
};

static struct entry *entries;
static int nr_entries, alloc_entries;
static int config_file_has_been_read;

static struct entry *add_entry(void)
{
	struct entry *entry;

	if (nr_entries == alloc_entries) {
		alloc_entries = (alloc_entries + 16) * 2;
		entries = xrealloc(entries, alloc_entries * sizeof(*entry));
	}
	entry = entries + nr_entries;
	memset(entry, 0, sizeof(*entry));
	nr_entries++;
	return entries + nr_entries - 1;
}

static void process_line(char *line)
{
	struct entry *entry;
	char *key = NULL, *value = NULL;

	entry = add_entry();
	strlcpy(entry->line, line, sizeof(entry->line));

	/* we parse commented out lines too */
	if (line[0] == '#')
		entry->is_commented_out = 1;
	parse_config_line(line + entry->is_commented_out, &key, &value);
	if (!key || !value)
		return;
	strlcpy(entry->key, key, sizeof(entry->key));
	strlcpy(entry->value, value, sizeof(entry->value));
}

void set_write(const char *filename)
{
	FILE *fp;
	int i;

	fp = fopen(filename, "w");
	if (!fp) {
		warn("could not open file %s", filename);
		return;
	}
	for (i = 0; i < nr_entries; i++)
		fprintf(fp, "%s\n", entries[i].line);
	fclose(fp);
}

int set_key_exists(const char *key)
{
	int i;

	for (i = 0; i < nr_entries; i++) {
		if (!entries[i].key)
			continue;
		if (!strcmp(entries[i].key, key))
			return 1;
	}
	return 0;
}

void set_set(const char *key, const char *value, int is_commented_out)
{
	int i;
	struct entry *e = NULL;

	if (!key || !value)
		die("NULL passed to function %s()", __func__);
	/* look for existing entry */
	for (i = 0; i < nr_entries; i++) {
		if (!entries[i].key)
			continue;
		if (!strcmp(entries[i].key, key)) {
			e = entries + i;
			goto entry_already_exists;
		}
	}
	e = add_entry();
entry_already_exists:
	e->is_commented_out = is_commented_out;
	if (e->is_commented_out)
		snprintf(e->line, sizeof(e->line), "# %s = %s", key, value);
	else
		snprintf(e->line, sizeof(e->line), "%s = %s", key, value);
	strlcpy(e->key, key, sizeof(e->key));
	strlcpy(e->value, value, sizeof(e->value));
}

int set_is_already_set_correctly(const char *key, const char *value)
{
	int i;

	for (i = 0; i < nr_entries; i++) {
		if (!entries[i].key)
			continue;
		if (strcmp(entries[i].key, key) != 0)
			continue;
		if (!strcmp(entries[i].value, value) != 0)
			return 1;
	}
	return 0;
}

void set_read(const char *filename)
{
	FILE *fp;
	char line[4096];

	if (config_file_has_been_read) {
		warn("%s(): trying to read config file again", __func__);
		return;
	}
	config_file_has_been_read = 1;
	fp = fopen(filename, "r");
	if (!fp) {
		warn("could not open file %s", filename);
		return;
	}
	while (fgets(line, sizeof(line), fp)) {
		char *p;

		if (line[0] == '\0')
			continue;
		p = strrchr(line, '\n');
		if (!p)
			continue;
		*p = '\0';
		process_line(line);
	}
	fclose(fp);
}

