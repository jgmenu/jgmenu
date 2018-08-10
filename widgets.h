#ifndef WIDGETS_H
#define WIDGETS_H

extern int widgets_mouseover(void);
extern void widgets_set_pointer_position(int x, int y);
extern char *widgets_get_mouseover_action(void);
extern void widgets_draw(void);
extern void widgets_add(const char *s);
extern void widgets_cleanup(void);

#endif /* WIDGETS_H */
