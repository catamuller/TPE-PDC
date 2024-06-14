#include "headers/smtp_parsing.h"
#include <stdlib.h>

#define CR '\r'
#define LF '\n'

static const struct parser_state_transition ST_0[] = {
  {.when = 'H',     .dest = HELO_H,      .act1 = may_eq},
  {.when = 'h',     .dest = HELO_H,      .act1 = may_eq},
  
  {.when = 'E',     .dest = EHLO_E,      .act1 = may_eq},
  {.when = 'e',     .dest = EHLO_E,      .act1 = may_eq},

  {.when = 'M',     .dest = MAIL_M,      .act1 = may_eq},
  {.when = 'm',     .dest = MAIL_M,      .act1 = may_eq},

  {.when = 'R',     .dest = R,           .act1 = may_eq}, /* RCPT or RSET */
  {.when = 'r',     .dest = R,           .act1 = may_eq},

  {.when = 'D',     .dest = DATA_D,      .act1 = may_eq},
  {.when = 'd',     .dest = DATA_D,      .act1 = may_eq},

  {.when = 'N',     .dest = NOOP_N,      .act1 = may_eq},
  {.when = 'n',     .dest = NOOP_N,      .act1 = may_eq},

  {.when = 'Q',     .dest = QUIT_Q,      .act1 = may_eq},
  {.when = 'q',     .dest = QUIT_Q,      .act1 = may_eq},

  {.when = 'V',     .dest = VRFY_V,      .act1 = may_eq},
  {.when = 'v',     .dest = VRFY_V,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_1[] = {
  {.when = 'E',     .dest = HELO_E,      .act1 = may_eq},
  {.when = 'e',     .dest = HELO_E,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_2[] = {
  {.when = 'L',     .dest = HELO_L,      .act1 = may_eq},
  {.when = 'l',     .dest = HELO_L,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_3[] = {
  {.when = 'O',     .dest = HELO_O,      .act1 = may_eq},
  {.when = 'o',     .dest = HELO_O,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_4[] = {
  {.when = ' ',     .dest = SPACE,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_5[] = {
  {.when = 'H',     .dest = EHLO_H,      .act1 = may_eq},
  {.when = 'h',     .dest = EHLO_H,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_6[] = {
  {.when = 'L',     .dest = EHLO_L,      .act1 = may_eq},
  {.when = 'l',     .dest = EHLO_L,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_7[] = {
  {.when = 'O',     .dest = EHLO_O,      .act1 = may_eq},
  {.when = 'o',     .dest = EHLO_O,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_8[] = {
  {.when = ' ',     .dest = SPACE,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_9[] = {
  {.when = 'A',     .dest = MAIL_A,      .act1 = may_eq},
  {.when = 'a',     .dest = MAIL_A,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

struct parser * smtp_parser_init() {
  struct parser_definition * def = calloc(1, sizeof(*def));
  const struct parser_state_transition ** states = {
    ST_0,
    ST_1,
    ST_2,
    ST_3,
    ST_4,
    ST_5,
    ST_6,
    ST_7,
    ST_8,
    ST_9
  };
  def->states = states;
  struct parser * p = parser_init(parser_no_classes(), def);
}

const struct parser_event * smtp_parser_feed(struct parser * p, const uint8_t c) {
  return parser_feed(p, c);
}

struct parser_event * smtp_parser_consume(buffer * buff, struct parser * p) {
  struct parser_event * e1;
  bool CRflag = false;

  while(buffer_can_read(buff)) {
    const uint8_t c = buffer_read(buff);
    e1 = smtp_parser_feed(p, c);
    if (CRflag && c == LF)
      break;
    if (c == CR)
      CRflag = true;
  }
  return e1;
}