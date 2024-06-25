#include <string.h>
#include <stdio.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "headers/selector.h"
#include "headers/logger.h"

#define ERROR -1

enum vars {
    LOGGING=0,

    UNKNOWN=-1
};

static char * variables[] = {"LOGGING"};
static int v_size = sizeof(variables);

int init_config_socket(char *ip, int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        return -1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        close(sock);
        return -1;
    }

    if (bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        close(sock);
        return -1;
    }

    if (listen(sock, SOMAXCONN) == -1) {
        close(sock);
        return -1;
    }

    return sock;
}

static int parse_variable(char *var) {
    if(var == NULL) {
        return UNKNOWN;
    }
    for(int i=0; i<v_size; i++) {
        if(strcasecmp(var, variables[i]) == 0) {
            return i;
        }
    }
    return UNKNOWN;
}

static int parse_value(char *val, int *ret) {
    if(val == NULL) {
        return UNKNOWN;
    }
    for(int i=0; val[i]!='\0'; i++) {
        if(val[i] < '0' && val[i] > '9') {
            return ERROR;
        }
    }
    *ret = atoi(val);
    return 0;
}

static void set_variable(enum vars variable, int value) {
    switch(variable) {
        case LOGGING:
            set_logging(value);
            break;
        case UNKNOWN:
            break;
        default:
            break;
    }
}

void config_read(struct selector_key *key) {
    printf("%s\n", key->data);
    if(key->data != NULL) {
        char* token;
        token = strtok(key->data, "=");
        if(token == NULL) {
            return;
        }
        enum vars variable = parse_variable(token);
        if(variable == UNKNOWN) {
            return;
        }
        token = strtok(NULL, "=");
        if(token == NULL) {
            return;
        }
        int value = 0;
        if(parse_value(token,&value) == ERROR) {
            return;
        }

        set_variable(variable, value);
    }
}

