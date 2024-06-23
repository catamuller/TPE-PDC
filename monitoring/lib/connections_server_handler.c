#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#include "headers/config.h"
#include "headers/connections_server_handler.h"

#define CRLF "\r\n"
#define CLIENT_NAME "metrics_client"
#define EHLO_MSG "EHLO " CLIENT_NAME CRLF

#define MAIL_MSG "MAIL FROM: test@smtpd.com" CRLF

#define CHECK_MSG "RCPT TO: test" CRLF

#define ON 1
#define OFF 0

#define UNREACHABLE "Unreachable"
#define REACHABLE "Online"

int server_status = OFF;

static int server_socket = -1;


static int create_connection_socket(void){

    int sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock == -1) {
        return -1;
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));

    int port = atoi(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) == -1) {
        close(sock);
        return -1;
    }
    serv_addr.sin_port = htons(port);
    serv_addr.sin_family = AF_INET;

    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        close(sock);
        return -1;
    }

    return sock;

}

int connect_to_server(void) {
    size_t buff_size = 256;
    char buff[buff_size];

    server_socket = create_connection_socket();

    if (server_socket == -1) {
        return 1;
    }

    if (send_request(EHLO_MSG, buff, buff_size) == 1) {
        return 1;
    }

    send_request(MAIL_MSG, buff, buff_size);

    server_status = ON;

    return 0;
}

void close_connection(void) {
    if (server_socket != -1) {
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
    if (send_request(CHECK_MSG, buff, buff_size) == 1) {
        server_status = OFF;
    } else {
        server_status = ON;
    }
}

char* get_server_status(void) {
    return server_status == ON ? REACHABLE : UNREACHABLE;
}
