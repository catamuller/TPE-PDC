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
    printf("Active Connections: %-30dHistoric Connections: %-30d\n", get_active_connections(), get_historic_connections());
    printf("------------------------------------------------------------------------\n");

    fflush(stdout);

    sleep(1);
}

int main(int argc, char *argv[]) {

    size_t cmd_size = 256;
    char command[cmd_size];

    FILE * server_check;

    snprintf(command, cmd_size, "sudo ./scripts/server_check.sh %s %s", SERVER_IP, SERVER_PORT);
    printf("%s\n", command);

    server_check = popen(command, "r");

    if (server_check == NULL) {
        perror("Failed to run script\n");
        goto finish;
    }

    char buff[16] = {'\0'};

    fgets(buff, 15, server_check);

    size_t len = strlen(buff);
    if (len > 0 && buff[len - 1] == '\n') {
        buff[len - 1] = '\0';
    }

    if (strcmp(buff, "true") != 0) {
        perror("SMTP Server can't be reached\n");
        goto finish;
    }

    pclose(server_check);

    snprintf(command, cmd_size,"sudo ./scripts/monitor.sh %s %s", SERVER_IP, SERVER_PORT);

    stream = popen(command, "r");

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
    return 0;
}
