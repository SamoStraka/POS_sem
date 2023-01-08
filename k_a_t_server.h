#ifndef POS_SEM_K_A_T_SERVER_H
#define POS_SEM_K_A_T_SERVER_H

#include "k_a_t_definitions.h"

int server_main(int argc, char **argv);
void *server_readData(void *data);
void *server_writeData(void *data);
void reset_lopticka(DATAPONG *dataPong);
void *pohyb_lopticka(void *data);

#endif
