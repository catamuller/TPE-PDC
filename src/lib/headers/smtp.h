#ifndef SMTP_H
#define SMTP_H


#include "selector.h"

void smtp_passive_accept(struct selector_key *key);
char * get_state_name(unsigned smtp_state);

#endif 
