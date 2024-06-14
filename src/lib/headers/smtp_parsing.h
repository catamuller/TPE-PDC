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

  RCPT_R = 13,
  RCPT_C = 14,
  RCPT_P = 15,
  RCPT_T = 16,

  DATA_D = 17,
  DATA_A = 18,
  DATA_T = 19,
  DATA_FINAL_A = 20,

  RSET_R = 21,
  RSET_S = 22,
  RSET_E = 23,
  RSET_T = 24,

  NOOP_N = 25,
  NOOP_O = 26,
  NOOP_SECOND_O = 27,
  NOOP_P = 28,

  QUIT_Q = 29,
  QUIT_U = 30,
  QUIT_I = 31,
  QUIT_T = 32,

  VRFY_V = 33,
  VRFY_R = 34,
  VRFY_F = 35,
  VRFY_Y = 36,

  R = 37,

  NEQ = 38,
  EQ = 39,

  SPACE = 40,
  MAIL_SPACE = 41,
  RCPT_SPACE = 42
};

struct parser * smtp_parser_init();
const struct parser_event * smtp_parser_feed(struct parser * p, const uint8_t c);
#endif