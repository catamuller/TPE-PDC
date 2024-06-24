#ifndef TPE_PDC_DISPLAY_H
#define TPE_PDC_DISPLAY_H

#include "args.h"

void display_metrics(struct metricsargs * args, char* server_status, double ms_delay, int active_connections, int historic_connections, int server_current_connections, int server_total_connections, int server_transferred_bytes);

#endif //TPE_PDC_DISPLAY_H
