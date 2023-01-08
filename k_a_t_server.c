#include "k_a_t_server.h"

void data_setKlientData(DATA *pData, KLIENTDATA data);

int server_main(int argc, char *argv[]) {
    if (argc < 1) {
        printError("Sever je nutne spustit s nasledujucimi argumentmi: port.");
    }
    int port = atoi(argv[0]);
    if (port <= 0) {
        printError("Port musi byt cele cislo vacsie ako 0.");
    }

    //vytvorenie TCP socketu <sys/socket.h>
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        printError("Chyba - socket.");
    }

    //definovanie adresy servera <arpa/inet.h>
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;         //internetove sockety
    serverAddress.sin_addr.s_addr = INADDR_ANY; //prijimame spojenia z celeho internetu
    serverAddress.sin_port = htons(port);       //nastavenie portu

    //prepojenie adresy servera so socketom <sys/socket.h>
    if (bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        printError("Chyba - bind.");
    }

    //server bude prijimat nove spojenia cez socket serverSocket <sys/socket.h>
    listen(serverSocket, 10);

    //server caka na pripojenie klienta <sys/socket.h>
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    int clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, &clientAddressLength);

    //uzavretie pasivneho socketu <unistd.h>
    close(serverSocket);
    if (clientSocket < 0) {
        printError("Chyba - accept.");
    }

    //inicializacia dat zdielanych medzi vlaknami
    DATA data;
    DATAPONG dataPong = {velkostPolaX / 2, velkostPolaY / 2, 1, 1, velkostPolaY / 2, 0, velkostPolaY / 2, 0};
    data_init(&data, clientSocket, dataPong);

    //inicializacia okna
    initscr();
    cbreak();
    noecho();
    nodelay(stdscr, TRUE);
    keypad(stdscr, TRUE);
    scrollok(stdscr, FALSE);

    //vytvorenie vlakna pre zapisovanie dat do socketu <pthread.h>
    pthread_t thread;
    pthread_create(&thread, NULL, server_writeData, (void *) &data);

    //vytvorenie vlakna pre pohyb lopticky
    pthread_t thread_lopticka;
    pthread_create(&thread_lopticka, NULL, pohyb_lopticka, (void *) &data);

    //v hlavnom vlakne sa bude vykonavat citanie dat zo socketu
    server_readData((void *) &data);

    //pockame na skoncenie vlakna pre pohyb lopticky
    pthread_join(thread_lopticka, NULL);

    //pockame na skoncenie zapisovacieho vlakna <pthread.h>
    pthread_join(thread, NULL);

    //uzavretie socketu klienta <unistd.h>
    close(clientSocket);

    //ukoncenie windowu
    endwin();

    //vypis vysledkov
    vypisKoniec(data.dataPong);

    data_destroy(&data);

    exit_curses(1);

    return (EXIT_SUCCESS);
}

void *server_readData(void *data) {
    DATA *pdata = (DATA *) data;

    KLIENTDATA buffer;
    while (!data_isStopped(pdata)) {
        if (read(pdata->socket, &buffer, sizeof(buffer)) > 0) {
            data_setKlientData(pdata, buffer);
            vypisHru(data_getDataPong(pdata));
        } else {
            data_stop(pdata);
        }
    }

    return NULL;
}

void *server_writeData(void *data) {
    DATA *pdata = (DATA *) data;

    SERVERDATA buffer;
    while (!data_isStopped(pdata)) {
        int ch;

        if (kbhit()) {
            ch = getch();
        } else {
            ch = getch();
            sleep(1);
        }

        if (ch == end) {
            data_stop(pdata);
        }

        if (ch == KEY_UP || ch == KEY_DOWN) {
            pthread_mutex_lock(&pdata->mutex);
            switch (ch) {
                case KEY_UP:
                    if (pdata->dataPong.server.posY > 0)
                        pdata->dataPong.server.posY--;
                    break;
                case KEY_DOWN:
                    if (pdata->dataPong.server.posY < velkostPolaY - 1)
                        pdata->dataPong.server.posY++;
                    break;
            }
            pthread_mutex_unlock(&pdata->mutex);

            buffer = data_getServerData(pdata);
            write(pdata->socket, &buffer, sizeof(buffer));
            vypisHru(data_getDataPong(pdata));
        }
    }
    return NULL;
}

void reset_lopticka(DATAPONG *dataPong) {
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

    SERVERDATA buffer;
    while (!data_isStopped(pdata)) {
        usleep(900000);
        pthread_mutex_lock(&pdata->mutex);
        //pohyb
        pdata->dataPong.lopticka.posY += pdata->dataPong.lopticka.movY;
        pdata->dataPong.lopticka.posX += pdata->dataPong.lopticka.movX;

        //odraz od steny
        if (pdata->dataPong.lopticka.posY >= velkostPolaY - 1 || pdata->dataPong.lopticka.posY <= 0)
            pdata->dataPong.lopticka.movY = -pdata->dataPong.lopticka.movY;

        //naraz do steny
        if (pdata->dataPong.lopticka.posX >= velkostPolaX - 2) {
            if (pdata->dataPong.lopticka.posY == pdata->dataPong.klient.posY) {
                pdata->dataPong.lopticka.movX = -pdata->dataPong.lopticka.movX;
            } else {
                pdata->dataPong.server.body++;
                reset_lopticka(&pdata->dataPong);
            }
        }
        if (pdata->dataPong.lopticka.posX <= 1) {
            if (pdata->dataPong.lopticka.posY == pdata->dataPong.server.posY) {
                pdata->dataPong.lopticka.movX = -pdata->dataPong.lopticka.movX;
            } else {
                pdata->dataPong.klient.body++;
                reset_lopticka(&pdata->dataPong);
            }
        }
        pthread_mutex_unlock(&pdata->mutex);

        buffer = data_getServerData(pdata);
        write(pdata->socket, &buffer, sizeof(buffer));

        vypisHru(data_getDataPong(pdata));
    }
    return NULL;
}