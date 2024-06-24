#ifndef TPE_PDC_ARGS_H
#define TPE_PDC_ARGS_H


#include <stdbool.h>

#define MAX_USERS 600

struct user {
    char *name;
    char *pass;
};

struct metricargs {

    char *          server_addr;
    unsigned short  server_port;

    bool            disectors_enabled;

};

/**
 * Interpreta la linea de comandos (argc, argv) llenando
 * args con defaults o la seleccion humana. Puede cortar
 * la ejecución.
 */
void parse_args(const int argc, char **argv, struct smtpargs *args);

#endif //TPE_PDC_ARGS_H
