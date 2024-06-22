#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "headers/config.h"
#include "headers/metrics_handler.h"

typedef struct IPV4Connection {
    unsigned net1;
    unsigned net2;
    unsigned net3;
    unsigned host;
    unsigned port;
}Connections;

static size_t active_connections_idx = 0;
static Connections active_connections[MAX_ACTIVE_CONNECTIONS];

void close_metrics(void){
    dump_active_connections();
}

int add_to_active_connections(unsigned net1, unsigned net2, unsigned net3, unsigned host, unsigned port) {

    if(active_connections_idx >= MAX_ACTIVE_CONNECTIONS) {
        return 1;
    }

    active_connections[active_connections_idx].net1 = net1;
    active_connections[active_connections_idx].net2 = net2;
    active_connections[active_connections_idx].net3 = net3;
    active_connections[active_connections_idx].host = host;
    active_connections[active_connections_idx].port = port;

    active_connections_idx++;

    return 0;
}

void reset_active_connections(void) {
    active_connections_idx = 0;
}

void dump_active_connections(void) {
    for (size_t i = 0; i < active_connections_idx; i++) {
        printf("%d.%d.%d.%d:%d\n", active_connections[i].net1,
               active_connections[i].net2, active_connections[i].net3,
               active_connections[i].host, active_connections[i].port);
    }
}

