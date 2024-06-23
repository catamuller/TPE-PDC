#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "headers/config.h"
#include "headers/connections_metrics_handler.h"
#include "headers/historic_map.h"

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

FILE * ip_dump = NULL;

bool can_write = false;

int init_metrics(void) {
    ip_dump = fopen("ip_logs.txt", "w");
    if (ip_dump == NULL) {
        return 1;
    }
    can_write = true;
    return 0;
}

void close_metrics(void){
    dump_active_connections();
    dump_historic_connections();
    if (ip_dump != NULL) {
        fclose(ip_dump);
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

    if (entry_exists(net1, net2, net3, host, port)) {
        return 0;
    }

    if (insert_entry(net1, net2, net3, host, port) == 1) {
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
    if (!can_write) {
        return;
    }
    fprintf(ip_dump, "Active Connections:\n");
    for (size_t i = 0; i < active_connections_idx; i++) {
        fprintf(ip_dump, "%d.%d.%d.%d:%d\n", active_connections[i].net1,
               active_connections[i].net2, active_connections[i].net3,
               active_connections[i].host, active_connections[i].port);
    }
}

void dump_historic_connections(void) {
    if (!can_write) {
        return;
    }
    fprintf(ip_dump, "Historic Connections:\n");
    for (size_t i = 0; i < historic_connections_idx; i++) {
        fprintf(ip_dump, "%d.%d.%d.%d:%d\n", historic_connections[i].net1,
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
