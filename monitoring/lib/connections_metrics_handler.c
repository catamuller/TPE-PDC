#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "headers/config.h"
#include "headers/connections_metrics_handler.h"
#include "headers/map.h"

typedef struct IPV4Connection {
    unsigned net1;
    unsigned net2;
    unsigned net3;
    unsigned host;
    unsigned port;
}Connections;

static size_t active_connections_idx = 0;
static Connections active_connections[MAX_ACTIVE_CONNECTIONS];

static size_t historic_connections_idx = 0;
static Connections historic_connections[MAX_GLOBAL_CONNECTIONS];

Map * historic_connections_map;

int init_metrics(void) {
    historic_connections_map = mapInit(MAX_GLOBAL_CONNECTIONS);
    if (historic_connections_map == NULL) {
        return 1;
    }
    return 0;
}

void close_metrics(void){
    dump_active_connections();
    dump_historic_connections();
    if (historic_connections_map != NULL) {
        mapFree(historic_connections_map);
    }
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

int add_to_historic_connections(unsigned net1, unsigned net2, unsigned net3, unsigned host, unsigned port) {
    if(historic_connections_idx >= MAX_GLOBAL_CONNECTIONS) {
        return 1;
    }
    char ip[16];

    snprintf(ip, 16,"%d.%d.%d.%d:%d", net1, net2, net3, host, port);
    if (mapGet(historic_connections_map, ip).type != 0) {
        return 0;
    }
    MapValue aux = {1, ""};
    if (mapPut(historic_connections_map, ip, aux) == false) {
        return 1;
    }
    historic_connections[historic_connections_idx].net1 = net1;
    historic_connections[historic_connections_idx].net2 = net2;
    historic_connections[historic_connections_idx].net3 = net3;
    historic_connections[historic_connections_idx].host = host;
    historic_connections[historic_connections_idx].port = port;

    historic_connections_idx++;
    return 0;
}

void reset_active_connections(void) {
    active_connections_idx = 0;
}

void dump_active_connections(void) {
    printf("Active Connections:\n");
    for (size_t i = 0; i < active_connections_idx; i++) {
        printf("%d.%d.%d.%d:%d\n", active_connections[i].net1,
               active_connections[i].net2, active_connections[i].net3,
               active_connections[i].host, active_connections[i].port);
    }
}

void dump_historic_connections(void) {
    printf("Historic Connections:\n");
    for (size_t i = 0; i < historic_connections_idx; i++) {
        printf("%d.%d.%d.%d:%d\n", historic_connections[i].net1,
               historic_connections[i].net2, historic_connections[i].net3,
               historic_connections[i].host, historic_connections[i].port);
    }
}

int get_active_connections(void) {
    return active_connections_idx;
}

int get_historic_connections(void) {
    return historic_connections_idx;
}
