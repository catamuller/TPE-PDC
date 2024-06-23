
#ifndef TPE_PDC_CONNECTIONS_SERVER_HANDLER_H
#define TPE_PDC_CONNECTIONS_SERVER_HANDLER_H

#include <unistd.h>

int connect_to_server(void);

void close_connection(void);

int send_request(char* msg, char* buff, size_t buff_size);

void check_server_status(void);

char* get_server_status(void);

#endif //TPE_PDC_CONNECTIONS_SERVER_HANDLER_H
