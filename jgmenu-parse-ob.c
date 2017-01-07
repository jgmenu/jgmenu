/*
 * jgmenu-parse-ob.c
 *
 * Parses openbox menu and outputs a jgmenu-flavoured CSV file
 */

#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <sys/stat.h>

#include "util.h"
#include "sbuf.h"
#include "list.h"

static const char jgmenu_xdg_usage[] =
"Usage: jgmenu_run parse-ob\n";

struct ob_item {
	struct sbuf label;	/* <item label=""> */
	struct sbuf cmd;	/* <command></command> */
	int execute;		/* <action name="execute"> */
};

static struct ob_item *ob_item;

/*
 * List to keep track of the "labels" associated with each "id".
 * This is needed for ^checkout() items
 */
struct menu_labels {
	char *label;
	char *id;
	struct list_head list;
};

static struct list_head menu_labels;

static struct sbuf rootmenu;
static struct sbuf submenus;
static struct sbuf *buf;

static void usage(void)
{
	printf("%s", jgmenu_xdg_usage);
	exit(0);
}

static void ob_item_init(void)
{
	ob_item = xmalloc(sizeof(struct ob_item));
	sbuf_init(&ob_item->label);
	sbuf_init(&ob_item->cmd);
	ob_item->execute = 0;
}

static void ob_item_reset(void)
{
	sbuf_cpy(&ob_item->label, "");
	sbuf_cpy(&ob_item->cmd, "");
	ob_item->execute = 0;
}

static void ob_item_print(void)
{
	if (!ob_item->label.len)
		return;
	sbuf_addstr(buf, ob_item->label.buf);
	sbuf_addstr(buf, ",");
	sbuf_addstr(buf, ob_item->cmd.buf);
	sbuf_addstr(buf, "\n");
}

static void init_globals(void)
{
	ob_item_init();
	sbuf_init(&rootmenu);
	sbuf_init(&submenus);
	buf = &submenus;
	INIT_LIST_HEAD(&menu_labels);
}

static void get_full_node_name(struct sbuf *node_name, xmlNode *node)
{
	if (!strcmp((char *)node->name, "text")) {
		node = node->parent;
		if (!node || !node->name) {
			fprintf(stderr, "warning: node is root\n");
			return;
		}
	}

	for (;;) {
		sbuf_prepend(node_name, (char *)node->name);
		node = node->parent;
		if (!node || !node->name)
			return;
		sbuf_prepend(node_name, ".");
	}
}

static void add_label(char *label, char *id)
{
	struct menu_labels *ml;

	ml = xcalloc(1, sizeof(struct menu_labels));
	if (label)
		ml->label = strdup(label);
	if (id)
		ml->id = strdup(id);
	list_add_tail(&ml->list, &menu_labels);
}

static char *get_label(char *id)
{
	struct menu_labels *iter;
	char *ret = NULL;

	if (!id)
		goto out;
	list_for_each_entry(iter, &menu_labels, list) {
		if (!strcmp(id, iter->id)) {
			ret = iter->label;
			break;
		}
	}
out:
	return ret;
}

/*
 * This is where we do most of the work.
 */
static void process_node(xmlNode *node)
{
	struct sbuf node_name;
	char *content = NULL;
	xmlChar *id = NULL;
	xmlChar *label = NULL;
	xmlChar *name = NULL;
	xmlChar *execute = NULL;

	sbuf_init(&node_name);
	get_full_node_name(&node_name, node);
	if (!node_name.len)
		return;

	if (node->content)
		content = strdup(strstrip((char *)node->content));

	id = xmlGetProp(node, (const xmlChar *)"id");
	label = xmlGetProp(node, (const xmlChar *)"label");
	name = xmlGetProp(node, (const xmlChar *)"name");
	execute = xmlGetProp(node, (const xmlChar *)"execute");

	if (id && strstr((char *)node->name, "menu")) {
		if (!strcmp(node_name.buf, "openbox_menu.menu")) {
			/* tagged menus */
			ob_item_print();
			if (strstr((char *)id, "root-menu"))
				buf = &rootmenu;
			else
				buf = &submenus;
			add_label((char *)label, (char *)id);
			sbuf_addstr(buf, "\n");
			sbuf_addstr(buf, (char *)label);
			sbuf_addstr(buf, ",^tag(");
			sbuf_addstr(buf, (char *)id);
			sbuf_addstr(buf, ")\n");
		} else if (execute) {
			/* pipemenus */
			sbuf_addstr(buf, (char *)label);
			sbuf_addstr(buf, ",f=$(mktemp); ");
			sbuf_addstr(buf, (char *)execute);
			sbuf_addstr(buf, " >$f; jgmenu_run ob $f; rm -f $f\n");
		} else {
			/* item checking out a menu */
			sbuf_addstr(buf, (char *)get_label((char *)id));
			sbuf_addstr(buf, ",^checkout(");
			sbuf_addstr(buf, (char *)id);
			sbuf_addstr(buf, ")\n");
		}
	}

	/* <action name="execute"> */
	if (!strstr(node_name.buf, "action") && name &&
	    !strcasecmp((const char *)name, "execute"))
		ob_item->execute = 1;

	/* <command></command> */
	if (strstr(node_name.buf, "command") && content)
		sbuf_cpy(&ob_item->cmd, content);

	free(node_name.buf);
}

static void xml_tree_walk(xmlNode *node)
{
	xmlNode *n;
	xmlChar *label;

	for (n = node; n; n = n->next) {
		/* <item label="foo"> */
		label = xmlGetProp(n, (const xmlChar *)"label");
		if (!strcasecmp((char *)n->name, "item") && label) {
			sbuf_cpy(&ob_item->label, (char *)xmlGetProp(n, (const xmlChar *)"label"));
			xml_tree_walk(n->children);
			ob_item_print();
			ob_item_reset();
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
	char *file_name = NULL;
	int i;
	struct sbuf default_file;
	struct stat sb;
	int exists;

	LIBXML_TEST_VERSION

	i = 1;
	while (i < argc) {
		if (argv[i][0] != '-') {
			file_name = argv[i];
			break;
		}

		else if (!strncmp(argv[i], "--help", 6))
			usage();
		else
			die("unknown option '%s'", argv[i]);

		i++;
	}

	sbuf_init(&default_file);
	if (!file_name) {
		sbuf_addstr(&default_file, getenv("HOME"));
		sbuf_addstr(&default_file, "/.config/openbox/menu.xml");
		file_name = strdup(default_file.buf);
	}
	exists = stat(file_name, &sb) == 0;
	if (!exists)
		die("file '%s' does not exist", file_name);

	init_globals();
	parse_xml(file_name);
	printf("%s\n", rootmenu.buf);
	printf("%s\n", submenus.buf);

	return 0;
}
