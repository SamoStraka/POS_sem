#ifndef K_DEFINITIONS_H
#define K_DEFINITIONS_H

#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <curses.h>
#include <netdb.h>
#include <errno.h>

#ifdef    __cplusplus
extern "C" {
#endif

extern int velkostPolaX;
extern int velkostPolaY;
extern int maxSkore;
extern int end;

typedef struct lopticka {
    int posX;
    int posY;
    int movX;
    int movY;
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
    int socket;
    int stop;
    DATAPONG dataPong;
    pthread_mutex_t mutex;
} DATA;

typedef struct klientData {
    int posY;
    int stop;
} KLIENTDATA;

typedef struct serverData {
    DATAPONG dataPong;
    int stop;
} SERVERDATA;

void data_init(DATA *data, const int socket, DATAPONG dataPong);
void data_destroy(DATA *data);
void data_stop(DATA *data);
int data_isStopped(DATA *data);
DATAPONG data_getDataPong(DATA *data);
void data_setKlientData(DATA *data, KLIENTDATA klientData);
KLIENTDATA data_getKlientData(DATA *data);
void data_setServerData(DATA *data, SERVERDATA serverData);
SERVERDATA data_getServerData(DATA *data);

int kbhit(void);

void vypisHru(DATAPONG dataPong);

void vypisKoniec(DATAPONG dataPong);

void printError(char *str);

#ifdef    __cplusplus
}
#endif

#endif

