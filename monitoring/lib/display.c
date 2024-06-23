#include <stdio.h>
#include <stdlib.h>
#include "headers/config.h"

void display_metrics(char* server_status, int active_connections, int historic_connections) {

    system("clear");

    printf("------------------------------------------------------------------------\n");
    printf("------------------------------------------------------------------------\n");
    printf("MONITORING CLIENT %s\n", VERSION);
    printf("------------------------------------------------------------------------\n");
    printf("------------------------------------------------------------------------\n");
    printf("\n");
    printf("SERVER STATUS: %s\n", server_status);
    printf("BYTES TRANSFERRED: %d\n", 0);
    printf("\n");
    printf("------------------------------------------------------------------------\n");
    printf("\n");
    printf("Netstat Connections\n");
    printf("------------------------------------------------------------------------\n");
    printf("Active: %-30dHistoric: %-30d\n", active_connections, historic_connections);
    printf("------------------------------------------------------------------------\n");
    printf("\n");
    printf("Verified Connections\n");
    printf("------------------------------------------------------------------------\n");
    printf("Active: %-30dHistoric: %-30d\n", 0, 0);



    fflush(stdout);
}
