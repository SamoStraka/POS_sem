#include "k_a_t_client.h"

void data_setServerData(DATA *pData, SERVERDATA data);

int klient_main(int argc, char *argv[]) {
    if (argc < 2) {
        printError("Klienta je nutne spustit s nasledujucimi argumentmi: adresa port.");
    }

    //ziskanie adresy a portu servera <netdb.h>
    struct hostent *server = gethostbyname(argv[0]);
    if (server == NULL) {
        printError("Server neexistuje.");
    }
    int port = atoi(argv[1]);
    if (port <= 0) {
        printError("Port musi byt cele cislo vacsie ako 0.");
    }

    //vytvorenie socketu <sys/socket.h>
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printError("Chyba - socket.");
    }

    //definovanie adresy servera <arpa/inet.h>
    struct sockaddr_in serverAddress;
    bzero((char *) &serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    bcopy((char *) server->h_addr, (char *) &serverAddress.sin_addr.s_addr, server->h_length);
    serverAddress.sin_port = htons(port);

    if (connect(sock, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        printError("Chyba - connect.");
    }

    //inicializacia dat zdielanych medzi vlaknami
    DATA data;
    DATAPONG dataPong = {velkostPolaX / 2, velkostPolaY / 2, 1, 1, velkostPolaY / 2, 0, velkostPolaY / 2, 0};
    data_init(&data, sock, dataPong);

    //inicializacia okna
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    scrollok(stdscr, FALSE);

    //vytvorenie vlakna pre zapisovanie dat do socketu <pthread.h>
    pthread_t thread;
    pthread_create(&thread, NULL, klient_writeData, (void *) &data);

    //v hlavnom vlakne sa bude vykonavat citanie dat zo socketu
    klient_readData((void *) &data);

    //pockame na skoncenie zapisovacieho vlakna <pthread.h>
    pthread_join(thread, NULL);

    //uzavretie socketu <unistd.h>
    close(sock);

    //ukoncenie windowu
    endwin();

    //vypis vysledkov
    vypisKoniec(data.dataPong);

    data_destroy(&data);

    exit_curses(0);

    return (EXIT_SUCCESS);
}

void *klient_readData(void *data) {
    DATA *pdata = (DATA *) data;

    SERVERDATA buffer;
    while (!data_isStopped(pdata)) {
        if (read(pdata->socket, &buffer, sizeof(buffer)) > 0) {
            data_setServerData(pdata, buffer);
            vypisHru(data_getDataPong(pdata));
        } else {
            data_stop(pdata);
        }
    }

    return NULL;
}

void *klient_writeData(void *data) {
    DATA *pdata = (DATA *) data;

    KLIENTDATA buffer;
    while (!data_isStopped(pdata)) {
        int ch;

        if (kbhit()) {
            ch = getch();
        } else {
            ch = getch();
            sleep(1);
        }

        if (ch == end || pdata->dataPong.server.body >= maxSkore || pdata->dataPong.klient.body >= maxSkore) {
            data_stop(pdata);
        }

        if (ch == KEY_UP || ch == KEY_DOWN) {
            pthread_mutex_lock(&pdata->mutex);
            switch (ch) {
                case KEY_UP:
                    if (pdata->dataPong.klient.posY > 0)
                        pdata->dataPong.klient.posY--;
                    break;
                case KEY_DOWN:
                    if (pdata->dataPong.klient.posY < velkostPolaY - 1)
                        pdata->dataPong.klient.posY++;
                    break;
            }
            pthread_mutex_unlock(&pdata->mutex);

            buffer = data_getKlientData(pdata);
            write(pdata->socket, &buffer, sizeof(buffer));
            vypisHru(data_getDataPong(pdata));
        }
    }
    return NULL;
}
