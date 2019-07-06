/*
 * jgmenu-xdg.c
 *
 * Parses XML .menu file and outputs a csv formatted jgmenu file
 *
 * See this spec for further details:
 * https://specifications.freedesktop.org/menu-spec/menu-spec-1.0.html
 *
 * Generate jgmenu flavoured CSV menu data for an XDGish menu
 *
 * `jgmenu_run xdg` \[--no-dirs] \[<*.menu file*>]
 *
 * `jgmenu_run xdg` generates jgmenu flavoured CSV menu data for a menu
 * based on XML .menu files loosely in accordance with the XDG spec:
 *
 * http://standards.freedesktop.org/menu-spec/
 * http://standards.freedesktop.org/basedir-spec/
 * http://standards.freedesktop.org/desktop-entry-spec/
 * http://standards.freedesktop.org/desktop-entry-spec/
 *
 * `jgmenu_run xdg` is a very simple XDG implementation.
 * It understands the XML elements <*Menu*>, <*Name*>, <*Directory*>
 * and <*Include*><*And*><*Category*>, but ignores everything else.
 *
 * The .menu file is sought in `${XDG_CONFIG_DIRS:-/etc/xdg}` with
 * user configuration override in `${XDG_CONFIG_HOME:-$HOME/.config}`
 *
 * `$XDG_MENU_PREFIX` can be used to specity a .menu file. For example
 * `$XDG_MENU_PREFIX=lxde-` will load lxde-applications.menu
 * This can be useful if there are several .menu files on the system.
 *
 * \--no-dirs
 * :   ignore .menu and .directory files
 */

#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <sys/stat.h>

#include "xdgapps.h"
#include "util.h"
#include "sbuf.h"
#include "xdgdirs.h"

#define DEBUG_PRINT_XML_NODES 0

/*
 * In jgmenu-speak:
 *	- A "node" is a root-menu or a submenu.
 *	  It corresponds roughly to an XML <Menu> element.
 *	- An "item" is a individual menu entry (with name, command and icon)
 *	  These are defined by "categories", .desktop-files, etc
 */
struct jgmenu_node {
	char *name;
	char *tag;
	char *directory;
	int level;
	struct jgmenu_node *parent;
	struct list_head category_includes;
	struct list_head category_excludes;
	struct list_head categories;
	struct list_head desktop_files;
	struct list_head menu_items;
	struct list_head list;
};

struct jgmenu_item {
	char *name;
	char *cmd;
	char *icon_name;
	struct list_head list;
};

static LIST_HEAD(jgmenu_nodes);
static struct jgmenu_node *current_jgmenu_node;

static void print_menu_item(struct jgmenu_item *item)
{
	printf("%s,", item->name);
	if (item->cmd)
		printf("%s", item->cmd);
	if (item->icon_name)
		printf(",%s", item->icon_name);
	printf("\n");
}

static void print_csv_menu(void)
{
	struct jgmenu_node *n;
	struct jgmenu_item *item;

	list_for_each_entry(n, &jgmenu_nodes, list) {
		if (list_empty(&n->menu_items))
			continue;
		if (!n->parent) {
			cat("~/.config/jgmenu/prepend.csv");
		} else {
			printf("%s,^tag(%s)\n", n->name, n->tag);
			printf("Go back,^back(),folder\n");
		}
		/* Print directories first */
		list_for_each_entry(item, &n->menu_items, list)
			if (item->cmd && !strncmp(item->cmd, "^checkout", 9))
				print_menu_item(item);
		/* Then all other items */
		list_for_each_entry(item, &n->menu_items, list)
			if (!item->cmd || strncmp(item->cmd, "^checkout", 9))
				print_menu_item(item);
		printf("\n");
		if (!n->parent)
			cat("~/.config/jgmenu/append.csv");
	}
}

static void init_jgmenu_item(struct jgmenu_item *item)
{
	item->name = NULL;
	item->cmd = NULL;
	item->icon_name = NULL;
}

static void add_item_to_jgmenu_node(struct jgmenu_node *node,
				    struct desktop_file_data *data)
{
	struct jgmenu_item *item;

	item = xmalloc(sizeof(struct jgmenu_item));
	init_jgmenu_item(item);
	if (data->name)
		item->name = strdup(data->name);
	if (data->exec)
		item->cmd = strdup(strstrip(data->exec));
	if (data->icon)
		item->icon_name = strdup(data->icon);
	list_add_tail(&item->list, &node->menu_items);
}

static void add_dir_to_jgmenu_node(struct jgmenu_node *parent,
				   const char *name,
				   const char *tag)
{
	struct jgmenu_item *item;
	struct sbuf s;

	if (!name || !tag)
		die("no name or tag specified in add_dir_to_jgmenu_node()");
	if (!parent)
		return;
	sbuf_init(&s);
	sbuf_addstr(&s, "^checkout(");
	sbuf_addstr(&s, tag);
	sbuf_addstr(&s, ")");

	item = xmalloc(sizeof(struct jgmenu_item));
	init_jgmenu_item(item);
	item->name = strdup(name);
	item->cmd = strdup(s.buf);
	item->icon_name = strdup("folder");
	list_add_tail(&item->list, &parent->menu_items);
}

/* TODO: This needs to be changed to categories rather than category_includes */
static void process_categories_and_populate_desktop_files(void)
{
	struct jgmenu_node *jgmenu_node;
	struct desktop_file_data *desktop_file;
	struct sbuf *cat;

	list_for_each_entry(jgmenu_node, &jgmenu_nodes, list) {
		list_for_each_entry(cat, &jgmenu_node->category_includes, list) {
			xdgapps_filter_desktop_files_on_category(cat->buf);
			list_for_each_entry(desktop_file, &desktop_files_filtered, filtered_list)
				add_item_to_jgmenu_node(jgmenu_node, desktop_file);
		}
	}
}

/*
 * For each directory within a <Menu></Menu>, a menu item is added to the
 * _parent_ jgmenu node. This is slighly counter intuitive, but it's just
 * how it is.
 *
 * TODO: Throw error if .directory file does not exist
 */
static void process_directory_files(void)
{
	struct jgmenu_node *jgmenu_node;
	struct directory_file_data *directory_file;

	list_for_each_entry(jgmenu_node, &jgmenu_nodes, list) {
		if (list_empty(&jgmenu_node->menu_items))
			continue;

		list_for_each_entry(directory_file, &directory_files, list)
			if (!strcmp(directory_file->filename, jgmenu_node->directory))
				add_dir_to_jgmenu_node(jgmenu_node->parent, jgmenu_node->name, jgmenu_node->tag);
	}
}

static void add_data_to_jgmenu_node(const char *xml_node_name,
				    const char *content)
{
	struct jgmenu_node *jgmenu_node;
	struct sbuf *category;

	jgmenu_node = current_jgmenu_node;

	/* TODO: Lots to be added here */
	/* TODO: Check for tag duplicates */
	if (!strcasecmp(xml_node_name, "Name")) {
		jgmenu_node->name = strdup(content);
		jgmenu_node->tag = strdup(content);
	} else if (!strcasecmp(xml_node_name, "Directory")) {
		jgmenu_node->directory = strdup(content);
	} else if (!strcasecmp(xml_node_name, "Include.And.Category")) {
		category = xmalloc(sizeof(struct sbuf));
		sbuf_init(category);
		sbuf_addstr(category, content);
		list_add_tail(&category->list, &jgmenu_node->category_includes);
	}

	if (DEBUG_PRINT_XML_NODES)
		printf("%d-%s: %s\n", jgmenu_node->level, xml_node_name, content);
}

static void create_new_jgmenu_node(struct jgmenu_node *parent,
				   int level)
{
	struct jgmenu_node *node;

	if (DEBUG_PRINT_XML_NODES)
		printf("---\n");

	node = xcalloc(1, sizeof(struct jgmenu_node));
	node->name = NULL;
	node->tag = NULL;
	node->directory = NULL;
	node->level = level;

	if (parent)
		node->parent = parent;
	else
		node->parent = NULL;

	INIT_LIST_HEAD(&node->category_includes);
	INIT_LIST_HEAD(&node->category_excludes);
	INIT_LIST_HEAD(&node->categories);
	INIT_LIST_HEAD(&node->desktop_files);
	INIT_LIST_HEAD(&node->menu_items);
	list_add_tail(&node->list, &jgmenu_nodes);
	current_jgmenu_node = node;
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

/*
 * Gives the node name back to the parent "Menu" element.
 * For example, it would convert
 * <Include><And><Category></Category></And></Include> to
 * "Include.And.Category"
 */
static void get_full_node_name(struct sbuf *node_name, xmlNode *node)
{
	int ismenu;

	if (!strcmp((char *)node->name, "text")) {
		node = node->parent;
		if (!node || !node->name) {
			fprintf(stderr, "warning: node is root\n");
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
	if (node_name.len && strlen(strstrip((char *)node->content)))
		add_data_to_jgmenu_node(node_name.buf, strstrip((char *)node->content));
	free(node_name.buf);
}

static void revert_to_parent(void)
{
	if (current_jgmenu_node && current_jgmenu_node->parent)
		current_jgmenu_node = current_jgmenu_node->parent;
}

/*
 * n->next refers to siblings
 * n->children is obviously the children
 */
static void xml_tree_walk(xmlNode *node)
{
	xmlNode *n;

	for (n = node; n; n = n->next) {
		if (!strcasecmp((char *)n->name, "Menu")) {
			create_new_jgmenu_node(current_jgmenu_node, level(n));
			xml_tree_walk(n->children);
			revert_to_parent();
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

static void print_desktop_files(void)
{
	struct desktop_file_data *f;

	cat("~/.config/jgmenu/prepend.csv");
	list_for_each_entry(f, &desktop_files_all, full_list)
		if (f->name)
			printf("%s,%s,%s\n", f->name, f->exec, f->icon);
	cat("~/.config/jgmenu/append.csv");
}

int main(int argc, char **argv)
{
	struct sbuf filename;
	int i;
	int no_dirs = 0;

	sbuf_init(&filename);
	LIBXML_TEST_VERSION

	/* Create lists of .desktop- and .directory files */
	xdgapps_init_lists();

	i = 1;
	while (i < argc) {
		if (argv[i][0] != '-')
			sbuf_cpy(&filename, argv[i]);
		else if (!strncmp(argv[i], "--no-dirs", 9))
			no_dirs = 1;
		else
			die("unknown option '%s'", argv[i]);
		i++;
	}
	if (no_dirs) {
		print_desktop_files();
		goto out;
	}
	if (!filename.len)
		xdgdirs_find_menu_file(&filename);
	if (!filename.len)
		die("cannot find menu-file");
	info("parsing menu file '%s'", filename.buf);
	parse_xml(filename.buf);
	process_categories_and_populate_desktop_files();
	process_directory_files();
	print_csv_menu();
out:
	xfree(filename.buf);
	return 0;
}
