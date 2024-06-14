
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
#include "headers/requests.h"

#define N(x) (sizeof(x)/sizeof((x)[0]))
#define ATTACHMENT(key) ( (struct smtp *)(key)->data)

#define BUFFER_MAX_SIZE 1024

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
   SERVER_HELLO,

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
   CLIENT_MAIL_FROM,

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
    ERROR,
    QUIT
};


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

    struct smtp_request request;
    struct smtp_request_parser request_parser;

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

static void client_hello_read_init(struct selector_key * key) {
    struct smtp_request_parser *st = &ATTACHMENT(key)->request_parser;
    st->request = &ATTACHMENT(key)->request;
    st->state = request_helo;
    memset(st->request, 0, sizeof(*(st->request)));
}

static void client_hello_read_close(const unsigned state, struct selector_key *key) { 
    smtp_request_close(&ATTACHMENT(key)->request_parser);
}

static void request_read_init(const unsigned state,struct selector_key *key){
    struct request_parser *st = &ATTACHMENT(key)->request_parser;
    st->request=&ATTACHMENT(key)->request;
    request_parser_init(st);
}

static void request_read_close(const unsigned state, struct selector_key *key) { 
    request_close(&ATTACHMENT(key)->request_parser);
}


static void smtp_destroy(struct smtp * smtp) {
  free(smtp);
}

static unsigned request_read(struct selector_key * key);
static unsigned client_hello(struct selector_key * key);
static const struct state_definition client_statbl[] = {
    {
        .state            = CLIENT_HELLO,
        .on_arrival       = client_hello_read_init,
        .on_departure     = client_hello_read_close,
        .on_write_ready   = client_hello,
    },
    {   .state            = SERVER_HELLO,
        .on_arrival       = request_read_init,
        .on_departure     = request_read_close,
        .on_read_ready    = request_read,
    },
    {
        .state=DONE,
    },
    {
        .state= ERROR,
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

    //TODO: creo que esto hay que volarlo mas adelante
    memcpy(&state->raw_buff_write, "Hello\n", 6);
    buffer_write(&state->write_buffer, 6);

    request_parser_init(&state->request_parser);

    if(SELECTOR_SUCCESS != selector_register(key->s, client, &smtp_handler, OP_READ, state)) {
        goto fail;
    }
    return ;
fail:
    if(client != -1) {
        close(client);
    }
    smtp_destroy(state);
}

static unsigned request_read(struct selector_key * key) {
    unsigned ret = SERVER_HELLO;
    char buffer[BUFFER_MAX_SIZE] = {0};

    struct smtp * state = ATTACHMENT(key);
    int r = recv(key->fd, buffer, 1023, 0);
    if(r > 0) {
        buffer[r] = 0;
        puts(buffer);

        buffer_write_adv(&state->read_buffer,r);
        bool error=false;
        int st=request_consume(&state->read_buffer,&state->request_parser,&error);

        selector_set_interest_key(key, OP_WRITE);

        ret = CLIENT_HELLO;
    } else {
        ret = ERROR;
    }

    return ret;
}

static unsigned client_hello(struct selector_key * key) {
    unsigned ret = CLIENT_HELLO;

    size_t count;
    buffer *b = &ATTACHMENT(key)->write_buffer;

    uint8_t *ptr = buffer_read_ptr(b, &count);
    ssize_t n = send(key->fd, "Hello\n", 6, MSG_NOSIGNAL);

    if(n >= 0) {
        buffer_read_adv(b, n);
        if(!buffer_can_read(b)) {
            if(SELECTOR_SUCCESS == selector_set_interest_key(key, OP_READ)) {
                ret = SERVER_HELLO;
            } else {
                ret = ERROR;
            }
        }
    } else {
        ret = ERROR;
    }

    return ret;
}

static void smtp_done(struct selector_key* key);

static void smtp_read(struct selector_key *key){
    struct state_machine *stm = &ATTACHMENT(key)->stm;
    const enum smtp_state st = stm_handler_read(stm, key);
    if (ERROR == st || DONE == st) {
        smtp_done(key);
    }
}
static void smtp_write(struct selector_key *key){
  struct state_machine *stm = &ATTACHMENT(key)->stm;
    const enum smtp_state st = stm_handler_write(stm, key);
    if (ERROR == st || DONE == st) {
        smtp_done(key);
    }
}
static void smtp_block(struct selector_key *key){
  struct state_machine *stm = &ATTACHMENT(key)->stm;
    const enum smtp_state st = stm_handler_block(stm, key);

    if (ERROR == st || DONE == st) {
        smtp_done(key);
    }
}
static void smtp_close(struct selector_key *key){
  
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