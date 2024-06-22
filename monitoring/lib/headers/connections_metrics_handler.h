#ifndef TPE_PDC_METRICS_HANDLER_H
#define TPE_PDC_METRICS_HANDLER_H

int init_metrics(void);

void close_metrics(void);

int add_to_active_connections(unsigned net1, unsigned net2, unsigned net3, unsigned host, unsigned port);

int add_to_historic_connections(unsigned net1, unsigned net2, unsigned net3, unsigned host, unsigned port);

void reset_active_connections(void);

void dump_active_connections(void);

void dump_historic_connections(void);

int get_active_connections(void);

int get_historic_connections(void);

#endif //TPE_PDC_METRICS_HANDLER_H
