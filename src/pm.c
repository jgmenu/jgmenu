/*
 * pm.c
 *
 * Copyright (C) Johan Malm 2017
 *
 * Helpers for managing pipemenus
 */

#include <stdio.h>
#include <stdlib.h>

#include "util.h"
#include "list.h"
#include "pm.h"
#include "banned.h"

static LIST_HEAD(pipe_stack);
static int level;

struct pm {
	int level;
	void *pipe_node;
	void *parent_node;
	struct list_head list;
};

void pm_push(void *pipe_node, void *parent_node)
{
	struct pm *pm;

	pm = xmalloc(sizeof(struct pm));
	pm->level = ++level;
	pm->pipe_node = pipe_node;
	pm->parent_node = parent_node;
	list_add(&pm->list, &pipe_stack);
}

int pm_is_pipe_node(void *node)
{
	struct pm *pm;

	list_for_each_entry(pm, &pipe_stack, list)
		if (node == pm->pipe_node)
			return 1;
	return 0;
}

void pm_pop(void)
{
	struct pm *pm;

	pm = list_first_entry_or_null(&pipe_stack, struct pm, list);
	if (!pm)
		die("pm_is_pop(): no pipemenu left in stack");
	list_del(&pm->list);
	xfree(pm);
	--level;
}

void *pm_first_pipemenu_node(void)
{
	struct pm *pm;

	if (list_empty(&pipe_stack))
		return NULL;
	pm = list_last_entry(&pipe_stack, struct pm, list);
	return (void *)pm->pipe_node;
}

void pm_cleanup(void)
{
	struct pm *pm, *tmp_pm;

	list_for_each_entry_safe(pm, tmp_pm, &pipe_stack, list) {
		list_del(&pm->list);
		xfree(pm);
	}
	level = 0;
}
