#ifndef SMTP_H
#define SMTP_H

#define BUFFER_MAX_SIZE 1024
#define MAX_RCPT_TO 500
#include "selector.h"



typedef struct client_state {
  size_t rcptToIndex ;
  size_t rcptToTotal ;
  size_t clientRcptToIndex;
  char rcptTo[MAX_RCPT_TO][BUFFER_MAX_SIZE];
  char shortRcptTo[MAX_RCPT_TO][BUFFER_MAX_SIZE];

  size_t mailFromIndex ;
  char mailFrom[BUFFER_MAX_SIZE];

  char subject[BUFFER_MAX_SIZE];
  char randomId[BUFFER_MAX_SIZE];

  size_t dataIndex;
  char data[4*BUFFER_MAX_SIZE];

  char user[BUFFER_MAX_SIZE];
  size_t userIndex;
} client_state;

void smtp_passive_accept(struct selector_key *key);
char * get_state_name(unsigned smtp_state);

#endif 
