#ifndef TPE_PDC_DISPLAY_H
#define TPE_PDC_DISPLAY_H

#include "args.h"

void display_metrics(struct metricsargs * args, char* server_status, double ms_delay, int server_current_connections, int server_total_connections, int server_transferred_bytes, int sent_mails);

#endif //TPE_PDC_DISPLAY_H
