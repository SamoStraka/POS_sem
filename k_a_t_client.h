#ifndef POS_SEM_K_A_T_CLIENT_H
#define POS_SEM_K_A_T_CLIENT_H

#include "k_a_t_definitions.h"

int klient_main(int argc, char *argv[]);
void *klient_readData(void *data);
void *klient_writeData(void *data);

#endif
