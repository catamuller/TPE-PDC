#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include "headers/client_server_connections.h"

int server_socket;

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
    server_socket = create_connection_socket(ip, port);
    if (server_socket == -1) {
        return 1;
    }

    return server_socket;
}

int send_request(char* msg, char* buff, size_t buff_size) {
    if (server_socket == -1) {
        return 1;
    }

    size_t msg_len = strlen(msg);
    if (send(server_socket, msg, msg_len, 0) < 0) {
        return 1;
    }

    int bytes_received = recv(server_socket, buff, buff_size - 1, 0);
    if (bytes_received <= 0) {

        return 1;
    }

    buff[bytes_received] = '\0';

    return 0;
}

void close_connection(void) {
    if (server_socket != -1) {
        close(server_socket);
    }
}
