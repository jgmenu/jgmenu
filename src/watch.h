#ifndef WATCH_H
#define WATCH_H

extern void watch_init(void);
extern int watch_files_have_changed(void);
extern void watch_cleanup(void);

#endif /* WATCH_H */
