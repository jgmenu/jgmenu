#ifndef PM_H
#define PM_H

void pm_push(void *pipe_node, void *parent_node);
int pm_is_pipe_node(void *node);
void pm_pop(void);
void *pm_first_pipemenu_node(void);
void pm_cleanup(void);

#endif /* PM_H */
