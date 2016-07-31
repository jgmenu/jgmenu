/*
 * xdgmenu.c
 *
 * This is still completely experimental and doesn't yet work
 *
 * Parses xml .menu file and outputs a csv formatted jgmenu file
 *
 * It aim to be XDG compliant, although it has a long way to go!
 */

#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "util.h"
#include "sbuf.h"

#define DEBUG_PRINT_XML_NODES 0
#define DEBUG_PRINT_CACHE 0

/*
 * In jgmenu-speak,
 * 	- a "node" is an item with a ^tag() mark-up
 *	- "item" refers to anything else. "items" hang off "nodes"
 */
struct Menu_node {
	char *tag;
	char *name;
	struct list_head menu_items;
	struct list_head list;
};

struct Menu_item {
	char *name;
	char *icon_name;
	char *cmd;
	struct list_head list;
};

/*
 * Cache is needed to temporarily store data for each menu chunk (i.e. what's
 * within each <Menu></Menu>) for the following reasons:
 *   - in case <Name></Name> does not come first
 *     (the algorithm needs to know the names to great new ^tag() sections, etc
 *   - to create lists of desktop files taking into account all <Include> and
 *     <Exclude> tags
 */ 
struct Cache {
	char *name;
	char *directory;
//	struct list_head category_includes;
//	struct list_head category_excludes;
//	struct list_head categories;
//	struct list_head desktop_files;
	int level;
	struct list_head list;
};

static LIST_HEAD(menu_nodes);
static LIST_HEAD(cache);
static int menu_level;

static void print_csv_menu()
{
	struct Menu_node *n;
	struct Menu_item *item;
	
	list_for_each_entry(n, &menu_nodes, list) {
		printf("%s,^tag(%s)\n", n->name, n->tag);
		list_for_each_entry(item, &(n->menu_items), list) {
			printf("%s,%s\n", item->name, item->cmd);
		}
		printf("\n");
	}
}

static void add_item_to_jgmenu_node(const char *node, const char *content, int isdir)
{
	struct Menu_node *n;
	struct Menu_item *item;
	struct String s;

	sbuf_init(&s);
	if (isdir)
		sbuf_addstr(&s, "^checkout(");
	sbuf_addstr(&s, content);
	if (isdir)
		sbuf_addstr(&s, ")");

	list_for_each_entry(n, &menu_nodes, list) {
		if (strcmp(n->name, node))
			continue;

		item = xmalloc(sizeof(struct Menu_item));
		item->name = strdup(content);
		item->cmd = strdup(s.buf);
		list_add_tail(&(item->list), &(n->menu_items));
		return;
	}

	fprintf(stderr, "warning: node '%s' not found; nothing added\n", node);
}

static void add_jgmenu_node(const char *content)
{
	struct Menu_node *node;

	node = xmalloc(sizeof(struct Menu_node));
	node->name = strdup(content);
	node->tag = strdup(content);
	list_add_tail(&(node->list), &menu_nodes);
	INIT_LIST_HEAD(&(node->menu_items));
}

static void build_jgmenu_structure_from_cache()
{
	struct Cache *cache_item;
	static char parent[16][1024];

	list_for_each_entry_reverse(cache_item, &cache, list) {
		strcpy(parent[cache_item->level], cache_item->name);
		add_jgmenu_node(cache_item->name);

		/* TODO: parse .directory */
		/* The directory information is only useful to your parent! */
		if (cache_item->level) {
			add_item_to_jgmenu_node(parent[cache_item->level - 1],
					 "parsed .directory file", 1);
		}

		/* TODO: parse .desktop files and filter on category */
		add_item_to_jgmenu_node(cache_item->name, "some_desktop_file", 0);
	}
}



/*
 * TODO: Process stuff within <Menu></Menu> to establish categories and
 * .dekstop-files, etc taking into account includes and excludes.
 */
static void process_cache()
{
	struct Cache *cache_item;

	if (DEBUG_PRINT_CACHE) {
		printf("PRINT CACHE:\n");
		list_for_each_entry_reverse(cache_item, &cache, list)
			printf("L=%d; Name=%s; Directory=%s; Categories=\n", cache_item->level,
				cache_item->name, cache_item->directory);
	}

}

static int iskeyword(const char *node_name)
{
	if (!strcasecmp(node_name, "Include.And.Category") ||
	    !strcasecmp(node_name, "Name") ||
	    !strcasecmp(node_name, "Directory"))
		return 1;

	return 0;
}

static void add_to_cache(const char *node_name, const char *content)
{
	struct Cache *cache_item;

	if (!iskeyword(node_name))
		return;

	if (list_empty(&cache)) {
		fprintf(stderr, "warning: cache is empty\n");
		return;
	}

	if (DEBUG_PRINT_XML_NODES)
		printf("%d-%s: %s\n", menu_level, node_name, content);

	cache_item = list_first_entry(&cache, struct Cache, list);

	cache_item->level = menu_level;

	if (!strcasecmp(node_name, "Name")) {
		cache_item->name = strdup(content);
	} else if (!strcasecmp(node_name, "Directory")) {
		cache_item->directory = strdup(content);
	} else if (!strcasecmp(node_name, "Include.And.Category")) {
		//cache_item->categores...
	}
}

static void create_new_cache_entry()
{
	struct Cache *cache_item;

	cache_item = xmalloc(sizeof(struct Cache));
	list_add(&(cache_item->list), &cache);
}

static int level(xmlNode *node)
{
	int level = 0;

	for(;;) {
		node = node->parent;
		if (!node || !node->name)
			return level;

		if (!strcasecmp((char *)node->name, "Menu"))
			++level;
	}
}

static void get_full_node_name(struct String *node_name, xmlNode *node)
{
	int ismenu;

	if (!strcmp((char *)node->name, "text")) {
		node = node->parent;
		if (!node || !node->name) {
			fprintf(stderr, "warning: node->name == 'text' and node is 'root'\n");
			return;
		}
	}

	ismenu = !strcmp((char *)node->name, "Menu");
	for(;;) {
		if (!ismenu)
			sbuf_prepend(node_name, (char *)node->name);

		node = node->parent;
		if (!node || !node->name)
			return;

		ismenu = !strcmp((char *)node->name, "Menu");
		if (!ismenu)
			sbuf_prepend(node_name, ".");
	}
}

static void process_node(xmlNode *node)
{
	struct String node_name;

	/*
	 * Just filtering out a lot of rubbish here for the time being.
	 * We will need to check for <OnlyUnallocated/> etc.
	 */
	if (!node->content)
		return;

	sbuf_init(&node_name);

	get_full_node_name(&node_name, node);

	/* This filter out lots of rubbish */
	if (node_name.len || !strlen(strstrip((char *)node->content))) {
		add_to_cache(node_name.buf, strstrip((char *)node->content));
	}

	free(node_name.buf);
}

static void xml_tree_walk(xmlNode *node)
{
	xmlNode *n;

	for (n = node; n; n = n->next) {
		if (!strcasecmp((char *)n->name, "Menu")) {
			if (DEBUG_PRINT_XML_NODES)
				printf("---\n");
			create_new_cache_entry();
			menu_level = level(n);
			xml_tree_walk(n->children);
			continue;
		}

		if (!strcasecmp((char *)n->name, "Comment"))
			continue;

		process_node(n);
		xml_tree_walk(n->children);
	}
}

static void parse_xml(const char *filename)
{
	xmlDoc *d = xmlReadFile(filename, NULL, 0);

	if (!d)
		die("error reading file '%s'\n", filename);

	xml_tree_walk(xmlDocGetRootElement(d));
	xmlFreeDoc(d);
	xmlCleanupParser();
}

int main(int argc, char **argv)
{
	LIBXML_TEST_VERSION

	if (argc < 2)
		parse_xml("/etc/xdg/menus/gnome-applications.menu");
	else
		parse_xml(argv[1]);

	process_cache();
	build_jgmenu_structure_from_cache();

	print_csv_menu();

	return 0;
}
