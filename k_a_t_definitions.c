#include "k_a_t_definitions.h"

int velkostPolaX = 21;
int velkostPolaY = 7;
int maxSkore = 10;
int end = 27; //ESC

void data_init(DATA *data, const int socket, DATAPONG dataPong) {
    data->socket = socket;
    data->stop = 0;
    data->dataPong = dataPong;
    pthread_mutex_init(&data->mutex, NULL);
}

void data_destroy(DATA *data) {
    pthread_mutex_destroy(&data->mutex);
}

void data_stop(DATA *data) {
    pthread_mutex_lock(&data->mutex);
    data->stop = 1;
    pthread_mutex_unlock(&data->mutex);
}

int data_isStopped(DATA *data) {
    int stop;
    pthread_mutex_lock(&data->mutex);
    stop = data->stop;
    pthread_mutex_unlock(&data->mutex);
    return stop;
}

DATAPONG data_getDataPong(DATA *data) {
    DATAPONG datapong;
    pthread_mutex_lock(&data->mutex);
    datapong = data->dataPong;
    pthread_mutex_unlock(&data->mutex);
    return datapong;
}

void data_setKlientData(DATA *data, KLIENTDATA klientData) {
    pthread_mutex_lock(&data->mutex);
    data->dataPong.klient.posY = klientData.posY;
    if (!data->stop)
        data->stop = klientData.stop;
    pthread_mutex_unlock(&data->mutex);
}

KLIENTDATA data_getKlientData(DATA *data) {
    KLIENTDATA klientData;
    pthread_mutex_lock(&data->mutex);
    klientData.posY = data->dataPong.klient.posY;
    klientData.stop = data->stop;
    pthread_mutex_unlock(&data->mutex);
    return klientData;
}

void data_setServerData(DATA *data, SERVERDATA serverData) {
    pthread_mutex_lock(&data->mutex);
    data->dataPong = serverData.dataPong;
    if (!data->stop)
        data->stop = serverData.stop;
    pthread_mutex_unlock(&data->mutex);
}

SERVERDATA data_getServerData(DATA *data) {
    SERVERDATA serverData;
    pthread_mutex_lock(&data->mutex);
    serverData.dataPong = data->dataPong;
    serverData.stop = data->stop;
    pthread_mutex_unlock(&data->mutex);
    return serverData;
}

void rectangle(int y1, int x1, int y2, int x2) {
    mvhline(y1, x1, 0, x2 - x1);
    mvhline(y2, x1, 0, x2 - x1);
    mvvline(y1, x1, 0, y2 - y1);
    mvvline(y1, x2, 0, y2 - y1);
    mvaddch(y1, x1, ACS_ULCORNER);
    mvaddch(y2, x1, ACS_LLCORNER);
    mvaddch(y1, x2, ACS_URCORNER);
    mvaddch(y2, x2, ACS_LRCORNER);
}

void vypisSkore(DATAPONG dataPong) {
    char str[24];
    sprintf(str, "Server  %d : %d  Klient", dataPong.server.body, dataPong.klient.body);

    mvaddstr(velkostPolaY + 2, (velkostPolaX / 2) - (strlen(str) / 2), str);
}

void vypisHru(DATAPONG dataPong) {
    clear();

    rectangle(0, 0, velkostPolaY + 1, velkostPolaX + 1);

    mvaddch(dataPong.server.posY + 1, 1, '|');
    mvaddch(dataPong.klient.posY + 1, velkostPolaX, '|');
    mvaddch(dataPong.lopticka.posY + 1, dataPong.lopticka.posX + 1, '*');

    vypisSkore(dataPong);

    refresh();
}

void vypisKoniec(DATAPONG dataPong) {
    printf("Komunikácia bola ukončená\n\n");
    if (dataPong.klient.body > dataPong.server.body) {
        printf("\tSERVER\t%d : %d\tKLIENT\n", dataPong.server.body, dataPong.klient.body);
        printf("\tVíťazom sa stal klient\n\n");
    } else if (dataPong.klient.body < dataPong.server.body) {
        printf("\tSERVER\t%d : %d\tKLIENT\n", dataPong.server.body, dataPong.klient.body);
        printf("\tVíťazom sa stal server\n\n");
    } else {
        printf("\tSERVER\t%d : %d\tKLIENT\n", dataPong.server.body, dataPong.klient.body);
        printf("\tNastala remíza\n\n", dataPong.klient.body);
    }
}

int kbhit(void) {
    int ch = getch();

    if (ch != ERR) {
        ungetch(ch);
        return 1;
    } else {
        return 0;
    }
}

void printError(char *str) {
    if (errno != 0) {
        perror(str);
    } else {
        fprintf(stderr, "%s\n", str);
    }
    exit(EXIT_FAILURE);
}
