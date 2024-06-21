
#include <stdio.h>
#include <stdlib.h>  // malloc
#include <string.h>  // memset
#include <assert.h>  // assert
#include <errno.h>
#include <time.h>
#include <unistd.h>  // close
#include <pthread.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include "headers/buffer.h"
#include "headers/stm.h"
#include "headers/smtp.h"
//#include "headers/requests.h"
#include "headers/smtp_parsing.h"
#include "headers/parser.h"

#define N(x) (sizeof(x)/sizeof((x)[0]))
#define ATTACHMENT(key) ( (struct smtp *)(key)->data)

#define BUFFER_MAX_SIZE 1024

#define GOTO_PREVIOUS MAX_STATES

/** maquina de estados general */
enum smtp_state {
    /**
     * recibe el mensaje `HELLO` del cliente, y lo procesa
     *
     * Intereses:
     *     - OP_READ sobre client_fd
     *
     * Transiciones:
     *   - CLIENT_HELLO  mientras el mensaje no esté completo
     *   - SERVER_HELLO  cuando está completo
     *   - ERROR         ante cualquier error (IO/parseo)
     *   - QUIT
     */
    CLIENT_HELLO,

    /**
     * envía la respuesta del `HELLO` al cliente.
     *
     * Intereses:
     *     - OP_WRITE sobre client_fd
     *
     * Transiciones:
     *   - SERVER_HELLO          mientras queden bytes por enviar
     *   - CLIENT_MAIL_FROM      cuando se enviaron todos los bytes -> acá falta autenticación antes de esta transición
     *   - ERROR                 ante cualquier error (IO/parseo)
     */
   SERVER_NO_GREETING,
   SERVER_HELLO,
   SERVER_EHLO,
   SERVER_ALREADY_GREETED,

    /**
     * recibe el mensaje `MAIL FROM` del cliente, y lo procesa
     * 
     * Intereses:
     *      - OP_READ sobre client_fd
     * 
     * Transiciones:
     *    - CLIENT_MAIL_FROM mientras el mensaje no esté completo
     *    - SERVER_MAIL_FROM cuando esté completo
     *    - ERROR            ante cualquier error (IO/parseo)
     *    - QUIT
     */
   SERVER_NO_MAIL,
   CLIENT_MAIL_FROM,
   SERVER_ALREADY_MAIL,

    /**
     * envía la respuesta del `MAIL FROM` al cliente.
     * 
     * Intereses:
     *      - OP_WRITE sobre client_fd
     * 
     * Transiciones:
     *   - SERVER_MAIL_FROM mientras queden bytes por enviar
     *   - CLIENT_RCPT_TO   cuando se enviaron todos los bytes
     *   - ERROR            ante cualquier error (IO/parseo)
     */
   SERVER_MAIL_FROM,
   SERVER_WRONG_DOMAIN,

    /**
     * recibe el mensaje de `RCPT TO` del cliente, y lo procesa
     * 
     * Intereses:
     *      - OP_READ sobre client_fd
     * 
     * Transiciones:
     *   - CLIENT_RCPT_TO mientras el mensaje no esté completo
     *   - SERVER_RCPT_TO cuando esté completo
     *   - ERROR          ante cualquier error (IO/parseo)
     *   - QUIT
     */
   SERVER_NO_RCPT,
   CLIENT_RCPT_TO,

    /**
     * envía la respuesta del `RCPT TO` al cliente.
     * 
     * Intereses:
     *      - OP_WRITE sobre client_fd
     * 
     * Transiciones:
     *   - SERVER_RCPT_TO mientras queden bytes por enviar
     *   - CLIENT_DATA    cuando se enviaron todos los bytes
     *   - ERROR          ante cualquier error (IO/parseo)
     */
   SERVER_RCPT_TO,

    /**
     * recibe el mensaje de `DATA` del cliente
     * 
     * Intereses:
     *      - OP_READ sobre client_fd
     * 
     * Transiciones:
     *   - SERVER_DATA siempre
     *   - ERROR       ante cualquier error (IO/parseo)
     */
   CLIENT_DATA,

    /**
     * envía la respuesta del `DATA` al cliente
     * 
     * Intereses:
     *      - OP_WRITE sobre client_fd
     * 
     * Transiciones:
     *   - SERVER_DATA         mientras queden bytes por enviar
     *   - CLIENT_MAIL_CONTENT cuando se enviaron todos los bytes
     *   - ERROR               ante cualquier error (IO/parseo)
     */
   SERVER_DATA,
   
    /**
     * recibe el contenido del mail por parte del cliente (menos <CR><LF>.<CR><LF>)
     * 
     * Intereses:
     *      - OP_READ sobre client_fd
     * 
     * Transiciones:
     *   - CLIENT_MAIL_CONTENT si el contenido no contiene un `<CR><LF>.<CR><LF>`
     *   - SERVER_MAIL_END     si el contenido final es un `<CR><LF>.<CR><LF>`
     *   - ERROR               ante cualquier error (IO/parseo)
     */
   CLIENT_MAIL_CONTENT,

    /**
     * envía la respuesta de `queued` del contenido del MAIL.
     * 
     * Intereses:
     *      - OP_WRITE sobre client_fd
     * 
     * Transiciones:
     *   - CLIENT_MAIL_FROM siempre
     *   - ERROR            ante cualquier error (IO/parseo)
     */
   SERVER_MAIL_END,

    // estados terminales
   SERVER_WRONG_ARGUMENTS,
   ERROR,
   QUIT,    // todo: cierra todo
   CLOSE
};

static char* state_names[] = {"CLIENT_HELLO", "SERVER_NO_GREETING", "SERVER_HELLO",
                             "SERVER_EHLO", "SERVER_ALREADY_GREETED", "SERVER_NO_MAIL", "CLIENT_MAIL_FROM", "SERVER_ALREADY_MAIL",
                             "SERVER_MAIL_FROM", "SERVER_WRONG_DOMAIN", "SERVER_NO_RCPT", "CLIENT_RCPT_TO", "SERVER_RCPT_TO",
                             "CLIENT_DATA", "SERVER_DATA", "CLIENT_MAIL_CONTENT", "SERVER_MAIL_END", "SERVER_WRONG_ARGUMENTS",
                             "ERROR", "QUIT", "CLOSE"};


struct smtp {

    /** informacion del cliente */
    size_t client_fd;
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len;

    /** maquinas de estados */
    struct state_machine stm;

    /** buffers para ser usados read_buffer, write_buffer*/
    uint8_t raw_buff_read[2048], raw_buff_write[2048];
    buffer read_buffer, write_buffer;

    // struct smtp_request request;
    // struct smtp_request_parser request_parser;
    Parser smtp_parser;
    Parser smtp_data_parser;


};

static void smtp_read(struct selector_key *key);
static void smtp_write(struct selector_key *key);
static void smtp_block(struct selector_key *key);
static void smtp_close(struct selector_key *key);

static const struct fd_handler smtp_handler = {
    .handle_read   = smtp_read,
    .handle_write  = smtp_write,
    .handle_close  = smtp_close,
    .handle_block  = smtp_block,
};

// static void request_read_init(const unsigned state,struct selector_key *key){
//     struct request_parser *st = &ATTACHMENT(key)->request_parser;
//     st->request=&ATTACHMENT(key)->request;
//     request_parser_init(st);
// }

// static void request_read_close(const unsigned state, struct selector_key *key) { 
//     request_close(&ATTACHMENT(key)->request_parser);
// }


static void smtp_destroy(struct smtp * smtp) {
  free(smtp);
}

static unsigned client_read(struct selector_key * key);
static unsigned server_no_greeting(struct selector_key * key);
static unsigned server_hello(struct selector_key * key);
static unsigned server_ehlo(struct selector_key * key);
static unsigned server_already_greeted(struct selector_key * key);
static unsigned server_no_mail(struct selector_key *key);
static unsigned server_mail_from(struct selector_key *key);
static unsigned server_already_mail(struct selector_key *key);
static unsigned server_wrong_domain(struct selector_key *key);
static unsigned server_no_rcpt(struct selector_key *key);
static unsigned server_rcpt_to(struct selector_key *key);
static unsigned server_data(struct selector_key *key);
static unsigned client_read_data(struct selector_key * key);
static unsigned server_mail_end(struct selector_key *key);
static unsigned server_wrong_arguments(struct selector_key *key);
static unsigned server_error(struct selector_key *key);
static unsigned server_quit(struct selector_key *key);
static unsigned server_close(struct selector_key *key);

static const struct state_definition client_statbl[] = {
    {
        .state            = CLIENT_HELLO,
        .on_arrival       = NULL,
        .on_departure     = NULL,
        .on_read_ready    = client_read,
    },
    {
        .state            = SERVER_NO_GREETING,
        .on_arrival       = NULL,
        .on_departure     = NULL,
        .on_write_ready   = server_no_greeting
    },
    {
        .state            = SERVER_HELLO,
        .on_arrival       = NULL,
        .on_departure     = NULL,
        .on_write_ready   = server_hello,
    },
    {
        .state            = SERVER_EHLO,
        .on_arrival       = NULL,
        .on_departure     = NULL,
        .on_write_ready   = server_ehlo,
    },
    {
        .state            = SERVER_ALREADY_GREETED,
        .on_arrival       = NULL,
        .on_departure     = NULL,
        .on_write_ready   = server_already_greeted
    },
    {
        .state            = SERVER_NO_MAIL,
        .on_arrival       = NULL,
        .on_departure     = NULL,
        .on_write_ready   = server_no_mail
    },
    {
        .state            = CLIENT_MAIL_FROM,
        .on_arrival       = NULL,
        .on_departure     = NULL,
        .on_read_ready    = client_read,
    },
    {
        .state            = SERVER_ALREADY_MAIL,
        .on_arrival       = NULL,
        .on_departure     = NULL,
        .on_write_ready   = server_already_mail
    },
    {
        .state            = SERVER_MAIL_FROM,
        .on_arrival       = NULL,
        .on_departure     = NULL,
        .on_write_ready   = server_mail_from,
    },
    {
        .state            = SERVER_WRONG_DOMAIN,
        .on_arrival       = NULL,
        .on_departure     = NULL,
        .on_write_ready   = server_wrong_domain
    },
    {
        .state            = SERVER_NO_RCPT,
        .on_arrival       = NULL,
        .on_departure     = NULL,
        .on_write_ready   = server_no_rcpt
    },
    {
        .state            = CLIENT_RCPT_TO,
        .on_arrival       = NULL,
        .on_departure     = NULL,
        .on_read_ready    = client_read,
    },
    {
        .state            = SERVER_RCPT_TO,
        .on_arrival       = NULL,
        .on_departure     = NULL,
        .on_write_ready   = server_rcpt_to,
    },
    {
        .state            = CLIENT_DATA,
        .on_arrival       = NULL,
        .on_departure     = NULL,
        .on_read_ready    = client_read,
    },
    {
        .state            = SERVER_DATA,
        .on_arrival       = NULL,
        .on_departure     = NULL,
        .on_write_ready   = server_data
    },
    {
        .state            = CLIENT_MAIL_CONTENT,
        .on_arrival       = NULL,
        .on_departure     = NULL,
        .on_read_ready    = client_read_data         // TODO - cambiar por otra función que acepte <CR><LF>.<CR><LF>!!
    },
    {
        .state            = SERVER_MAIL_END,
        .on_arrival       = NULL,
        .on_departure     = NULL,
        .on_write_ready   = server_mail_end
    },
    {
        .state            = SERVER_WRONG_ARGUMENTS,
        .on_arrival       = NULL,
        .on_departure     = NULL,
        .on_write_ready   = server_wrong_arguments
    },
    {
        .state            = ERROR,
        .on_arrival       = NULL,
        .on_departure     = NULL,
        .on_write_ready   = server_error
    },
    {
        .state            = QUIT,
        .on_arrival       = NULL,
        .on_departure     = NULL,
        .on_write_ready   = server_quit
    },
    {
        .state            = CLOSE,
        .on_arrival       = NULL,
        .on_departure     = NULL,
        .on_write_ready   = server_close
    }
};


/** Intenta aceptar la nueva conexión entrante*/
void smtp_passive_accept(struct selector_key * key) {
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    struct smtp * state = NULL;

    const int client = accept(key->fd, (struct sockaddr*) &client_addr,&client_addr_len);
    if(client == -1) {
        goto fail;
    }
    if(selector_fd_set_nio(client) == -1) {
        goto fail;
    }
    state = malloc(sizeof(struct smtp));
    if(state == NULL) {
        // sin un estado, nos es imposible manejaro.
        // tal vez deberiamos apagar accept() hasta que detectemos
        // que se liberó alguna conexión.
        goto fail;
    }
    memset(state, 0, sizeof(*state)); //para que este en blanco
    memcpy(&state->client_addr, &client_addr, client_addr_len);
    state->client_addr_len = client_addr_len;
    state->client_fd = client;

    buffer_init(&state->read_buffer, N(state->raw_buff_read), state->raw_buff_read);
    buffer_init(&state->write_buffer, N(state->raw_buff_write), state->raw_buff_write);
    state->stm.states = client_statbl;
    state->stm.initial = CLIENT_HELLO;
    state->stm.max_state = CLOSE;
    stm_init(&state->stm);
    // TODO: creo que esto hay que volarlo mas adelante
    // memcpy(&state->raw_buff_write, "Hello\n", 6);
    // buffer_write(&state->write_buffer, 6);
    state->smtp_parser = smtp_parser_init();
    state->smtp_data_parser = smtp_data_parser_init();
    //smtp_request_parser_init(&state->request_parser);

    if(SELECTOR_SUCCESS != selector_register(key->s, client, &smtp_handler, OP_READ, state)) {
        goto fail;
    }
    return;
fail:
    if(client != -1) {
        close(client);
    }
    smtp_destroy(state);
}

// static unsigned request_read(struct selector_key * key) {
//     unsigned ret = SERVER_HELLO;
//     char buffer[BUFFER_MAX_SIZE] = {0};
// 
//     struct smtp * state = ATTACHMENT(key);
//     int r = recv(key->fd, buffer, 1023, 0);
//     if(r > 0) {
//         buffer[r] = 0;
//         puts(buffer);
// 
//         buffer_write_adv(&state->read_buffer,r);
//         bool error=false;
//         int st=request_consume(&state->read_buffer,&state->request_parser,&error);
// 
//         selector_set_interest_key(key, OP_WRITE);
// 
//         ret = CLIENT_HELLO;
//     } else {
//         ret = ERROR;
//     }
// 
//     return ret;
// }

static unsigned client_read(struct selector_key * key) {

    //char buffer[BUFFER_MAX_SIZE] = {0};

    struct smtp * state = ATTACHMENT(key);
    unsigned ret = ATTACHMENT(key)->stm.current->state;
    int r = recv(key->fd, state->read_buffer.read, BUFFER_MAX_SIZE-1, 0);
    if(r > 0) {
        state->read_buffer.data[r] = 0;
        puts((char*)state->read_buffer.read);
        buffer_write_adv(&state->read_buffer, r);
        struct parser_event * event = smtp_parser_consume(&state->read_buffer, state->smtp_parser);
        switch(event->type) {
            case HELO_CMP_EQ:
                selector_set_interest_key(key, OP_WRITE);
                buffer_reset(&state->read_buffer);
                parser_reset(state->smtp_parser);
                // if (state->stm.current->state < CLIENT_HELLO) {
                //     ret = SERVER_NO_GREETING;
                //     break;
                // }
                if (state->stm.current->state > CLIENT_HELLO) {
                    ret = SERVER_ALREADY_GREETED;
                    break;
                }
                ret = SERVER_HELLO;
                break;
            case EHLO_CMP_EQ:
                selector_set_interest_key(key, OP_WRITE);
                buffer_reset(&state->read_buffer);
                parser_reset(state->smtp_parser);
                // if (state->stm.current->state < CLIENT_HELLO) {
                //     ret = SERVER_NO_GREETING;
                //     break;
                // }
                if (state->stm.current->state > CLIENT_HELLO) {
                    ret = SERVER_ALREADY_GREETED;
                    break;
                }
                ret = SERVER_EHLO;
                break;
            case MAIL_FROM_CMP_EQ:
                selector_set_interest_key(key, OP_WRITE);
                buffer_reset(&state->read_buffer);
                parser_reset(state->smtp_parser);
                if (state->stm.current->state < CLIENT_MAIL_FROM) {
                    ret = SERVER_NO_GREETING;
                    break;
                }
                if (state->stm.current->state > CLIENT_MAIL_FROM) {
                    ret = SERVER_ALREADY_MAIL;
                    break;
                }
                ret = SERVER_MAIL_FROM;
                break;
            case RCPT_TO_CMP_EQ:
                selector_set_interest_key(key, OP_WRITE);
                buffer_reset(&state->read_buffer);
                parser_reset(state->smtp_parser);
                if (state->stm.current->state < CLIENT_RCPT_TO) {
                    ret = SERVER_NO_MAIL;
                    break;
                }
                ret = SERVER_RCPT_TO;
                break;
            case DATA_CMP_EQ:
                selector_set_interest_key(key, OP_WRITE);
                buffer_reset(&state->read_buffer);
                parser_reset(state->smtp_parser);
                if (state->stm.current->state < CLIENT_DATA) {
                    ret = SERVER_NO_RCPT;
                    break;
                }
                // if (state->stm.current->state > CLIENT_DATA) {
                //     ret = SERVER_ALREADY_DATA;
                //     break;
                // }
                ret = SERVER_DATA;
                break;
            // case CLIENT_DATA_CMP_EQ:
            //     selector_set_interest_key(key, OP_WRITE);
            //     buffer_reset(&state->read_buffer);
            //     parser_reset(state->smtp_parser);
            //     sendMail();
            //     ret = SERVER_MAIL_END;
            //     break;
            case NEQ_DOMAIN:
                selector_set_interest_key(key, OP_WRITE);
                buffer_reset(&state->read_buffer);
                parser_reset(state->smtp_parser);
                if (state->stm.current->state < CLIENT_MAIL_FROM) {
                    ret = SERVER_NO_GREETING;
                    break;
                }
                if (state->stm.current->state > CLIENT_MAIL_FROM) {
                    ret = SERVER_ALREADY_MAIL;
                    break;
                }
                ret = SERVER_WRONG_DOMAIN;
                break;
            case STRING_CMP_NEQ:
                selector_set_interest_key(key, OP_WRITE);
                buffer_reset(&state->read_buffer);
                parser_reset(state->smtp_parser);
                ret = SERVER_WRONG_ARGUMENTS;
                break;
            case QUIT_CMP_EQ:
                selector_set_interest_key(key, OP_WRITE);
                buffer_reset(&state->read_buffer);
                parser_reset(state->smtp_parser);
                ret = CLOSE;
                break;
            case PARSER_RESET_CMP_EQ:
                selector_set_interest_key(key, OP_READ);
                buffer_reset(&state->read_buffer);
                parser_reset(state->smtp_parser);
                break;
            default:
                selector_set_interest_key(key, OP_READ);
        }
        
        //buffer_write_adv(&state->read_buffer,r);
        //bool error=false;
        //int st=(&state->read_buffer,&state->smtp_parser,&error);

    } else {
        ret = ERROR;
    }

    return ret;
}

static unsigned client_read_data(struct selector_key * key) {
    struct smtp * state = ATTACHMENT(key);
    unsigned ret = CLIENT_MAIL_CONTENT;
    int r = recv(key->fd, state->read_buffer.read, BUFFER_MAX_SIZE-1, 0);
    if(r > 0) {
        state->read_buffer.data[r] = 0;
        puts((char*)state->read_buffer.read);
        buffer_write_adv(&state->read_buffer, r);
        struct parser_event * event = smtp_data_parser_consume(&state->read_buffer, state->smtp_data_parser);
        if (event->type == CLIENT_DATA_CMP_EQ) {
            selector_set_interest_key(key, OP_WRITE);
            buffer_reset(&state->read_buffer);
            parser_reset(state->smtp_parser);
            parser_reset(state->smtp_data_parser);
            sendMail();
            ret = SERVER_MAIL_END;
        }
    } else {
        ret = ERROR;
    }
    return ret;
}


static unsigned server_template(struct selector_key * key, int returnValue, const char * format, int code, int success) {
    unsigned ret = returnValue;

    //size_t count;
    buffer *b = &ATTACHMENT(key)->write_buffer;

    //uint8_t *ptr = buffer_read_ptr(b, &count);
    char buffer[BUFFER_MAX_SIZE];
    sprintf(buffer,format,code);
    ssize_t n = send(key->fd, buffer, strlen(buffer), MSG_NOSIGNAL);
    //strcpy((char*)b->write, buffer);
    if(n >= 0) {
        //buffer_read_adv(b, n);
        if(!buffer_can_read(b)) {
            if(SELECTOR_SUCCESS == selector_set_interest_key(key, OP_READ)) {
                if (success < GOTO_PREVIOUS)
                    ret = success;
                else
                    ret = ATTACHMENT(key)->stm.previous->state;
            } else {
                ret = ERROR;
            }
        }
        buffer_reset(b);
    } else {
        ret = ERROR;
    }

    return ret;
} 

static unsigned server_no_greeting(struct selector_key * key) {
    return server_template(key, SERVER_NO_GREETING, "%d - Say hi!\n", status_user_not_local, CLIENT_HELLO);
}

static unsigned server_hello(struct selector_key * key) {
    return server_template(key, SERVER_HELLO, "%d - OK\n", status_action_okay, CLIENT_MAIL_FROM);
}

static unsigned server_ehlo(struct selector_key * key) {
    return server_template(key,
    SERVER_EHLO,
    "%1$d - MAIL\n"
    "%1$d - RCPT\n"
    "%1$d - DATA\n"
    "%1$d - RSET\n"
    "%1$d - NOOP\n"
    "%1$d - QUIT\n"
    "%1$d - VRFY\n",
    status_action_okay,
    CLIENT_MAIL_FROM
    );
}

static unsigned server_already_greeted(struct selector_key * key) {
    return server_template(key, SERVER_ALREADY_GREETED, "%d - Already greeted!\n", status_bad_seq_cmds, GOTO_PREVIOUS);
}

static unsigned server_no_mail(struct selector_key *key) {
    return server_template(key, SERVER_NO_MAIL, "%d - Need MAIL command first!\n", status_bad_seq_cmds, GOTO_PREVIOUS);
}

static unsigned server_mail_from(struct selector_key *key) {
    return server_template(key, SERVER_MAIL_FROM, "%d - OK\n", status_action_okay, CLIENT_RCPT_TO);
}

static unsigned server_already_mail(struct selector_key *key) {
    return server_template(key, SERVER_ALREADY_MAIL, "%d - Nested MAIL command\n", status_bad_seq_cmds, GOTO_PREVIOUS);
}

static unsigned server_wrong_domain(struct selector_key *key) {
    return server_template(key, SERVER_WRONG_DOMAIN, "%d - Invalid specified domain\n", status_mailbox_not_found ,GOTO_PREVIOUS);
}

static unsigned server_no_rcpt(struct selector_key *key) {
    return server_template(key, SERVER_NO_RCPT, "%d - Need RCPT command first!\n", status_bad_seq_cmds, GOTO_PREVIOUS);
}

static unsigned server_rcpt_to(struct selector_key *key) {
    return server_template(key, SERVER_RCPT_TO, "%d - OK\n",status_action_okay, CLIENT_DATA);
}

static unsigned server_data(struct selector_key *key) {
    return server_template(key, SERVER_DATA, "%d - OK. End with <CR><LF>.<CR><LF>\n", status_action_okay, CLIENT_MAIL_CONTENT);
}

static unsigned server_mail_end(struct selector_key *key) {
    return server_template(key, SERVER_MAIL_END, "%d - queued!\n", status_action_okay, CLIENT_MAIL_FROM);
}

static unsigned server_wrong_arguments(struct selector_key *key) {
    return server_template(key, SERVER_WRONG_ARGUMENTS, "%d - Invalid arguments or syntax error\n", status_syntax_error_in_parameters, GOTO_PREVIOUS);
}
static unsigned server_error(struct selector_key *key) {
    return server_template(key, ERROR, "%d - An error has occured. Sorry!\n", status_local_error_in_processing, CLIENT_MAIL_FROM);
}
static unsigned server_quit(struct selector_key *key) {
    return server_template(key, QUIT, "%d - Bye bye!\n", status_service_closing, CLOSE);
}

static unsigned server_close(struct selector_key *key) {
    return server_template(key, CLOSE, "%d - Closing\n", status_service_closing, CLOSE);
}

static void smtp_done(struct selector_key* key);

static void smtp_read(struct selector_key *key){
    struct state_machine *stm = &ATTACHMENT(key)->stm;
    const enum smtp_state st = stm_handler_read(stm, key);
    if (ERROR == st || CLOSE == st) {
        smtp_done(key);
    }
}
static void smtp_write(struct selector_key *key){
  struct state_machine *stm = &ATTACHMENT(key)->stm;
    const enum smtp_state st = stm_handler_write(stm, key);
    if (ERROR == st || CLOSE == st) {
        smtp_done(key);
    }
}
static void smtp_block(struct selector_key *key){
  struct state_machine *stm = &ATTACHMENT(key)->stm;
    const enum smtp_state st = stm_handler_block(stm, key);

    if (ERROR == st || CLOSE == st) {
        smtp_done(key);
    }
}
static void smtp_close(struct selector_key *key){
  // TODO
}
static void smtp_done(struct selector_key* key) {
  const int fds[] = {
        ATTACHMENT(key)->client_fd,
    };
    for(unsigned i = 0; i < N(fds); i++) {
        if(fds[i] != -1) {
            if(SELECTOR_SUCCESS != selector_unregister_fd(key->s, fds[i])) {
                abort();
            }
            close(fds[i]);
        }
    }
}

char * get_state_name(unsigned smtp_state) {
    return state_names[smtp_state];
}