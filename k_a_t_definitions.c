#include "k_a_t_definitions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <curses.h>

#include <unistd.h>
#include <fcntl.h>

int velkostPolaX = 21;
int velkostPolaY = 7;

int end = KEY_BACKSPACE;

void data_init(DATA *data, const int server, const int socket, const DATAPONG dataPong) {
    data->socket = socket;
    data->stop = 0;
    data->server = server;
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

void *data_readData(void *data) {
    DATA *pdata = (DATA *) data;

    while (!data_isStopped(pdata)) {
        if (read(pdata->socket, &pdata->dataPong, sizeof(pdata->dataPong)) > 0) {
            vypisHru(pdata->dataPong);
        } else {
            data_stop(pdata);
        }
    }

    /*char buffer[BUFFER_LENGTH + 1];
	buffer[BUFFER_LENGTH] = '\0';
    while(!data_isStopped(pdata)) {
		bzero(buffer, BUFFER_LENGTH);
		if (read(pdata->socket, buffer, BUFFER_LENGTH) > 0) {
			char *posSemi = strchr(buffer, ':');
			char *pos = strstr(posSemi + 1, endMsg);
			if (pos != NULL && pos - posSemi == 2 && *(pos + strlen(endMsg)) == '\0') {
				*(pos - 2) = '\0';
				printf("Pouzivatel %s ukoncil komunikaciu.\n", buffer);
				data_stop(pdata);
			}
			else {
				printf("%s\n", buffer);
			}			
		}
		else {
			data_stop(pdata);
		}
	}*/

    return NULL;
}

void *data_writeData(void *data) {
    DATA *pdata = (DATA *) data;
    initscr();

    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    scrollok(stdscr, FALSE);
    while (!data_isStopped(pdata)) {
        int ch;

        if (kbhit()) {
            ch = getch();
        } else {
            ch = getch();
            sleep(1);
        }

        if (ch == KEY_UP || ch == KEY_DOWN) {
            switch (ch) {
                case KEY_UP:
                    if (pdata->server) {
                        if (pdata->dataPong.server.posY > 0)
                            pdata->dataPong.server.posY--;
                    } else {
                        if (pdata->dataPong.klient.posY > 0)
                            pdata->dataPong.klient.posY--;
                    }
                    break;
                case KEY_DOWN:
                    if (pdata->server) {
                        if (pdata->dataPong.server.posY < velkostPolaY - 1)
                            pdata->dataPong.server.posY++;
                    } else {
                        if (pdata->dataPong.klient.posY < velkostPolaY - 1)
                            pdata->dataPong.klient.posY++;
                    }
                    break;
            }
            write(pdata->socket, &pdata->dataPong, sizeof(pdata->dataPong));
            vypisHru(pdata->dataPong);
        } else if (ch == end) {
            data_stop(pdata);
        }
    }

    endwin();

    return NULL;
}

void reset_lopticka(DATAPONG* dataPong){
    dataPong->lopticka.posY = velkostPolaY / 2;
    dataPong->lopticka.posX = velkostPolaX / 2;

    switch (rand() % 4) {
        case 0:
            dataPong->lopticka.movY = 1;
            dataPong->lopticka.movX = 1;
            break;
        case 1:
            dataPong->lopticka.movY = 1;
            dataPong->lopticka.movX = -1;
            break;
        case 2:
            dataPong->lopticka.movY = -1;
            dataPong->lopticka.movX = 1;
            break;
        case 3:
            dataPong->lopticka.movY = -1;
            dataPong->lopticka.movX = -1;
            break;
    }

}

void *pohyb_lopticka(void *data) {
    DATA *pdata = (DATA *) data;

    while (!data_isStopped(pdata)) {
        usleep(900000);

        //pohyb
        pdata->dataPong.lopticka.posY += pdata->dataPong.lopticka.movY;
        pdata->dataPong.lopticka.posX += pdata->dataPong.lopticka.movX;

        //odraz od steny
        if (pdata->dataPong.lopticka.posY >= velkostPolaY - 1 || pdata->dataPong.lopticka.posY <= 0)
            pdata->dataPong.lopticka.movY = -pdata->dataPong.lopticka.movY;

        //naraz do steny
        if (pdata->dataPong.lopticka.posX >= velkostPolaX - 2) {
            if(pdata->dataPong.lopticka.posY == pdata->dataPong.klient.posY){
                pdata->dataPong.lopticka.movX = -pdata->dataPong.lopticka.movX;
            } else {
                pdata->dataPong.server.body++;
                reset_lopticka(&pdata->dataPong);
            }
        }
        if (pdata->dataPong.lopticka.posX <= 1) {
            if(pdata->dataPong.lopticka.posY == pdata->dataPong.server.posY){
                pdata->dataPong.lopticka.movX = -pdata->dataPong.lopticka.movX;
            } else {
                pdata->dataPong.klient.body++;
                reset_lopticka(&pdata->dataPong);
            }
        }

        write(pdata->socket, &pdata->dataPong, sizeof(pdata->dataPong));

        vypisHru(pdata->dataPong);
    }
    return NULL;
}

void vypis(DATAPONG dataPong) {
    printw("lopticka: \tx: %d \ty: %d\n", dataPong.lopticka.posX, dataPong.lopticka.posY);
    printw("server: \ty: %d\n", dataPong.server.posY);
    printw("klient: \ty: %d\n", dataPong.klient.posY);
}

void rectangle(int y1, int x1, int y2, int x2)
{
    mvhline(y1, x1, 0, x2-x1);
    mvhline(y2, x1, 0, x2-x1);
    mvvline(y1, x1, 0, y2-y1);
    mvvline(y1, x2, 0, y2-y1);
    mvaddch(y1, x1, ACS_ULCORNER);
    mvaddch(y2, x1, ACS_LLCORNER);
    mvaddch(y1, x2, ACS_URCORNER);
    mvaddch(y2, x2, ACS_LRCORNER);
}

void vypisSkore(DATAPONG dataPong){
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
