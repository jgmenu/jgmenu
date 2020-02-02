/*
 * jgmenu-ob.c
 *
 * Parses openbox menu and outputs a jgmenu-flavoured CSV file
 */

#include <stdio.h>
#include <string.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <unistd.h>
#include <fcntl.h>

#include "util.h"
#include "sbuf.h"
#include "list.h"
#include "i18n.h"
#include "banned.h"

static const char reconfigure_command[] = "openbox --reconfigure";
static const char restart_command[] = "openbox --restart";
static const char root_menu_default[] = "root-menu";
static char *root_menu = (char *)root_menu_default;
static char *i18nfile;
static int ispipemenu;

struct tag {
	char *label;
	char *id;
	struct tag *parent;
	struct list_head items;
	struct list_head list;
};

struct item {
	char *label;
	char *icon;
	char *cmd;
	int pipe;
	int checkout;
	int isseparator;
	struct list_head list;
};

static struct list_head tags;
static struct tag *curtag;
static struct item *curitem;

static void print_one_node(struct tag *tag)
{
	struct item *item;
	struct sbuf label_escaped;
	char *t9n;

	if (list_empty(&tag->items))
		return;
	if (ispipemenu && !strcmp(tag->id, root_menu))
		goto dont_print_tag;
	printf("^tag(%s)\n", tag->id);
dont_print_tag:
	if (tag->parent)
		printf("Back,^back()\n");
	list_for_each_entry(item, &tag->items, list) {
		t9n = NULL;
		if (i18nfile && item->label)
			t9n = i18n_translate(item->label);
		sbuf_init(&label_escaped);
		sbuf_cpy(&label_escaped, t9n ? t9n : item->label);
		sbuf_replace(&label_escaped, "&", "&amp;");
		sbuf_replace(&label_escaped, "<", "&lt;");
		sbuf_replace(&label_escaped, ">", "&gt;");
		sbuf_replace_spaces_with_one_tab(&label_escaped);
		if (strchr(label_escaped.buf, ',')) {
			sbuf_prepend(&label_escaped, "\"\"\"");
			sbuf_addstr(&label_escaped, "\"\"\"");
		}
		if (item->pipe) {
			/* Use double quotes to support bl-places-pipemenu */
			printf("%s,^pipe(jgmenu_run ob --cmd=\"%s\" --tag=\"%s\")",
			       label_escaped.buf, item->cmd, item->label);
			if (item->icon)
				printf(",%s", item->icon);
			printf("\n");
		} else if (item->checkout) {
			printf("%s,^checkout(%s)", label_escaped.buf, item->cmd);
			if (item->icon)
				printf(",%s", item->icon);
			printf("\n");
		} else if (item->isseparator) {
			printf("^sep(%s)\n", label_escaped.buf);
		} else {
			printf("%s,%s", label_escaped.buf, item->cmd);
			if (item->icon)
				printf(",%s", item->icon);
			printf("\n");
		}
		xfree(label_escaped.buf);
	}
	printf("\n");
}

static void print_menu(void)
{
	struct tag *tag;

	/* print root menu first */
	list_for_each_entry(tag, &tags, list)
		if (tag->id && !strcmp(tag->id, root_menu))
			print_one_node(tag);

	/* then print all other nodes */
	list_for_each_entry(tag, &tags, list)
		if (tag->id && strcmp(tag->id, root_menu))
			print_one_node(tag);
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
		return NULL;
	/* ob pipe-menus don't wrap first level in <menu></menu> */
	if (!strcmp((char *)n->parent->name, "openbox_pipe_menu"))
		id = xstrdup(root_menu);
	else
		id = (char *)xmlGetProp(n->parent, (const xmlChar *)"id");
	if (!id)
		return NULL;
	list_for_each_entry(tag, &tags, list) {
		if (tag->id && !strcmp(tag->id, id)) {
			xfree(id);
			return tag;
		}
	}
	xfree(id);
	return NULL;
}

static void new_tag(xmlNode *n);

static void new_item(xmlNode *n, int isseparator)
{
	struct item *item;

	if (!curtag)
		new_tag(NULL);
	item = xmalloc(sizeof(struct item));
	item->label = (char *)xmlGetProp(n, (const xmlChar *)"label");
	item->icon = (char *)xmlGetProp(n, (const xmlChar *)"icon");
	item->cmd = NULL;
	item->pipe = 0;
	item->checkout = 0;
	if (isseparator)
		item->isseparator = 1;
	else
		item->isseparator = 0;
	curitem = item;
}

/**
 *  new_tag - create new tag (to generate ^tag() markup)
 *  @n - <menu label="" id=""></menu> node to create tag against
 *       If NULL is provided, we create a root-menu tag
 */
static void new_tag(xmlNode *n)
{
	struct tag *t = xcalloc(1, sizeof(struct tag));
	struct tag *parent = get_parent_tag(n);
	char *label = (char *)xmlGetProp(n, (const xmlChar *)"label");
	char *icon = (char *)xmlGetProp(n, (const xmlChar *)"icon");
	char *id = (char *)xmlGetProp(n, (const xmlChar *)"id");

	/*
	 * The pipe-menu "root" has no <menu> element and therefore no
	 * LABEL or ID.
	 */
	if (id)
		t->id = xstrdup(id);
	else
		t->id = xstrdup(root_menu);
	t->label = xstrdup(label);
	t->parent = parent;
	INIT_LIST_HEAD(&t->items);
	list_add_tail(&t->list, &tags);
	curtag = t;

	/* Create an item to generate the ^checkout() markup */
	if (parent) {
		new_item(n, 0);
		xfree(curitem->label);
		curitem->label = xstrdup(label);
		xfree(curitem->icon);
		curitem->icon = xstrdup(icon);
		curitem->cmd = xstrdup(id);
		curitem->checkout = 1;
		list_add_tail(&curitem->list, &curtag->parent->items);
	}
	xmlFree(label);
	xmlFree(icon);
	xmlFree(id);
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

static void get_special_action(xmlNode *node, char **cmd)
{
	char *action;

	action = (char *)xmlGetProp(node, (const xmlChar *)"name");
	if (!action)
		goto out;
	if (!strcasecmp(action, "Execute"))
		goto out;
	if (!strcasecmp(action, "reconfigure"))
		*cmd = xstrdup(reconfigure_command);
	else if (!strcasecmp(action, "restart"))
		*cmd = xstrdup(restart_command);
out:
	if (action)
		xmlFree(action);
}

/*
 * In an openbox menu xml file, a menu item (such as a program) is written as
 * follows:
 *
 * <item label="foo">
 *   <action name="Execute">
 *     <command>foo</command>
 *   </action>
 * </item>
 *
 * The tag <execute> can be used instead of <command>. The openbox.org wiki
 * says that the <execute> tag is depreciated, but one of the examples on the
 * wiki menu page uses <execute> and so does the /etc/xdg/openbox/menu.xml.
 *
 * http://openbox.org/wiki/Help:Menus
 * http://openbox.org/wiki/Help:Actions#Action_syntax
 */
static void process_node(xmlNode *node)
{
	struct sbuf buf;
	struct sbuf node_name;

	sbuf_init(&buf);
	sbuf_init(&node_name);
	get_full_node_name(&node_name, node);
	if (!node_name.len)
		goto clean;
	if (!strstr(node_name.buf, "item.action"))
		goto clean;

	if (strstr(node_name.buf, "item.action.execute") ||
	    strstr(node_name.buf, "item.action.command")) {
		xmlChar *c = xmlNodeGetContent(node);

		xfree(curitem->cmd);
		curitem->cmd = c ? xstrdup(strstrip((char *)c)) : NULL;
		xmlFree(c);
	}

	/* Catch <action name="Reconfigure"> and <action name="Restart"> */
	get_special_action(node, &curitem->cmd);

clean:
	xfree(buf.buf);
	xfree(node_name.buf);
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
	int has_parent = 0;
	char *label;
	char *execute;
	char *id;

	label = (char *)xmlGetProp(n, (const xmlChar *)"label");
	execute = (char *)xmlGetProp(n, (const xmlChar *)"execute");
	id = (char *)xmlGetProp(n, (const xmlChar *)"id");

	if (label && !execute) {
		/* new ^tag() */
		new_tag(n);
		has_parent = 1;
	} else if (execute) {
		/* pipe-menu */
		new_item(n, 0);
		curitem->pipe = 1;
		curitem->cmd = xstrdup(execute);
		list_add_tail(&curitem->list, &curtag->items);
	} else if (id) {
		/* checkout a menu defined elsewhere */
		new_item(n, 0);
		curitem->checkout = 1;
		curitem->cmd = xstrdup(id);
		xfree(curitem->label);
		curitem->label = xstrdup(get_tag_label(id));
		list_add_tail(&curitem->list, &curtag->items);
	}
	xfree(label);
	xfree(execute);
	xfree(id);
	return has_parent;
}

static void xml_tree_walk(xmlNode *node)
{
	xmlNode *n;
	int has_parent;

	for (n = node; n && n->name; n = n->next) {
		if (!strcasecmp((char *)n->name, "menu")) {
			has_parent = menu_start(n);
			xml_tree_walk(n->children);
			if (has_parent)
				revert_to_parent();
			continue;
		}
		if (!strcasecmp((char *)n->name, "item")) {
			new_item(n, 0);
			list_add_tail(&curitem->list, &curtag->items);
			xml_tree_walk(n->children);
			continue;
		}
		if (!strcasecmp((char *)n->name, "separator")) {
			new_item(n, 1);
			list_add_tail(&curitem->list, &curtag->items);
			xml_tree_walk(n->children);
			continue;
		}
		if (!strcasecmp((char *)n->name, "Comment"))
			continue;
		if (!strcasecmp((char *)n->name, "openbox_pipe_menu")) {
			if (!curtag)
				new_tag(NULL);
			ispipemenu = 1;
		}

		process_node(n);
		xml_tree_walk(n->children);
	}
}

static void parse_xml(struct sbuf *xmlbuf)
{
	xmlDoc *d;

	d = xmlParseMemory(xmlbuf->buf, strlen(xmlbuf->buf));
	if (!d)
		exit(1);
	xml_tree_walk(xmlDocGetRootElement(d));
	print_menu();
	xmlFreeDoc(d);
	xmlCleanupParser();
}

static void cleanup(void)
{
	struct tag *tag, *tag_tmp;
	struct item *item, *i_tmp;

	list_for_each_entry(tag, &tags, list) {
		list_for_each_entry(item, &tag->items, list) {
			xfree(item->label);
			xfree(item->icon);
			xfree(item->cmd);
		}
	}
	list_for_each_entry(tag, &tags, list) {
		xfree(tag->id);
		xfree(tag->label);
	}
	list_for_each_entry(tag, &tags, list) {
		list_for_each_entry_safe(item, i_tmp, &tag->items, list) {
			list_del(&item->list);
			xfree(item);
		}
	}
	list_for_each_entry_safe(tag, tag_tmp, &tags, list) {
		list_del(&tag->list);
		xfree(tag);
	}
}

static void handle_argument_clash(void)
{
	die("both --cmd=<cmd> and <file> provided");
}

static void init_i18n(void)
{
	i18nfile = getenv("JGMENU_I18N");
	if (i18nfile)
		i18nfile = i18n_set_translation_file(i18nfile);
}

int main(int argc, char **argv)
{
	int i;
	struct sbuf default_file;
	FILE *fp = NULL;
	struct sbuf xmlbuf;
	char buf[BUFSIZ], *p;

	atexit(cleanup);
	INIT_LIST_HEAD(&tags);
	LIBXML_TEST_VERSION

	i = 1;
	while (i < argc) {
		if (argv[i][0] != '-') {
			if (argc > i + 1)
				die("<file> must be the last argument");
			if (fp)
				handle_argument_clash();
			fp = fopen(argv[i], "r");
			if (!fp)
				die("ob: cannot open file '%s'", argv[i]);
		} else if (!strncmp(argv[i], "--tag=", 6)) {
			root_menu = argv[i] + 6;
		} else if (!strncmp(argv[i], "--cmd=", 6)) {
			fp = popen(argv[i] + 6, "r");
			if (!fp)
				die("ob: cannot run command '%s'", argv[i] + 6);
		}
		i++;
	}
	if (!fp) {
		sbuf_init(&default_file);
		sbuf_cpy(&default_file, getenv("HOME"));
		sbuf_addstr(&default_file, "/.config/openbox/menu.xml");
		fp = fopen(default_file.buf, "r");
		if (fp)
			goto out;
		sbuf_cpy(&default_file, "/etc/xdg/openbox/menu.xml");
		fp = fopen(default_file.buf, "r");
out:
		xfree(default_file.buf);
	}
	if (!fp)
		die("ob: cannot open openbox menu file");
	sbuf_init(&xmlbuf);
	for (i = 0; fgets(buf, sizeof(buf), fp); i++) {
		buf[BUFSIZ - 1] = '\0';
		p = strrchr(buf, '\n');
		if (p)
			*p = '\0';
		sbuf_addstr(&xmlbuf, buf);
	}
	init_i18n();
	parse_xml(&xmlbuf);
	xfree(xmlbuf.buf);
	if (i18nfile)
		i18n_cleanup();

	return 0;
}
