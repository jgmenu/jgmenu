#ifndef ARGS_H
#define ARGS_H

void args_exec_commands(int argc, char **argv);
void args_parse(int argc, char **argv);
char *args_checkout(void);
char *args_csv_file(void);
char *args_csv_cmd(void);
int args_simple(void);
int args_die_when_loaded(void);

#endif /* ARGS_H */
