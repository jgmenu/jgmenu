#ifndef ARGS_H
#define ARGS_H

extern void args_exec_commands(int argc, char **argv);
extern void args_parse(int argc, char **argv);
extern char *args_checkout(void);
extern char *args_csv_file(void);
extern char *args_csv_cmd(void);
extern int args_simple(void);
extern int args_die_when_loaded(void);

#endif /* ARGS_H */
