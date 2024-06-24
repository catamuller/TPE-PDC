#ifndef TPE_PDC_ARGS_H
#define TPE_PDC_ARGS_H


#include <stdbool.h>

struct metricsargs {

    char * server_addr;
    char * server_port;

    bool show_netstat;
    bool show_version;

};

void parse_args(const int argc, char **argv, struct metricsargs *args);

#endif //TPE_PDC_ARGS_H
