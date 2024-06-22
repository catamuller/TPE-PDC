#ifndef TPE_PDC_METRICS_HANDLER_H
#define TPE_PDC_METRICS_HANDLER_H

void close_metrics(void);

int add_to_active_connections(unsigned net1, unsigned net2, unsigned net3, unsigned host, unsigned port);

void reset_active_connections(void);

void dump_active_connections(void);

#endif //TPE_PDC_METRICS_HANDLER_H
