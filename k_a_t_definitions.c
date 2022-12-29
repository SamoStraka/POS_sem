#include "k_a_t_definitions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>

char *endMsg = ":end";

void data_init(DATA *data, const char* userName, const int socket) {
	data->socket = socket;
	data->stop = 0;
	data->userName[USER_LENGTH] = '\0';
	strncpy(data->userName, userName, USER_LENGTH);
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
    char buffer[BUFFER_LENGTH + 1];
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
	}
	
	return NULL;
}

void *data_writeData(void *data) {    
    DATA *pdata = (DATA *)data;
    char buffer[BUFFER_LENGTH + 1];
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
				write(pdata->socket, buffer, strlen(buffer) + 1);
				
				if (strstr(textStart, endMsg) == textStart && strlen(textStart) == strlen(endMsg)) {
					printf("Koniec komunikacie.\n");
					data_stop(pdata);
				}
			}
        }
    }
	fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) & ~O_NONBLOCK);
	
	return NULL;
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
