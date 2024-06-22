#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>

#include "lib/headers/config.h"
#include "lib/headers/metrics_handler.h"

#define BUFFER_SIZE 1024

#define SEPARATOR "|"

char netstat_response[1024] = {0};
FILE * stream = NULL;

static bool done = false;

static void check_connections(void) {
    char command[256];
    snprintf(command, sizeof(command),
             "netstat -anp | grep :%s | grep ESTABLISHED | awk '{ if ($5 != \"%s:%s\") print $5 }'",
             SERVER_PORT, SERVER_IP, SERVER_PORT);
    stream = popen(command, "r");
}

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

void process_output(const char *output) {
    char *token;
    char separator[] = "|";
    char *saveptr;

    token = strtok_r(output, separator, &saveptr);
    while (token != NULL) {
        if (strcmp(token, "END_BATCH") == 0) {
            // Process end of batch
            printf("End of batch\n");
        } else {
            // Process connection info
            printf("Connection: %s\n", token);
        }
        token = strtok_r(NULL, separator, &saveptr);
    }
}

int main(int argc, char *argv[]) {
    FILE *fp;
    char out[1024];

    fp = popen("sudo ./scripts/monitor.sh 127.0.0.1 2525", "r");
    if (fp == NULL) {
        printf("Failed to run script\n");
        exit(1);
    }

    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);

    while(!done) {
        while (fgets(out, 1023, fp) != NULL) {

            char * section = strtok(out, "\n");

            if (strcmp(section, "END_BATCH") == 0) {
                dump_active_connections();
                reset_active_connections();
            } else {
                int net1N, net2N, net3N, hostN, portN;
                sscanf(section, "%d.%d.%d.%d:%d", &net1N, &net2N, &net3N, &hostN, &portN);
                add_to_active_connections(net1N, net2N, net3N, hostN, portN);
            }
        }
    }

    close_metrics();
    pclose(fp);
    return 0;
}
