
#ifndef TPE_PDC_CLIENT_SERVER_CONNECTIONS_H
#define TPE_PDC_CLIENT_SERVER_CONNECTIONS_H

#include <unistd.h>

int connect_to_server(char * ip, char * port);

void close_connection(void);

int send_request(char* msg, char* buff, size_t buff_size);




#endif //TPE_PDC_CLIENT_SERVER_CONNECTIONS_H
