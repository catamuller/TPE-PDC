#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>

#include "lib/headers/config.h"
#include "lib/headers/connections_metrics_handler.h"
#include "lib/headers/display.h"
#include "lib/headers/connections_server_handler.h"

#define BUFFER_SIZE 1024

char netstat_response[1024] = {0};
FILE * stream = NULL;

static bool done = false;

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

int main(int argc, char *argv[]) {

    size_t buff_size = 256;
    char buff[buff_size];

    if (connect_to_server() == 1) {
        perror("Failed to connect to server\n");
        goto finish;
    }

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
        check_server_status();
        display_metrics(get_server_status(), get_active_connections(), get_historic_connections());
        sleep(1);
    }

    finish:

    close_metrics();
    close_connection();

    if (stream != NULL)
        pclose(stream);


    return 0;
}
