
#ifndef TPE_PDC_CONNECTIONS_SERVER_HANDLER_H
#define TPE_PDC_CONNECTIONS_SERVER_HANDLER_H

#include <unistd.h>

int connect_to_server(char * ip, char * port);

void close_connection(void);

int send_request(char* msg, char* buff, size_t buff_size);

void check_server_status(void);

char* get_server_status(void);

double get_ms_delay(void);

void retrieve_server_stats(void);

int get_server_current_connections(void);

int get_server_total_connections(void);

int get_server_transferred_bytes(void);

#endif //TPE_PDC_CONNECTIONS_SERVER_HANDLER_H
