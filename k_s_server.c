#include "k_s_server.h"
#include "k_s_definitions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

static char* spracujData(char *data) {
    char *akt = data;
    while (*akt != '\0') {
        if (islower(*akt)) {
            *akt = toupper(*akt);
        }
        else if (isupper(*akt)) {
            *akt = tolower(*akt);
        } 
		akt++;		
    }
    return data;
}

int server_main(int argc, char** argv) {
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
    int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLength);

    //uzavretie pasivneho socketu <unistd.h>
    close(serverSocket);
    if (clientSocket < 0) {
        printError("Chyba - accept.");        
    }
	
    printf("Klient sa pripojil na server.\n");
    char buffer[BUFFER_LENGTH + 1];
    buffer[BUFFER_LENGTH] = '\0';
    int koniec = 0;
    while (!koniec) {
        //citanie dat zo socketu <unistd.h>
		read(clientSocket, buffer, BUFFER_LENGTH);
        if (strcmp(buffer, endMsg) != 0) {
            printf("Klient poslal nasledujuce data:\n%s\n", buffer);
            spracujData(buffer);
			//zapis dat do socketu <unistd.h>
			write(clientSocket, buffer, strlen(buffer) + 1);
        }
        else {
            koniec = 1;
        }
    }
    printf("Klient ukoncil komunikaciu.\n");
    
    //uzavretie socketu klienta <unistd.h>
    close(clientSocket);
    
    return (EXIT_SUCCESS);
}