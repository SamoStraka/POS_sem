#include "k_a_t_definitions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <curses.h>

#include <unistd.h>
#include <fcntl.h>

int velkostPolaX = 20;
int velkostPolaY = 10;

char end = KEY_BACKSPACE;

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
    DATA *pdata = (DATA *)data;

    while(!data_isStopped(pdata)) {
        if (read(pdata->socket, &pdata->dataPong, sizeof(pdata->dataPong)) > 0) {
            printf("Nacitanie!\n");
            vypis(pdata->dataPong);
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
    DATA *pdata = (DATA *)data;
    initscr();

    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    scrollok(stdscr, TRUE);
    while(!data_isStopped(pdata)) {
        int ch;

        if (kbhit()) {
            ch = getch();
            refresh();
        } else {
            ch = getch();
            refresh();
            sleep(1);
        }

    if(ch == KEY_UP || ch == KEY_DOWN){
            switch (ch) {
                case KEY_UP:
                    if(pdata->server){
                        if(pdata->dataPong.server.posY < velkostPolaY - 1)
                            pdata->dataPong.server.posY++;
                    } else {
                        if(pdata->dataPong.klient.posY < velkostPolaY - 1)
                            pdata->dataPong.klient.posY++;
                    }
                    break;
                case KEY_DOWN:
                    if(pdata->server){
                        if(pdata->dataPong.server.posY > 0)
                            pdata->dataPong.server.posY--;
                    } else {
                        if(pdata->dataPong.klient.posY > 0)
                            pdata->dataPong.klient.posY--;
                    }
                    break;
            }
            write(pdata->socket, &pdata->dataPong, sizeof(pdata->dataPong));
        }
        else if(ch == end){
            printf("Koniec hry.\n");
            data_stop(pdata);
        }
    }

    endwin();

    /*char buffer[BUFFER_LENGTH + 1];
	buffer[BUFFER_LENGTH] = '\0';
	int userNameLength = strlen(pdata->userName);

	//pre pripad, ze chceme poslat viac dat, ako je kapacita buffra
	fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK);
	fd_set inputs;
    FD_ZERO(&inputs);
	struct timeval tv;
	tv.tv_usec = 0;
    while(!data_isStopped(pdata)) {
		tv.tv_sec = 1;
		FD_SET(STDIN_FILENO, &inputs);
		select(STDIN_FILENO + 1, &inputs, NULL, NULL, &tv);
		if (FD_ISSET(STDIN_FILENO, &inputs)) {
			sprintf(buffer, "%s: ", pdata->userName);
			char *textStart = buffer + (userNameLength + 2);
			while (fgets(textStart, BUFFER_LENGTH - (userNameLength + 2), stdin) > 0) {
				char *pos = strchr(textStart, '\n');
				if (pos != NULL) {
					*pos = '\0';
				}

                int ch = getchar();
                printf("Received Input: %c\n", ch);

				write(pdata->socket, buffer, strlen(buffer) + 1);
				
				if (strstr(textStart, endMsg) == textStart && strlen(textStart) == strlen(endMsg)) {
					printf("Koniec komunikacie.\n");
					data_stop(pdata);
				}
			}
        }
    }
	fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) & ~O_NONBLOCK);*/
	
	return NULL;
}

void *pohyb_lopticka(void *data) {
    DATA *pdata = (DATA *)data;

    while(!data_isStopped(pdata)) {
        usleep(900000);

        //pohyb
        pdata->dataPong.lopticka.posY += pdata->dataPong.lopticka.movY;
        pdata->dataPong.lopticka.posX += pdata->dataPong.lopticka.movX;

        //odraz od steny
        if(pdata->dataPong.lopticka.posY > velkostPolaY - 1 || pdata->dataPong.lopticka.posY < 0)
            pdata->dataPong.lopticka.posY -= pdata->dataPong.lopticka.posY;

        //naraz do steny
        if(pdata->dataPong.lopticka.posX >= velkostPolaX - 1){
            pdata->dataPong.server.body++;
            //reset lopticky
            pdata->dataPong.lopticka.posY = velkostPolaY / 2;
            pdata->dataPong.lopticka.posX = velkostPolaX / 2;
        }
        if(pdata->dataPong.lopticka.posX <= 0) {
            pdata->dataPong.klient.body++;
            //reset lopticky
            pdata->dataPong.lopticka.posY = velkostPolaY / 2;
            pdata->dataPong.lopticka.posX = velkostPolaX / 2;
        }

        write(pdata->socket, &pdata->dataPong, sizeof(pdata->dataPong));
    }
    return NULL;
}

void vypis(DATAPONG dataPong) {
    move(10, 30);
    addch(c)
    printf("lopticka: \tx: %d \ty: %d\n", dataPong.lopticka.posX, dataPong.lopticka.posY);
    printf("server: \ty: %d\n", dataPong.server.posY);
    printf("klient: \ty: %d\n", dataPong.klient.posY);
}

int kbhit(void)
{
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
	}
	else {
		fprintf(stderr, "%s\n", str);
	}
    exit(EXIT_FAILURE);
}
