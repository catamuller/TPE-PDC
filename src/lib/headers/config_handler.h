#ifndef TPE_PDC_CONFIG_HANDLER_H
#define TPE_PDC_CONFIG_HANDLER_H

int init_config_socket(char *ip, int port);

void config_read(struct selector_key *key);

#endif //TPE_PDC_CONFIG_HANDLER_H
