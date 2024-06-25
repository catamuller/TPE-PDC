#ifndef SMTP_H
#define SMTP_H

#define BUFFER_MAX_SIZE 1024
#define MAX_RCPT_TO 500
#include "selector.h"



typedef struct client_state {
  size_t rcptToIndex ;
  size_t rcptToTotal ;
  size_t clientRcptToIndex;
  char rcptTo[MAX_RCPT_TO][BUFFER_MAX_SIZE];
  char shortRcptTo[MAX_RCPT_TO][BUFFER_MAX_SIZE];

  size_t mailFromIndex ;
  char mailFrom[BUFFER_MAX_SIZE];

  char subject[BUFFER_MAX_SIZE];
  char randomId[BUFFER_MAX_SIZE];

  size_t dataIndex;
  char data[4*BUFFER_MAX_SIZE];

  char user[BUFFER_MAX_SIZE];
  size_t userIndex;

  short currentState;
} client_state;

/** maquina de estados general */
enum smtp_state {
    SERVER_UNRECOGNIZED_CMD,
    SERVER_INVALID_MAIL_CMD,
    SERVER_INVALID_HELO_CMD,
    SERVER_INVALID_EHLO_CMD,
    SERVER_INVALID_RCPT_CMD,
    SERVER_INVALID_STAT_CMD,
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
   CLIENT_RSET,
   SERVER_RSET,
   CLIENT_NOOP,
   SERVER_NOOP,
   CLIENT_HELP,
   SERVER_HELP,
   CLIENT_VRFY,
   SERVER_VRFY,
   CLIENT_STAT_CURRENT_CONNECTIONS,
   SERVER_STAT_CURRENT_CONNECTIONS,
   CLIENT_STAT_TOTAL_CONNECTIONS,
   SERVER_STAT_TOTAL_CONNECTIONS,
   CLIENT_STAT_BYTES_TRANSFERED,
   SERVER_STAT_BYTES_TRANSFERED,
   CLIENT_STAT_MAILS_SENT,
   SERVER_STAT_MAILS_SENT,
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

void smtp_passive_accept(struct selector_key *key);
char * get_state_name(unsigned smtp_state);

#endif 
