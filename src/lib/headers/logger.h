#ifndef TPE_PDC_LOGGER_H
#define TPE_PDC_LOGGER_H

int init_logger(char * logLocation);

void close_logger(void);

int log_data(char *str, ...);

void update_logger_content(void);

#endif //TPE_PDC_LOGGER_H
