#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <limits.h>    /* LONG_MIN et al */
#include <string.h>    /* memset */
#include <errno.h>
#include <getopt.h>

#include "headers/args.h"
#include "headers/config.h"


static void usage(const char *progname) {
    fprintf(stderr,
            "Usage: %s [OPTION]...\n"
            "\n"
            "   -h               Imprime la ayuda y termina.\n"
            "   -i <addr>        Define a qué ip el cliente intentará conectarse (Default en config.h).\n"
            "   -P <port>        Define a que puerto el cliente intentará conectarse (Default en config.h).\n"
            "\n",
            progname);
    exit(1);
}

void parse_args(const int argc, char **argv, struct clientsargs *args) {
    memset(args, 0, sizeof(*args));

    args->server_addr   = SERVER_IP;
    args->server_port   = SERVER_PORT;


    int c;

    while (true) {
        int option_index = 0;
        static struct option long_options[] = {
                { 0,           0,                 0, 0 }
        };

        c = getopt_long(argc, argv, "hi:P:vn", long_options, &option_index);

        if (c == -1)
            break;
        switch (c) {
            case 'h':
                usage(argv[0]);
                break;
            case 'i':
                args->server_addr = optarg;
                break;
            case 'P':
                args->server_port = optarg;
                break;
            default:
                fprintf(stderr, "unknown argument %d.\n", c);
                exit(1);
        }

    }
    if (optind < argc) {
        fprintf(stderr, "argument not accepted: ");
        while (optind < argc) {
            fprintf(stderr, "%s ", argv[optind++]);
        }
        fprintf(stderr, "\n");
        exit(1);
    }
}
