#ifndef SMTP_PARSING_H
#define SMTP_PARSING_H

#include "parser_utils.h"
#include "buffer.h"
#include "smtp.h"
#define MAX_STATES 524
#define MAX_RCPT_TO 500

enum smtp_req_cmd {
  EHLO_CMD = 0x01,
  HELO_CMD = 0x02,
  MAIL_CMD = 0x03,
  RCPT_CMD = 0x04,
  DATA_CMD = 0x05,
  RSET_CMD = 0x06,
  NOOP_CMD = 0x07,
  QUIT_CMD = 0x08,
  VRFY_CMD = 0x09
};

enum smtp_reply_status_codes {
  status_system                                                 = 211,
  status_help                                                   = 214,
  status_service_ready                                          = 220,
  status_service_closing                                        = 221,
  status_action_okay                                            = 250,
  status_user_not_local                                         = 251,
  status_cannot_verfy_user                                      = 252,
  status_start_mail_input                                       = 354,
  status_service_not_available                                  = 421,
  status_mailbox_unavailable                                    = 450,
  status_local_error_in_processing                              = 451,
  status_insufficient_system_storage                            = 452,
  status_unable_to_accommodate_params                           = 455,
  status_syntax_error_no_command                                = 500,
  status_syntax_error_in_parameters                             = 501,
  status_cmd_not_implemented                                    = 502,
  status_bad_seq_cmds                                           = 503,
  status_cmd_parameter_not_implemented                          = 504,
  status_mailbox_not_found                                      = 550,
  status_pls_try_forward_path                                   = 551,
  status_exceeded_storage_allocation                            = 552,
  status_mailbox_name_not_allowed                               = 553,
  status_transaction_failed                                     = 554,
  status_mail_from_rcpt_to_params_unrecognized_not_implemented  = 555
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

enum data_state_names {
  DATA_ANY              = 0,
  DATA_FIRST_CR_STATE   = 1,
  DATA_FIRST_LF_STATE   = 2,
  DATA_DOT              = 3,
  DATA_SECOND_CR_STATE  = 4,
  DATA_SECOND_LF_STATE  = 5,
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

  HELO_DOMAIN_STATE = 40,
  HELO_CR_STATE = 41,
  HELO_LF_STATE = 42,

  EHLO_DOMAIN_STATE = 43,
  EHLO_CR_STATE = 44,
  EHLO_LF_STATE = 45,

  NEQ = 46,
  FROM_F = 47,
  FROM_R = 48,
  FROM_O = 49,
  FROM_M = 50,

  FROM_COLON = 51,
  FROM_SPACE = 52,
  WRONG_DOMAIN = 53,

  TO_T = 54,
  TO_O = 55,

  TO_COLON = 56,
  TO_SPACE = 57,

  RCPT_TO_CR_STATE = 58,
  RCPT_TO_LF_STATE = 59,

  QUIT_CR_STATE = 60,
  QUIT_LF_STATE = 61,

  DATA_CR_STATE = 62,
  DATA_LF_STATE = 63,

  PARSER_RESET_CR_STATE = 64,
  PARSER_RESET_LF_STATE = 65,

  STAT_S = 66,
  STAT_T = 67,
  STAT_A = 68,
  STAT_SECOND_T = 69,
  STAT_SPACE = 70,

  CURRENT_C = 71,
  CURRENT_U = 72,
  CURRENT_R = 73,
  CURRENT_SECOND_R = 74,
  CURRENT_E = 75,
  CURRENT_N = 76,
  CURRENT_T = 77,

  TOTAL_T = 78,
  TOTAL_O = 79,
  TOTAL_SECOND_T = 80,
  TOTAL_A = 81,
  TOTAL_L = 82,

  BYTES_B = 83,
  BYTES_Y = 84,
  BYTES_T = 85,
  BYTES_E = 86,
  BYTES_S = 87,

  CURRENT_CR_STATE = 88,
  CURRENT_LF_STATE = 89,

  TOTAL_CR_STATE = 90,
  TOTAL_LF_STATE = 91,

  BYTES_CR_STATE = 92,
  BYTES_LF_STATE = 93,

  NOOP_CR_STATE = 94,
  NOOP_LF_STATE = 95,

  RSET_CR_STATE = 96,
  RSET_LF_STATE = 97,

  XSTAT_X = 98,

  EQ = 99,
};

struct parser * smtp_parser_init(client_state * state);
struct parser * smtp_data_parser_init(client_state * state);
void sendMail(client_state * state);
const struct parser_event * smtp_parser_feed(struct parser * p, const uint8_t c);
const struct parser_event * smtp_parser_consume(buffer * buff, struct parser * p, client_state * state);
const struct parser_event * smtp_data_parser_consume(buffer * buff, struct parser * p, client_state * state, size_t * data_size);
#endif
