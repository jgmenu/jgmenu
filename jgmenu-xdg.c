/*
 * jgmenu-xdg.c
 *
 * This is still completely experimental and only just works
 *
 * Parses xml .menu file and outputs a csv formatted jgmenu file
 *
 * It aim to be XDG compliant, although it has a long way to go!
 *
 * See this spec for further details:
 * https://specifications.freedesktop.org/menu-spec/menu-spec-1.0.html
 *
 * ===================================================================
 *
 * Quick outline of how this file works:
 *
 * #1 - Read .menu-file (XML format)
 * ---------------------------------
 * <Menu>
 *	<Name>Programming</Name>
 *	<Directory>Development.directory</Directory>
 *	<Category>Development</Category>
 * </Menu
 * <Menu>
 *	<Name>Graphics</Name>
 *	<Directory>Graphics.directory</Directory>
 *	<Category>Graphics</Category>
 * </Menu>
 *
 * #2 - Parse XML tree into nodes
 * ------------------------------
 * Menu.Name = Development
 * Menu.Directory = Development.directory
 * Menu.Category = Development
 *
 * Menu.Name = Graphics
 * Menu.Directory = Graphics.directory
 * Menu.Category = Graphics
 *
 * #3 - Create cache for each <Menu></Menu>
 * ----------------------------------------
 * Name=Development
 * Directory=Development.directory
 * Categories=		(this takes into account all includes and excludes)
 * Desktop-files=	(individual .desktop files if specified)
 * etc.
 *
 * #4 - Create jgmenu style csv file
 * ---------------------------------
 * root,^tag(root)
 * Programming,^checkout(Development)
 * Graphics,^checkout(Graphics)
 *
 * Programming,^tag(Development)
 * foo1,foo1
 * foo2,foo2
 *
 * Graphics,^tag(Graphics)
 * bar1,bar1
 * bar2,bar2
 */

#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

#include "xdgapps.h"
#include "util.h"
#include "sbuf.h"

#define DEBUG_PRINT_XML_NODES 0
#define DEBUG_PRINT_CACHE 0

/*
 * In jgmenu-speak,
 *	- a "node" is an item with a ^tag() mark-up
 *	- everything else is an "item"
 *	  "items" hang off "nodes"
 */
struct jgmenu_node {
	char *tag;
	char *name;
	char *parent;
	struct list_head menu_items;
	struct list_head list;
};

struct jgmenu_item {
	char *name;
	char *cmd;
	char *icon_name;
	struct list_head list;
};

/*
 * cache_entry is needed to temporarily store data for each menu chunk (i.e. what's
 * within each <Menu></Menu>) for the following reasons:
 *   - in case <Name></Name> does not come first
 *     (we need to know the names to great new ^tag() sections, etc)
 *   - to create lists of desktop files taking into account all <Include> and
 *     <Exclude> tags
 */
struct cache_entry {
	char *name;
	char *directory;
	struct list_head category_includes;
	struct list_head category_excludes;
	struct list_head categories;
	struct list_head desktop_files;
	int level;
	struct list_head list;
};

static LIST_HEAD(menu_nodes);
static LIST_HEAD(cache);
static int menu_level;

/* command line args */
static char *data_dir;

void usage(void)
{
	printf("Usage: jgmenu-xdg [OPTIONS] <menu-file>\n"
	       "    --data-dir=<dir>      specify directory containing .menu file\n");
	exit(0);
}

static void print_csv_menu(void)
{
	struct jgmenu_node *n;
	struct jgmenu_item *item;

	list_for_each_entry(n, &menu_nodes, list) {
		printf("%s,^tag(%s)\n", n->name, n->tag);
		if (n->parent)
			printf("Go back,^checkout(%s),folder\n", n->parent);

		/* TODO: Consider printing directories first */

		list_for_each_entry(item, &n->menu_items, list) {
			if (!item->name)
				die("item->name not specified");
			printf("%s,", item->name);
			if (item->cmd)
				printf("%s", item->cmd);
			if (item->icon_name)
				printf(",%s", item->icon_name);
			printf("\n");
		}
		printf("\n");
	}
}

static void add_app_to_jgmenu_node(const char *node, struct desktop_file_data *data)
{
	struct jgmenu_node *n;
	struct jgmenu_item *item;

	list_for_each_entry(n, &menu_nodes, list) {
		if (strcmp(n->name, node))
			continue;

		item = xmalloc(sizeof(struct jgmenu_item));
		item->name = strdup(data->name);
		item->cmd = strdup(data->exec);
		item->icon_name = strdup(data->icon);
		list_add_tail(&item->list, &n->menu_items);
		return;
	}
}

static void add_dir_to_jgmenu_node(const char *node, const char *name, const char *tag)
{
	struct jgmenu_node *n;
	struct jgmenu_item *item;
	struct sbuf s;

	if (!name || !tag)
		die("XXXXX");

	sbuf_init(&s);
	sbuf_addstr(&s, "^checkout(");
	sbuf_addstr(&s, tag);
	sbuf_addstr(&s, ")");

	list_for_each_entry(n, &menu_nodes, list) {
		if (strcmp(n->name, node))
			continue;

		item = xmalloc(sizeof(struct jgmenu_item));
		item->name = strdup(name);
		item->cmd = strdup(s.buf);
		item->icon_name = strdup("folder");
		list_add_tail(&item->list, &n->menu_items);
		return;
	}
}

static void add_jgmenu_node(const char *content, const char *parent)
{
	struct jgmenu_node *node;

	node = xmalloc(sizeof(struct jgmenu_node));
	node->name = strdup(content);
	node->tag = strdup(content);
	if (parent)
		node->parent = strdup(parent);
	else
		node->parent = NULL;

	list_add_tail(&node->list, &menu_nodes);
	INIT_LIST_HEAD(&node->menu_items);
}

static void build_jgmenu_structure_from_cache(void)
{
	struct cache_entry *ce;
	static char parent[16][1024];
	struct directory_file_data *directory_file;
	struct desktop_file_data *desktop_file;
	struct sbuf *cat;

	list_for_each_entry_reverse(ce, &cache, list) {
		strcpy(parent[ce->level], ce->name);
		if (ce->level > 1)
			add_jgmenu_node(ce->name, parent[ce->level - 1]);
		else
			add_jgmenu_node(ce->name, NULL);

		/* Add .directory file data to parent */
		/* The directory information is only useful to your parent! */
		list_for_each_entry(directory_file, &directory_files, list) {
			if (ce->level &&
			    !strcmp(directory_file->filename, ce->directory)) {
				add_dir_to_jgmenu_node(parent[ce->level - 1],
						       directory_file->name,
						       ce->name);
			}
		}

		/* Add .desktop files filtered on category */
		/*
		 *  TODO: This needs to be changed to categories rather than
		 *       category_includes
		 */
		list_for_each_entry(cat, &ce->category_includes, list) {
			xdgapps_filter_desktop_files_on_category(cat->buf);
			list_for_each_entry(desktop_file, &desktop_files_filtered, filtered_list)
				add_app_to_jgmenu_node(ce->name, desktop_file);
		}
	}
}

/*
 * TODO: Process stuff within <Menu></Menu> to establish categories and
 * .dekstop-files, etc taking into account includes and excludes.
 */
static void process_cache(void)
{
	struct cache_entry *ce;
	struct sbuf *cat;

	if (DEBUG_PRINT_CACHE) {
		printf("PRINT CACHE:\n");
		list_for_each_entry_reverse(ce, &cache, list) {
			printf("Level=%d; Name=%s; Directory=%s\n", ce->level,
			       ce->name, ce->directory);
			list_for_each_entry(cat, &ce->category_includes, list)
				printf("--Categories:%s\n", cat->buf);
		}
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
	struct cache_entry *ce;
	struct sbuf *category;
	static int prev_level;

	/* TODO: Consider deleting iskeyword() */
	if (!iskeyword(node_name))
		return;

	if (list_empty(&cache)) {
		fprintf(stderr, "warning: cache is empty\n");
		return;
	}

	ce = list_first_entry(&cache, struct cache_entry, list);

	/*
	 * If menu_level has decreased (i.e. the </Menu> of a submenu has been reached)
	 * then use the first item in the cache list that matches that level.
	 */
	if (prev_level > menu_level) {
		list_for_each_entry(ce, &cache, list)
			if (ce->level == menu_level)
				break;
		if (DEBUG_PRINT_XML_NODES)
			printf("^");
	}

	if (DEBUG_PRINT_XML_NODES)
		printf("%d-%s: %s\n", menu_level, node_name, content);

	if (!ce->level)
		ce->level = menu_level;

	/* TODO: Lots to be added here */
	if (!strcasecmp(node_name, "Name")) {
		ce->name = strdup(content);
	} else if (!strcasecmp(node_name, "Directory")) {
		ce->directory = strdup(content);
	} else if (!strcasecmp(node_name, "Include.And.Category")) {
		category = xmalloc(sizeof(struct sbuf));
		sbuf_init(category);
		sbuf_addstr(category, content);
		list_add_tail(&category->list, &ce->category_includes);
	}

	prev_level = menu_level;
}

static void create_new_cache_entry(void)
{
	struct cache_entry *ce;

	ce = xcalloc(1, sizeof(struct cache_entry));
	INIT_LIST_HEAD(&ce->category_includes);
	INIT_LIST_HEAD(&ce->category_excludes);
	INIT_LIST_HEAD(&ce->categories);
	INIT_LIST_HEAD(&ce->desktop_files);
	list_add(&ce->list, &cache);
}

static int level(xmlNode *node)
{
	int level = 0;

	for (;;) {
		node = node->parent;
		if (!node || !node->name)
			return level;

		if (!strcasecmp((char *)node->name, "Menu"))
			++level;
	}
}

static void get_full_node_name(struct sbuf *node_name, xmlNode *node)
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
	for (;;) {
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
	struct sbuf node_name;

	/*
	 * Just filtering out a lot of rubbish here for the time being.
	 * We will need to check for <OnlyUnallocated/> etc.
	 */
	if (!node->content)
		return;

	sbuf_init(&node_name);

	get_full_node_name(&node_name, node);

	/* This if statement filters out a lot */
	if (node_name.len || !strlen(strstrip((char *)node->content)))
		add_to_cache(node_name.buf, strstrip((char *)node->content));

	free(node_name.buf);
}

static void xml_tree_walk(xmlNode *node)
{
	xmlNode *n;

	for (n = node; n; n = n->next) {
		if (!strcasecmp((char *)n->name, "Menu")) {
			if (DEBUG_PRINT_XML_NODES)
				printf("---\n");
			menu_level = level(n);
			create_new_cache_entry();
			xml_tree_walk(n->children);
			continue;
		}

		if (!strcasecmp((char *)n->name, "Comment"))
			continue;

		menu_level = level(n);
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
	char *file_name = NULL;
	int i;

	if (argc < 2)
		usage();

	LIBXML_TEST_VERSION

	/* Create lists of .desktop- and .directory files */
	xdgapps_init_lists();

	i = 1;
	while (i < argc) {
		if (argv[i][0] != '-') {
			file_name = argv[i];
			break;
		}

		if (!strncmp(argv[i], "--data-dir=", 11))
			data_dir = argv[i] + 11;
		else
			die("unknown option '%s'", argv[i]);

		i++;
	}

	if (!file_name) {
		fprintf(stderr, "error: no menu-file specified\n");
		usage();
	}

	parse_xml(file_name);
	process_cache();
	build_jgmenu_structure_from_cache();

	print_csv_menu();

	return 0;
}
