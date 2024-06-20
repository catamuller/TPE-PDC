#ifndef SMTP_PARSING_H
#define SMTP_PARSING_H

#include "parser_utils.h"
#include "buffer.h"
#define MAX_STATES 524

enum smtp_req_cmd {
  EHLO = 0x01,
  HELO = 0x02,
  MAIL = 0x03,
  RCPT = 0x04,
  DATA = 0x05,
  RSET = 0x06,
  NOOP = 0x07,
  QUIT = 0x08,
  VRFY = 0x09
};

enum smtp_request_state {
  request_greeting,
  request_ehlo,
  request_helo,
  request_mail,
  request_rcpt,
  request_data,
  request_rset,
  request_noop,
  request_quit,
  request_vrfy
};

enum state_names {
  HELO_H = 1,
  HELO_E = 2,
  HELO_L = 3,
  HELO_O = 4,

  EHLO_E = 5,
  EHLO_H = 6,
  EHLO_L = 7,
  EHLO_O = 8,

  MAIL_M = 9,
  MAIL_A = 10,
  MAIL_I = 11,
  MAIL_L = 12,

  RCPT_C = 13,
  RCPT_P = 14,
  RCPT_T = 15,

  DATA_D = 16,
  DATA_A = 17,
  DATA_T = 18,
  DATA_FINAL_A = 19,

  RSET_S = 20,
  RSET_E = 21,
  RSET_T = 22,

  NOOP_N = 23,
  NOOP_O = 24,
  NOOP_SECOND_O = 25,
  NOOP_P = 26,

  QUIT_Q = 27,
  QUIT_U = 28,
  QUIT_I = 29,
  QUIT_T = 30,

  VRFY_V = 31,
  VRFY_R = 32,
  VRFY_F = 33,
  VRFY_Y = 34,

  R = 35,

  HELO_SPACE = 36,
  EHLO_SPACE = 37,
  MAIL_SPACE = 38,
  RCPT_SPACE = 39,

  NEQ = 90,
  EQ = 91,
};

struct parser * smtp_parser_init();
const struct parser_event * smtp_parser_feed(struct parser * p, const uint8_t c);
#endif