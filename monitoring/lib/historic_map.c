#include "headers/config.h"

#define HASH_SIZE 101

typedef struct {
    unsigned int net1;
    unsigned int net2;
    unsigned int net3;
    unsigned int host;
    unsigned int port;
} Entry;

Entry hash_table[HASH_SIZE];
int filled_slots = 0;

unsigned int hash_function(unsigned int key) {
    return key % HASH_SIZE;
}

int insert_entry(unsigned int net1, unsigned int net2, unsigned int net3, unsigned int host, unsigned int port) {
    unsigned int index = hash_function(net1 + net2 + net3 + host + port);

    if (filled_slots >= MAX_GLOBAL_CONNECTIONS) {
        return 1;
    }

    while (hash_table[index].net1 != 0) {
        index = (index + 1) % HASH_SIZE;
    }

    hash_table[index].net1 = net1;
    hash_table[index].net2 = net2;
    hash_table[index].net3 = net3;
    hash_table[index].host = host;
    hash_table[index].port = port;

    filled_slots++;
    return 0;
}

int entry_exists(unsigned net1, unsigned net2, unsigned net3, unsigned host, unsigned port) {
    unsigned int index = hash_function(net1 + net2 + net3 + host + port);
    unsigned int start_index = index;

    while (hash_table[index].net1 != 0) {
        if (hash_table[index].net1 == net1 &&
            hash_table[index].net2 == net2 &&
            hash_table[index].net3 == net3 &&
            hash_table[index].host == host &&
            hash_table[index].port == port) {
            return 1;
        }

        index = (index + 1) % HASH_SIZE;
        if (index == start_index) {
            break;
        }
    }

    return 0;
}
