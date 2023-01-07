#include "k_a_t_definitions.h"

#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <curses.h>

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
    bzero((char *)&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serverAddress.sin_addr.s_addr, server->h_length);
    serverAddress.sin_port = htons(port);

    if (connect(sock,(struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        printError("Chyba - connect.");        
    }
    
	//inicializacia dat zdielanych medzi vlaknami
    DATA data;
    DATAPONG dataPong = {velkostPolaX / 2, velkostPolaY / 2, 1, 1, velkostPolaY / 2, 0, velkostPolaY / 2, 0};
	data_init(&data, 0, sock, dataPong);
	
	//vytvorenie vlakna pre zapisovanie dat do socketu <pthread.h>
    pthread_t thread;
    pthread_create(&thread, NULL, data_writeData, (void *)&data);

	//v hlavnom vlakne sa bude vykonavat citanie dat zo socketu
	data_readData((void *)&data);

	//pockame na skoncenie zapisovacieho vlakna <pthread.h>
	pthread_join(thread, NULL);


    //uzavretie socketu <unistd.h>
    close(sock);

    //ukoncenie windowu
    endwin();

    //vypis vysledkov
    vypisKoniec(&data);

    data_destroy(&data);
    exit_curses(1);

    return (EXIT_SUCCESS);
}
