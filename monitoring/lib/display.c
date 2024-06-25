#include <stdio.h>
#include <stdlib.h>
#include "headers/config.h"
#include "headers/args.h"

void display_metrics(struct metricsargs * args, char* server_status, double ms_delay, int server_current_connections, int server_total_connections, int server_transferred_bytes) {

    system("clear");

    if (args->show_version) {
        printf("////////////////////////////////////////////////////////////////////////\n");
        printf("%s\n", VERSION);
    }
    printf("////////////////////////////////////////////////////////////////////////\n");
    printf("\n");
    printf("SERVER STATUS: %s\n", server_status);
    printf("DELAY: %.3fms\n", ms_delay);
    printf("BYTES TRANSFERRED: %d\n", server_transferred_bytes);
    printf("\n");
    printf("------------------------------------------------------------------------\n");
    printf("\n");
    printf("Verified Connections\n");
    printf("------------------------------------------------------------------------\n");
    printf("Current: %-30dTotal: %-30d\n", server_current_connections, server_total_connections);



    fflush(stdout);
}
