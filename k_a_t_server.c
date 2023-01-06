#include "k_a_t_definitions.h"

#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

int server_main(int argc, char* argv[]) {
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
    if (bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        printError("Chyba - bind.");
    }
    
    //server bude prijimat nove spojenia cez socket serverSocket <sys/socket.h>
    listen(serverSocket, 10);
    
    //server caka na pripojenie klienta <sys/socket.h>
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLength);

    //uzavretie pasivneho socketu <unistd.h>
    close(serverSocket);
    if (clientSocket < 0) {
        printError("Chyba - accept.");        
    }
 
	//inicializacia dat zdielanych medzi vlaknami
    DATA data;
    DATAPONG dataPong = {velkostPolaX / 2, velkostPolaY / 2, 1, 1, velkostPolaY / 2, 0, velkostPolaY / 2, 0};
	data_init(&data, 1, clientSocket, dataPong);
	
	//vytvorenie vlakna pre zapisovanie dat do socketu <pthread.h>
    pthread_t thread;
    pthread_create(&thread, NULL, data_writeData, (void *)&data);

    //vytvorenie vlakna pre pohyb lopticky
    pthread_t thread_lopticka;
    pthread_create(&thread_lopticka, NULL, pohyb_lopticka, (void *)&data);

	//v hlavnom vlakne sa bude vykonavat citanie dat zo socketu
	data_readData((void *)&data);

    //pockame na skoncenie vlakna pre pohyb lopticky
    pthread_join(thread_lopticka, NULL);

	//pockame na skoncenie zapisovacieho vlakna <pthread.h>
	pthread_join(thread, NULL);
	data_destroy(&data);
	
    //uzavretie socketu klienta <unistd.h>
    close(clientSocket);
    
    return (EXIT_SUCCESS);
}