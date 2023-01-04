#ifndef K_DEFINITIONS_H
#define	K_DEFINITIONS_H

#include <pthread.h>

#ifdef	__cplusplus
extern "C" {
#endif

typedef struct lopticka {
    int posX;
    int posY;
} LOPTICKA;

typedef struct hrac {
    int posY;
    int body;
} HRAC;

typedef struct dataPong {
    LOPTICKA lopticka;
    HRAC server;
    HRAC klient;
} DATAPONG;

typedef struct data {
    int server;
    int socket;
    int stop;
    DATAPONG dataPong;
    pthread_mutex_t mutex;
} DATA;

void data_init(DATA *data, const int server, const int socket, const DATAPONG dataPong);
void data_destroy(DATA *data);
void data_stop(DATA *data);
int data_isStopped(DATA *data);
void *data_readData(void *data);
void *data_writeData(void *data);
void vypis(DATAPONG dataPong);

void printError(char *str);

#ifdef	__cplusplus
}
#endif

#endif	/* K_DEFINITIONS_H */

