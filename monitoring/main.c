#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>

#include "lib/headers/config.h"
#include "lib/headers/display.h"
#include "lib/headers/connections_server_handler.h"
#include "lib/headers/args.h"

#define BUFFER_SIZE 1024

char netstat_response[1024] = {0};
FILE * stream = NULL;

static bool done = false;

struct metricsargs args_struct;

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


int main(int argc, char *argv[]) {

    struct metricsargs * args = &args_struct;

    parse_args(argc, argv, args);

    if (connect_to_server(args->server_addr, args->server_port) == 1) {
        perror("Failed to connect to server\n");
        goto finish;
    }


    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);

    while(!done) {
        check_server_status();
        retrieve_server_stats();
        display_metrics(args, get_server_status(), get_ms_delay(),
                        get_server_current_connections(), get_server_total_connections(),
                        get_server_transferred_bytes(), get_server_sent_mails());
        sleep(1);
    }

    finish:

    close_connection();

    if (stream != NULL)
        pclose(stream);


    return 0;
}
