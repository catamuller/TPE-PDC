#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <stdio.h>

#include "headers/config.h"
#include "headers/connections_server_handler.h"

#define CRLF "\r\n"
#define CLIENT_NAME "metrics_client"
#define EHLO_MSG "EHLO " CLIENT_NAME CRLF

#define CHECK_MSG "NOOP" CRLF

#define TOTAL_MSG "XSTAT total" CRLF
#define CURRENT_MSG "XSTAT current" CRLF
#define BYTES_MSG "XSTAT bytes" CRLF

#define QUIT_MSG "QUIT" CRLF

#define ON 1
#define OFF 0

#define UNREACHABLE "Unreachable"
#define REACHABLE "Online"

int server_status = OFF;

static int server_socket = -1;

double ms_delay = 0.0;

int current_connections = 0;
int total_connections = 0;
int transferred_bytes = 0;


static int create_connection_socket(char * ip, char * port){

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1) {
        return -1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));

    int port_i = atoi(port);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) == -1) {
        close(sock);
        return -1;
    }
    serv_addr.sin_port = htons(port_i);
    serv_addr.sin_family = AF_INET;

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        close(sock);
        return -1;
    }

    return sock;
}

int connect_to_server(char * ip, char * port) {
    size_t buff_size = 256;
    char buff[buff_size];

    server_socket = create_connection_socket(ip, port);

    if (server_socket == -1) {
        perror("Failed to create socket.\n");
        return 1;
    }

    if (send_request(EHLO_MSG, buff, buff_size) == 1) {
        perror("Failed to authenticate.\n");
        return 1;
    }

    if (send_request(TOTAL_MSG, buff, buff_size) == 1) {
        perror("XSTAT command failed.\n");
        return 1;
    }

    int code = atoi(buff);

    if (code == 500 || code == 502) {
        perror("The server is not compatible.\n");
        return 1;
    }

    server_status = ON;

    return 0;
}

void close_connection(void) {
    if (server_socket != -1) {
        size_t buff_size = 256;
        char buff[buff_size];
        send_request(QUIT_MSG, buff, buff_size);
        close(server_socket);
    }
}

int send_request(char* msg, char* buff, size_t buff_size) {
    if (server_socket == -1) {
        server_status = OFF;
        return 1;
    }

    size_t msg_len = strlen(msg);
    if (send(server_socket, msg, msg_len, 0) < 0) {
        server_status = OFF;
        return 1;
    }

    int bytes_received = recv(server_socket, buff, buff_size - 1, 0);
    if (bytes_received <= 0) {
        server_status = OFF;
        return 1;
    }

    buff[bytes_received] = '\0';

    return 0;
}

void check_server_status(void) {
    size_t buff_size = 256;
    char buff[buff_size];

    clock_t start_time = clock();

    if (send_request(CHECK_MSG, buff, buff_size) == 1) {
        server_status = OFF;
    } else {
        server_status = ON;
    }

    clock_t end_time = clock();

    ms_delay = ((double)(end_time - start_time) * 1000) / CLOCKS_PER_SEC;
}

char* get_server_status(void) {
    return server_status == ON ? REACHABLE : UNREACHABLE;
}

double get_ms_delay(void) {
    return ms_delay;
}

void retrieve_server_stats(void) {
    size_t buff_size = 256;
    char buff[buff_size];
    char *token;

    if (send_request(CURRENT_MSG, buff, buff_size) == 0) {
        token = strtok(buff, ":");
        token = strtok(NULL, ":");
        current_connections = atoi(token);
    }
    if (send_request(TOTAL_MSG, buff, buff_size) == 0) {
        token = strtok(buff, ":");
        token = strtok(NULL, ":");
        total_connections = atoi(token);
    }
    if (send_request(BYTES_MSG, buff, buff_size) == 0) {
        token = strtok(buff, ":");
        token = strtok(NULL, ":");
        transferred_bytes = atoi(token);
    }
}

int get_server_current_connections(void) {
    return current_connections;
}

int get_server_total_connections(void) {
    return total_connections;
}

int get_server_transferred_bytes(void) {
    return transferred_bytes;
}
