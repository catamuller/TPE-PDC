#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <limits.h>    /* LONG_MIN et al */
#include <string.h>    /* memset */
#include <errno.h>
#include <getopt.h>

#include "headers/args.h"

static unsigned short port(const char *s) {
     char *end     = 0;
     const long sl = strtol(s, &end, 10);

     if (end == s || '\0' != *end
        || ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno) 
        || sl < 0 || sl > USHRT_MAX) {
         fprintf(stderr, "port should in in the range of 1-65536: %s\n", s);
         exit(1);
         return 1;
     }
     return (unsigned short)sl;
}

static void user(char *s, struct user *user) {
    char *p = strchr(s, ':');
    if(p == NULL) {
        fprintf(stderr, "password not found\n");
        exit(1);
    } else {
        *p = 0;
        p++;
        user->name = s;
        user->pass = p;
    }

}

static void version(void) {
    fprintf(stderr, "smtpd version 0.0\n"
                    "ITBA Protocolos de Comunicación 2024/1 -- Grupo 8\n"
                    "AQUI VA LA LICENCIA\n");
}

static void usage(const char *progname) {
    fprintf(stderr,
        "Usage: %s [OPTION]...\n"
        "\n"
        "   -h               Imprime la ayuda y termina.\n"
        "   -l <addr>        Dirección donde servirá el servidor SMTP.\n"
        "   -P <con port>    Puerto entrante para conexiones.\n"
        "   -C <conf addr>   Dirección entrante para configuración.\n"
        "   -c <conf port>   Puerto entrante para configuración.\n"
        "   -u <name>:<pass> Usuario y contraseña de usuario que puede usar el proxy. Hasta 10.\n"
        "   -v               Imprime información sobre la versión y termina.\n"
        "\n",
        progname);
    exit(1);
}

void parse_args(const int argc, char **argv, struct smtpargs *args) {
    memset(args, 0, sizeof(*args)); // sobre todo para setear en null los punteros de users

    args->socks_addr = "0.0.0.0";
    args->socks_port = 2525;

    args->mng_addr   = "127.0.0.1";
    args->mng_port   = 8080;

    args->conf_addr  = "127.0.0.1";
    args->conf_port  = 2526;

    args->disectors_enabled = true;

    int c;
    int nusers = 0;

    while (true) {
        int option_index = 0;
        static struct option long_options[] = {
            { 0,           0,                 0, 0 }
        };

        c = getopt_long(argc, argv, "hl:P:u:C:c:v", long_options, &option_index);

        if (c == -1)
            break;
        switch (c) {
            case 'h':
                usage(argv[0]);
                break;
            case 'l':
                args->socks_addr = optarg;
                break;
            case 'P':
                args->mng_port   = port(optarg);
                break;
            case 'u':
                if(nusers >= MAX_USERS) {
                    fprintf(stderr, "maximun number of command line users reached: %d.\n", MAX_USERS);
                    exit(1);
                } else {
                    user(optarg, args->users + nusers);
                    nusers++;
                }
                break;
            case 'C':
                args->conf_addr = optarg;
                break;
            case 'c':
                args->conf_port = port(optarg);
                break;
            case 'v':
                version();
                exit(0);
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
