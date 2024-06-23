#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "lib/headers/config.h"
#include "lib/headers/connections_metrics_handler.h"

#define BUFFER_SIZE 1024

#define SEPARATOR "|"

char netstat_response[1024] = {0};
FILE * stream = NULL;

static bool done = false;

static void close_connection(void) {
    if (stream != NULL) {
        pclose(stream);
        stream = NULL;
    }
}

static int init_stream(void) {

    stream = popen("ls -l", "r");

    if (stream == NULL) {
        return 1;
    }

    return 0;
}

static void sigterm_handler(const int signal) {
    printf("Signal %d, cleaning up and exiting\n",signal);
    done = true;
}

static int save_response(char * response) {
    if (fgets(response, 1024, stream) == NULL) {
        return 1;
    }
    return 0;
}

void process_ips(void) {
    size_t out_size = 1024;
    char out[out_size];
    reset_active_connections();

    while (fgets(out, out_size - 1, stream) != NULL) {

        char * section = strtok(out, "\n");

        if (strcmp(section, "END_BATCH") == 0) {
            break;
        } else {
            int net1N, net2N, net3N, hostN, portN;
            sscanf(section, "%d.%d.%d.%d:%d", &net1N, &net2N, &net3N, &hostN, &portN);
            add_to_active_connections(net1N, net2N, net3N, hostN, portN);
            add_to_historic_connections(net1N, net2N, net3N, hostN, portN);
        }
    }
}

void display_metrics(void) {

    printf("\033[2J\033[H");

    printf("Connections\n");
    printf("------------------------------------------------------------------------\n");
    printf("Active: %-30dHistoric: %-30d\n", get_active_connections(), get_historic_connections());
    printf("------------------------------------------------------------------------\n");

    fflush(stdout);

    sleep(1);
}

int create_connection_socket(void){

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

    printf("Connecting to %s:%d\n", SERVER_IP, port);
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) {
        close(sock);
        return -1;
    }

    return sock;

}

int main(int argc, char *argv[]) {

    size_t buff_size = 256;
    char buff[buff_size];

    printf("Creating Socket\n");
    int socket = create_connection_socket();

    if (socket == -1) {
        perror("Failed to connect to server\n");
        goto finish;
    }

    printf("Sending message\n");

    if (send(socket, "EHLO metrics_client\r\n.\r\n", 21, 0) < 0) {
        perror("Send failed");
        goto finish;
    }


    int bytes_received = recv(socket, buff, buff_size - 1, 0);

    if (bytes_received <= 0) {
        perror("Failed to authenticate to server\n");
        goto finish;
    }

    buff[bytes_received] = '\0';

    printf("Server connection successful. Continuing\n");

    snprintf(buff, buff_size,"sudo ./scripts/connections_monitor.sh %s %s", SERVER_IP, SERVER_PORT);

    stream = popen(buff, "r");

    if (stream == NULL) {
        perror("Failed to run script\n");
        goto finish;
    }

    if(init_metrics() == 1) {
        perror("Failed to initialize metrics\n");
        goto finish;
    }

    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);

    while(!done) {
        process_ips();
        display_metrics();
    }

    finish:

    close_metrics();
    if(stream != NULL) {
        pclose(stream);
    }
    close(socket);
    return 0;
}
