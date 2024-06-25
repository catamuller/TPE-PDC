#ifndef TPE_PDC_LOGGER_H
#define TPE_PDC_LOGGER_H

#include <stdbool.h>

int init_logger(char * logLocation);

void close_logger(void);

int log_data(char *str, ...);

void update_logger_content(void);

void set_logging(bool value);

#endif //TPE_PDC_LOGGER_H
