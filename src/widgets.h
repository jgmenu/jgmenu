#ifndef WIDGETS_H
#define WIDGETS_H

void widgets_select(const char *ksym);
int widgets_get_kb_grabbed(void);
void widgets_toggle_kb_grabbed(void);
int widgets_mouseover(void);
void widgets_set_pointer_position(int x, int y);
char *widgets_get_selection_action(void);
void widgets_draw(void);
void widgets_add(const char *s);
void widgets_cleanup(void);

#endif /* WIDGETS_H */
