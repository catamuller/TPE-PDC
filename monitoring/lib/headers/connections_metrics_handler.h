#ifndef TPE_PDC_METRICS_HANDLER_H
#define TPE_PDC_METRICS_HANDLER_H

int init_metrics(void);

void close_metrics(void);

int add_to_current_connections(unsigned net1, unsigned net2, unsigned net3, unsigned host, unsigned port);

int add_to_total_connections(unsigned net1, unsigned net2, unsigned net3, unsigned host, unsigned port);

void reset_current_connections(void);

void dump_current_connections(void);

void dump_total_connections(void);

int get_current_connections(void);

int get_total_connections(void);

#endif //TPE_PDC_METRICS_HANDLER_H
