#include <stdio.h>
#include <string.h>
#include "k_s_server.h"
#include "k_s_client.h"

int main(int argc, char** argv) {
    if(argc < 2){
        printf("At least one argument is needed.");
        return 1;
    }
    if(strcmp(argv[1], "server") == 0){
        server_main(argc - 2, argv + 2);
    } else if(strcmp(argv[1], "klient") == 0){
        klient_main(argc - 2, argv + 2);
    } else {
        printf("Unspecified option.");
        return 1;
    }
    return 0;
}
