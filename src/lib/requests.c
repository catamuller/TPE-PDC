/**
 * requests.c -- parser de los requests de SMTP
 */
#include <string.h>
#include <arpa/inet.h>

#include "headers/requests.h"
#include "headers/buffer.h"

static void remaining_set(struct smtp_request_parser *p, const int n) {
  p->bytes_read = 0;
  p->bytes_remaining = n;
}

static bool remaining_is_done(struct smtp_request_parser *p) {
  return p->bytes_read >= p->bytes_remaining;
}

void smtp_request_parser_init(struct smtp_request_parser *p) {
  p->state = request_greeting;
  memset(p->request, 0, sizeof(*(p->request)));
}

enum smtp_request_state smtp_request_parser_feed(struct smtp_request_parser *p, const uint8_t c) {
  enum smtp_request_state next;
}

enum smtp_request_state request_consume(buffer *buff, struct smtp_request_parser *p) {
  enum smtp_request_state st = p->state;

  while(buffer_can_read(buff)) {
    const uint8_t c = buffer_read(buff);
    st = smtp_request_parser_feed(p, c);
  }
  return st;
}

void smtp_request_close(struct smtp_request_parser *p) {

}

enum smtp_reply_status_codes cmd_resolve(struct smtp_request* request) {
  
}