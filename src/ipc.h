#ifndef IPC_H
#define IPC_H

#include <stdio.h>
#include <stdlib.h>

void ipc_read_socket(void);
void ipc_init_socket(void);
void ipc_align_based_on_env_vars(void);

#endif /* IPC_H */
