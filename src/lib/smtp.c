
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

#define N(x) (sizeof(x)/sizeof((x)[0]))

/** obtiene el struct (smtp *) desde la llave de selección */
#define ATTACHEMENT(key) ( (struct smtp*)(key)->data)

/** maquina de estados general */
enum smtp_state {
    /**
     * recibe el mensaje `hello` del cliente, y lo procesa
     *
     * Intereses:
     *     - OP_READ sobre client_fd
     *
     * Transiciones:
     *   - HELLO_READ  mientras el mensaje no esté completo
     *   - HELLO_WRITE cuando está completo
     *   - ERROR       ante cualquier error (IO/parseo)
     */
    RESPONSE_WRITE,

    /**
     * envía la respuesta del `hello' al cliente.
     *
     * Intereses:
     *     - OP_WRITE sobre client_fd
     *
     * Transiciones:
     *   - RESPONSE_WRITE  mientras queden bytes por enviar
     *   - REQUEST_READ cuando se enviaron todos los bytes
     *   - ERROR        ante cualquier error (IO/parseo)
     */
   REQUEST_READ,

   DATA_WRITE,

   DATA_READ,

    // estados terminales
    DONE,
    ERROR,
};


struct smtp {

    /** informacion del cliente */
    struct sockaddr_storage client_addr;
    socklen_t client_addr_len;

    /** maquinas de estados */
    struct state_machine stm;

    /** buffers para ser usados read_buffer, write_buffer*/
    uint8_t raw_buff_read[2048], raw_buff_write[2048];
    buffer read_buffer, write_buffer;

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

static void smtp_destroy(struct smtp * smtp) {
  free(smtp);
}

static unsigned request_read(struct selector_key * key);
static unsigned response_write(struct selector_key * key);
static const struct state_definition client_statbl[] = {
    {
        .state            = RESPONSE_WRITE,/*
        .on_arrival       = hello_read_init,
        .on_departure     = hello_read_close,*/
        .on_write_ready    = response_write,
    },
    {   .state            = REQUEST_READ,/*
        .on_arrival       = hello_read_init,
        .on_departure     = hello_read_close,*/
        .on_read_ready    = request_read,
    },
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

    buffer_init(&state->read_buffer, N(state->raw_buff_read), state->raw_buff_read);
    buffer_init(&state->write_buffer, N(state->raw_buff_write), state->raw_buff_write);

    //TODO: creo que esto hay que volarlo mas adelante
    memcpy(&state->raw_buff_write, "Hello\n", 6);
    buffer_write(&state->write_buffer, 6);

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
    unsigned ret = REQUEST_READ;

    char buffer[1024] = {0};
    int r = recv(key->fd, buffer, 1023, 0);
    if(r > 0) {
        buffer[r] = 0;
        puts(buffer);
        selector_set_interest_key(key, OP_WRITE);

        ret = RESPONSE_WRITE;
    } else {
        ret = ERROR;
    }

    return ret;
}

static unsigned response_write(struct selector_key * key) {
    unsigned ret = RESPONSE_WRITE;

    size_t count;
    buffer *b = &ATTACHEMENT(key)->write_buffer;

    uint8_t *ptr = buffer_read_ptr(b, &count);
    ssize_t n = send(key->fd, "Hello\n", 6, MSG_NOSIGNAL);

    if(n >= 0) {
        buffer_read_adv(b, n);
        if(!buffer_can_read(b)) {
            if(SELECTOR_SUCCESS == selector_set_interest_key(key, OP_READ)) {
                ret = REQUEST_READ;
            } else {
                ret = ERROR;
            }
        }
    } else {
        ret = ERROR;
    }

    return ret;
}

static void smtp_read(struct selector_key *key){

}
static void smtp_write(struct selector_key *key){

}
static void smtp_block(struct selector_key *key){

}
static void smtp_close(struct selector_key *key){
    
}