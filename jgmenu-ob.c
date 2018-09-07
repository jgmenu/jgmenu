/*
 * jgmenu-ob.c
 *
 * Parses openbox menu and outputs a jgmenu-flavoured CSV file
 */

#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include "util.h"
#include "sbuf.h"
#include "list.h"

static char *root_menu;
char template[] = "temp_jgmenu_ob_XXXXXX";

struct tag {
	char *label;
	char *id;
	struct tag *parent;
	struct list_head items;
	struct list_head list;
};

struct item {
	char *label;
	char *cmd;
	int pipe;
	int checkout;
	struct list_head list;
};

static struct list_head tags;
static struct tag *curtag;
static struct item *curitem;

static void print_it(struct tag *tag)
{
	struct item *item;

	if (list_empty(&tag->items))
		return;
	printf("%s,^tag(%s)\n", tag->label, tag->id);
	if (tag->parent)
		printf("Back,^back()\n");
	list_for_each_entry(item, &tag->items, list) {
		if (item->pipe)
			printf("%s,^pipe(jgmenu_run ob --cmd='%s' --tag='%s')\n",
			       item->label, item->cmd, item->label);
		else if (item->checkout)
			printf("%s,^checkout(%s)\n", item->label, item->cmd);
		else
			printf("%s,%s\n", item->label, item->cmd);
	}
	printf("\n");
}

static void print_menu(void)
{
	struct tag *tag;

	list_for_each_entry(tag, &tags, list)
		if (tag->id && !strcmp(tag->id, root_menu))
			print_it(tag);

	list_for_each_entry(tag, &tags, list)
		if (tag->id && strcmp(tag->id, root_menu))
			print_it(tag);
}

static char *get_tag_label(const char *id)
{
	struct tag *tag;

	if (!id)
		return NULL;
	list_for_each_entry(tag, &tags, list)
		if (tag->id && !strcmp(tag->id, id))
			return tag->label;
	return NULL;
}

static struct tag *get_parent_tag(xmlNode *n)
{
	struct tag *tag;
	char *id = NULL;

	if (!n || !n->parent)
		goto out;

	/* ob pipe-menus don't wrap first level in <menu></menu> */
	if (!strcmp((char *)n->parent->name, "openbox_pipe_menu"))
		id = strdup(root_menu);
	else
		id = (char *)xmlGetProp(n->parent, (const xmlChar *)"id");
	if (!id)
		goto out;
	list_for_each_entry(tag, &tags, list)
		if (tag->id && !strcmp(tag->id, id))
			return tag;
out:
	return NULL;
}

static void new_tag(xmlNode *n);

static void new_item(xmlNode *n)
{
	struct item *item;
	char *label = (char *)xmlGetProp(n, (const xmlChar *)"label");

	if (!curtag)
		new_tag(NULL);

	item = xmalloc(sizeof(struct item));
	item->label = NULL;
	if (label)
		item->label = label;
	item->cmd = NULL;
	item->pipe = 0;
	item->checkout = 0;
	curitem = item;
}

static void new_sep(xmlNode *n)
{
	struct sbuf s;
	char *label = (char *)xmlGetProp(n, (const xmlChar *)"label");

	sbuf_init(&s);
	new_item(n);
	sbuf_cpy(&s, "^sep(");
	if (label)
		sbuf_addstr(&s, label);
	sbuf_addstr(&s, ")");
	curitem->label = strdup(s.buf);
}

static void new_tag(xmlNode *n)
{
	struct tag *t = xcalloc(1, sizeof(struct tag));
	struct tag *parent = get_parent_tag(n);
	char *label = (char *)xmlGetProp(n, (const xmlChar *)"label");
	char *id = (char *)xmlGetProp(n, (const xmlChar *)"id");

	/*
	 * The pipe-menu "root" has no <menu> element and therefore no LABEL
	 * or ID.
	 */
	if (id)
		t->id = id;
	else
		t->id = strdup(root_menu);
	t->label = label;
	t->parent = parent;
	INIT_LIST_HEAD(&t->items);
	list_add_tail(&t->list, &tags);
	curtag = t;

	if (parent && strcmp(id, root_menu) != 0) {
		new_item(n);
		curitem->label = label;
		curitem->cmd = id;
		curitem->checkout = 1;
		list_add_tail(&curitem->list, &curtag->parent->items);
	}
}

static void revert_to_parent(void)
{
	if (curtag && curtag->parent)
		curtag = curtag->parent;
}

static int node_filter(const xmlChar *name)
{
	return strcasecmp((char *)name, "menu");
}

static void get_full_node_name(struct sbuf *node_name, xmlNode *node)
{
	int incl;

	if (!strcmp((char *)node->name, "text")) {
		node = node->parent;
		if (!node || !node->name) {
			fprintf(stderr, "warning: node is root\n");
			return;
		}
	}

	incl = node_filter(node->name);
	for (;;) {
		if (incl)
			sbuf_prepend(node_name, (char *)node->name);
		node = node->parent;
		if (!node || !node->name)
			return;
		incl = node_filter(node->name);
		if (incl)
			sbuf_prepend(node_name, ".");
	}
}

static void process_node(xmlNode *node)
{
	struct sbuf buf;
	struct sbuf node_name;
	char *content = NULL;

	sbuf_init(&buf);
	sbuf_init(&node_name);
	get_full_node_name(&node_name, node);
	if (!node_name.len)
		return;

	if (node->content)
		content = strdup(strstrip((char *)node->content));

	/* <command></command> */
	if (strstr(node_name.buf, "item.action.command") && content)
		curitem->cmd = content;
}

/*
 * <menu> elements can be three things:
 *
 *  - "normal" menu (gets tag). Has ID, LABEL and CONTENT
 *  - "pipe" menu. Has EXECUTE and LABEL
 *  - Link to a menu defined else where. Has ID only.
 */
static int menu_start(xmlNode *n)
{
	int ret = 0;
	char *label;
	char *execute;
	char *id = NULL;

	label = (char *)xmlGetProp(n, (const xmlChar *)"label");
	execute = (char *)xmlGetProp(n, (const xmlChar *)"execute");
	id = (char *)xmlGetProp(n, (const xmlChar *)"id");

	if (label && !execute) {
		/* new ^tag() */
		new_tag(n);
		ret = 1;
	} else if (execute) {
		/* pipe-menu */
		new_item(n);
		curitem->pipe = 1;
		curitem->cmd = execute;
		list_add_tail(&curitem->list, &curtag->items);
	} else if (id) {
		/* checkout a menu defined elsewhere */
		new_item(n);
		curitem->checkout = 1;
		curitem->cmd = id;
		curitem->label = get_tag_label(id);
		list_add_tail(&curitem->list, &curtag->items);
	}

	return ret;
}

static void xml_tree_walk(xmlNode *node)
{
	xmlNode *n;
	int ret;

	for (n = node; n && n->name; n = n->next) {
		if (!strcasecmp((char *)n->name, "menu")) {
			ret = menu_start(n);
			xml_tree_walk(n->children);
			if (ret)
				revert_to_parent();
			continue;
		}
		if (!strcasecmp((char *)n->name, "item")) {
			new_item(n);
			list_add_tail(&curitem->list, &curtag->items);
			xml_tree_walk(n->children);
			continue;
		}
		if (!strcasecmp((char *)n->name, "separator")) {
			new_sep(n);
			list_add_tail(&curitem->list, &curtag->items);
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
		exit(1);
	xml_tree_walk(xmlDocGetRootElement(d));
	xmlFreeDoc(d);
	xmlCleanupParser();
}

void read_command(const char *cmd, char *template)
{
	char buf[BUFSIZ];
	int link[2];
	int fd;
	ssize_t cnt;

	if (pipe(link) == -1)
		die("pipe");

	chdir("/tmp");
	fd = mkstemp(template);
	if (fd < 0)
		die("unable to create tempfile");
	switch (fork()) {
	case -1:
		die("fork");
		break;
	case 0:
		if (close(link[0] == -1))
			warn("close 1");
		if (dup2(link[1], STDOUT_FILENO) == -1)
			die("dup2");
		execl("/bin/sh", "/bin/sh", "-c", cmd, NULL);
		break;
	default:
		break;
	}
	if (close(link[1]) == -1)
		warn("close 3");
	while ((cnt = read(link[0], buf, sizeof(buf))) > 0) {
		if (write(fd, buf, cnt) != cnt)
			warn("bad write to temp file '%s'", template);
	}
	if (close(link[0]) == -1)
		warn("close 4");
	if (wait(NULL) == -1)
		warn("wait");
}

static void unlink_temp_file(void)
{
	unlink(template);
}

static void cleanup(void)
{
	xfree(root_menu);
}

void handle_argument_clash(void)
{
	die("both --cmd=<cmd> and <file> provided");
}

int main(int argc, char **argv)
{
	char *filename = NULL;
	int i;
	struct sbuf default_file;
	struct stat sb;

	atexit(cleanup);
	LIBXML_TEST_VERSION

	i = 1;
	while (i < argc) {
		if (argv[i][0] != '-') {
			if (filename)
				handle_argument_clash();
			filename = xstrdup(argv[i]);
			if (argc > i + 1)
				die("<file> must be the last argument");
			break;
		} else if (!strncmp(argv[i], "--tag=", 6)) {
			root_menu = strdup(argv[i] + 6);
		} else if (!strncmp(argv[i], "--cmd=", 6)) {
			if (filename)
				handle_argument_clash();
			read_command(argv[i] + 6, template);
			atexit(unlink_temp_file);
			filename = xstrdup(template);
		}
		i++;
	}
	if (!root_menu)
		root_menu = strdup("root-menu");
	if (!filename) {
		sbuf_init(&default_file);
		sbuf_cpy(&default_file, getenv("HOME"));
		sbuf_addstr(&default_file, "/.config/openbox/menu.xml");
		filename = strdup(default_file.buf);
		xfree(default_file.buf);
	}
	if (stat(filename, &sb))
		die("file '%s' does not exist", filename);
	INIT_LIST_HEAD(&tags);
	parse_xml(filename);

	print_menu();
	xfree(filename);

	return 0;
}
