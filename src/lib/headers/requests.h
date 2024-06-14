#ifndef REQUESTS_H
#define REQUESTS_H

#include <stdint.h>
#include <stdbool.h>

#include <netinet/in.h>

#include "buffer.h"

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

struct smtp_request {
  enum smtp_req_cmd     cmd;
};

struct smtp_request_parser {
  struct smtp_request    *request;
  enum smtp_request_state state;
  /** cuantos bytes tenemos que leer */
  uint8_t bytes_remaining;
  /** cuantos bytes ya leimos */
  uint8_t bytes_read;
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
/** inicializa el parser */
void smtp_request_parser_init(struct smtp_request_parser *p);

/** entrega un byte al parser. */
enum smtp_request_state smtp_request_parser_feed(struct smtp_request_parser *p, const uint8_t c);

/**
 * @brief por cada elemento del buffer, llama a `smtp_request_parser_feed` hasta
 * que el parseo se encuentra completo o se requieran más bytes.
 * 
 */
enum smtp_request_state request_consume(buffer *buff, struct smtp_request_parser *p);

void smtp_request_close(struct smtp_request_parser *p);

enum smtp_reply_status_codes cmd_resolve(struct smtp_request* request);

#endif