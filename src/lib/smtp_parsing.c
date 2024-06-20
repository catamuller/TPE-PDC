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
  {.when = ' ',     .dest = HELO_SPACE,       .act1 = may_eq},

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
  {.when = ' ',     .dest = EHLO_SPACE,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_9[] = {
  {.when = 'A',     .dest = MAIL_A,      .act1 = may_eq},
  {.when = 'a',     .dest = MAIL_A,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_10[] = {
  {.when = 'I',     .dest = MAIL_I,      .act1 = may_eq},
  {.when = 'i',     .dest = MAIL_I,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_11[] = {
  {.when = 'L',     .dest = MAIL_L,      .act1 = may_eq},
  {.when = 'l',     .dest = MAIL_L,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_12[] = {
  {.when = ' ',     .dest = MAIL_SPACE,      .act1 = may_eq},
  {.when = ' ',     .dest = MAIL_SPACE,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};


static const struct parser_state_transition ST_13[] = {
  {.when = 'P',     .dest = RCPT_P,      .act1 = may_eq},
  {.when = 'p',     .dest = RCPT_P,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_14[] = {
  {.when = 'T',     .dest = RCPT_T,      .act1 = may_eq},
  {.when = 't',     .dest = RCPT_T,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_15[] = {
  {.when = ' ',     .dest = RCPT_SPACE,      .act1 = may_eq},
  {.when = ' ',     .dest = RCPT_SPACE,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_16[] = {
  {.when = 'A',     .dest = DATA_A,      .act1 = may_eq},
  {.when = 'a',     .dest = DATA_A,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_17[] = {
  {.when = 'T',     .dest = DATA_T,      .act1 = may_eq},
  {.when = 't',     .dest = DATA_T,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_18[] = {
  {.when = 'A',     .dest = DATA_FINAL_A,      .act1 = may_eq},
  {.when = 'a',     .dest = DATA_FINAL_A,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_19[] = {
  /* TODO: define what to do when client writes DATA */
};

static const struct parser_state_transition ST_20[] = {
  {.when = 'E',     .dest = RSET_E,      .act1 = may_eq},
  {.when = 'e',     .dest = RSET_E,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_21[] = {
  {.when = 'T',     .dest = RSET_T,      .act1 = may_eq},
  {.when = 't',     .dest = RSET_T,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_22[] = {
  /* TODO: define what to do when client writes RSET*/
};

static const struct parser_state_transition ST_23[] = {
  {.when = 'O',     .dest = NOOP_O,      .act1 = may_eq},
  {.when = 'o',     .dest = NOOP_O,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_24[] = {
  {.when = 'O',     .dest = NOOP_SECOND_O,      .act1 = may_eq},
  {.when = 'o',     .dest = NOOP_SECOND_O,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_25[] = {
  {.when = 'P',     .dest = NOOP_P,      .act1 = may_eq},
  {.when = 'p',     .dest = NOOP_P,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_26[] = {
  /* TODO: define what to do when client writes NOOP */
};

static const struct parser_state_transition ST_27[] = {
  {.when = 'U',     .dest = QUIT_U,      .act1 = may_eq},
  {.when = 'u',     .dest = QUIT_U,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_28[] = {
  {.when = 'I',     .dest = QUIT_I,      .act1 = may_eq},
  {.when = 'i',     .dest = QUIT_I,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_29[] = {
  {.when = 'T',     .dest = QUIT_T,      .act1 = may_eq},
  {.when = 't',     .dest = QUIT_T,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_30[] = {
  /* TODO: define what to do when client writes QUIT */
};

static const struct parser_state_transition ST_31[] = {
  {.when = 'R',     .dest = VRFY_R,      .act1 = may_eq},
  {.when = 'r',     .dest = VRFY_R,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_32[] = {
  {.when = 'F',     .dest = VRFY_F,      .act1 = may_eq},
  {.when = 'f',     .dest = VRFY_F,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_33[] = {
  {.when = 'Y',     .dest = VRFY_Y,      .act1 = may_eq},
  {.when = 'y',     .dest = VRFY_Y,      .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_34[] = {
  /* TODO: define what to do when client writes VRFY */
};

static const struct parser_state_transition ST_35[] = {
  {.when = 'C',     .dest = RCPT_C,          .act1 = may_eq},
  {.when = 'c',     .dest = RCPT_C,          .act1 = may_eq},

  {.when = 'S',     .dest = RSET_S,          .act1 = may_eq},
  {.when = 's',     .dest = RSET_S,          .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_36[] = {
  /* TODO: define what to do when client writes HELO\s*/
};

static const struct parser_state_transition ST_37[] = {
  /* TODO: define what to do when client writes EHLO\s*/
};

static const struct parser_state_transition ST_38[] = {
  /* TODO: define what to do when client writes MAIL\s*/
};

static const struct parser_state_transition ST_39[] = {
  /* TODO: define what to do when client writes RCPT\s*/
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
    ST_9,
    ST_10,
    ST_11,
    ST_12,
    ST_13,
    ST_14,
    ST_15,
    ST_16,
    ST_17,
    ST_18,
    ST_19,
    ST_20,
    ST_21,
    ST_22,
    ST_23,
    ST_24,
    ST_25,
    ST_26,
    ST_27,
    ST_28,
    ST_29,
    ST_30,
    ST_31,
    ST_32,
    ST_33,
    ST_34,
    ST_35,
    ST_36,
    ST_37,
    ST_38,
    ST_39
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