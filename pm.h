#ifndef PM_H
#define PM_H

extern void pm_push(void *pipe_node, void *parent_node);
extern int pm_is_pipe_node(void *node);
extern void pm_pop(void);
extern void *pm_first_pipemenu_node(void);
extern void pm_cleanup(void);

#endif /* PM_H */
