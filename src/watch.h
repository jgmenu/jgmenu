#ifndef WATCH_H
#define WATCH_H

void watch_init(void);
int watch_files_have_changed(void);
void watch_cleanup(void);

#endif /* WATCH_H */
