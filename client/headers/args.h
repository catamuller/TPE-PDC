#ifndef TPE_PDC_ARGS_H
#define TPE_PDC_ARGS_H


#include <stdbool.h>

struct clientsargs {

    char * server_addr;
    char * server_port;


};

void parse_args(const int argc, char **argv, struct clientsargs *args);

#endif //TPE_PDC_ARGS_H
