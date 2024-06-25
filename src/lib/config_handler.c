#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <strings.h>
#include "headers/selector.h"
#include "headers/logger.h"

#define ERROR (-1)
#define BUFF_SIZE 256

enum vars {
    LOGGING=0,
    UNKNOWN=-1
};

void receive_data(struct selector_key *key);

static const struct fd_handler config_handler = {
        .handle_read   = receive_data,
        .handle_write  = NULL,
        .handle_close  = NULL,
        .handle_block  = NULL,
};

static char * variables[] = {"LOGGING", NULL};

static int socket_listen = -1;
static int socket_addr = -1;
static struct sockaddr_in serv_addr;
static char read_buff[BUFF_SIZE];

int init_config_socket(char *ip, int port, const char **err_msg) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        *err_msg = "Unable to create socket for config.";
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
        close(sock);
        *err_msg = "Invalid IP address.";
        return -1;
    }

    if (bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        close(sock);
        *err_msg = "Bind failed.";
        return -1;
    }

    if (listen(sock, SOMAXCONN) == -1) {
        close(sock);
        *err_msg = "Listen failed.";
        return -1;
    }

    socket_listen = sock;
    return sock;
}

static int parse_variable(char *var) {
    if(var == NULL) {
        return UNKNOWN;
    }
    for(int i=0; variables[i]!=NULL; i++) {
        if(strcasecmp(var, variables[i]) == 0) {
            return i;
        }
    }
    return UNKNOWN;
}

int parse_input(const char *input, int *var, int *val) {
    char buff[BUFF_SIZE];
    strncpy(buff, input, BUFF_SIZE - 1);
    char *token = strtok(buff, "=");

    if (token == NULL || strlen(token) == strlen(input)) {
        return ERROR;
    }

    *var = parse_variable(token);

    token = strtok(NULL, "=");

    if (token == NULL) {
        return ERROR;
    }

    char *endptr;
    *val = strtol(token, &endptr, 10);

    return 0;
}

static void set_variable(enum vars variable, int value) {
    switch(variable) {
        case LOGGING:
            set_logging(value);
            break;
        default:
            break;
    }
}

void receive_data(struct selector_key *key) {
    if(socket_addr == -1) {
        return;
    }
    ssize_t bytes = recv(socket_addr, read_buff, BUFF_SIZE, 0);
    if(bytes == 0) {
        if(socket_addr != -1) {
            selector_unregister_fd(key->s, socket_addr);
            socket_addr = -1;
        }
    }

    int variable, value;
    if(parse_input(read_buff, &variable, &value) == -1) {
        return;
    }

    set_variable(variable, value);
}

void accept_connection(struct selector_key *key) {
    if (socket_addr == -1) {
        socklen_t addrlen = sizeof(struct sockaddr_in);
        socket_addr = accept(socket_listen, (struct sockaddr *)&serv_addr, &addrlen);
        if (socket_addr < 0) {
            return;
        }

        int flags = fcntl(socket_addr, F_GETFL, 0);
        if (flags == -1) {
            close(socket_addr);
            socket_addr = -1;
            return;
        }
        if (fcntl(socket_addr, F_SETFL, flags | O_NONBLOCK) == -1) {
            close(socket_addr);
            socket_addr = -1;
            return;
        }

        if(SELECTOR_SUCCESS != selector_register(key->s, socket_addr, &config_handler, OP_READ, NULL)) {
            goto fail;
        }
    }
    return;

    fail:
    if(socket_addr != -1) {
        close(socket_addr);
    }
}
