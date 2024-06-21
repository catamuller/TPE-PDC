#include "headers/smtp_parsing.h"
#include <stdlib.h>
#include <stdio.h>

#define CR '\r'
#define LF '\n'

#define BUFFER_MAX_SIZE 1024

#define N(x) (sizeof(x)/sizeof((x)[0]))

char domain[BUFFER_MAX_SIZE] = "smtp.com";
struct parser_state_transition * domain_states[100] = {0};
static struct parser_state_transition STATE_DOMAIN_CR[] = {
      {.when = '\r',          .dest = 1,        .act1 = may_eq},

      {.when = ANY,           .dest = WRONG_DOMAIN,         .act1 = neq}
};

static struct parser_state_transition STATE_DOMAIN_LF[] = {
      {.when = '\n',          .dest = 1,        .act1 = eqMAILFROM},

      {.when = ANY,           .dest = WRONG_DOMAIN,         .act1 = neqDomain}
};

struct parser_state_transition STATE_ACCEPT[] = {
      {.when = ANY,           .dest = 1,            .act1 = eq}
};

void setCRDest(int index) {
  STATE_DOMAIN_CR[0].dest = index;
}

void setLFDest(int index) {
  STATE_DOMAIN_LF[0].dest = index;
}

void setAcceptDest(int index) {
  STATE_ACCEPT[0].dest = index;
}

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
  {.when = '\r',    .dest = DATA_CR_STATE,  .act1 = may_eq},
  {.when = ANY,     .dest = NEQ,         .act1 = neq}
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

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
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

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
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
  {.when = '\r',    .dest = QUIT_CR_STATE,  .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
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

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
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

  {.when = ANY,     .dest = HELO_DOMAIN_STATE,         .act1 = may_eq}
};

static const struct parser_state_transition ST_37[] = {
  /* TODO: define what to do when client writes EHLO\s*/

  {.when = ANY,     .dest = EHLO_DOMAIN_STATE,         .act1 = neq}
};

static const struct parser_state_transition ST_38[] = {
  /* TODO: define what to do when client writes MAIL\s*/
  {.when = 'F',     .dest = FROM_F,        .act1 = may_eq},
  {.when = 'f',     .dest = FROM_F,        .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_39[] = {
  /* TODO: define what to do when client writes RCPT\s*/
  {.when = 'T',     .dest = TO_T,          .act1 = may_eq},
  {.when = 't',     .dest = TO_T,          .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_40[] = {
  {.when = '\r',    .dest = HELO_CR_STATE,     .act1 = may_eq},

  {.when = ANY,     .dest = HELO_DOMAIN_STATE, .act1 = may_eq}
};

static const struct parser_state_transition ST_41[] = {
  {.when = '\n',    .dest = HELO_LF_STATE,     .act1 = eqHELO},

  {.when = ANY,     .dest = HELO_DOMAIN_STATE, .act1 = may_eq}
};

static const struct parser_state_transition ST_42[] = {
  
  {.when = ANY,     .dest = HELO_DOMAIN_STATE, .act1 = may_eq}
};

static const struct parser_state_transition ST_43[] = {
  {.when = '\r',    .dest = EHLO_CR_STATE,     .act1 = may_eq},

  {.when = ANY,     .dest = EHLO_DOMAIN_STATE, .act1 = may_eq}
};

static const struct parser_state_transition ST_44[] = {
  {.when = '\n',    .dest = EHLO_LF_STATE,     .act1 = eqEHLO},

  {.when = ANY,     .dest = EHLO_DOMAIN_STATE, .act1 = may_eq}
};

static const struct parser_state_transition ST_45[] = {
  
  {.when = ANY,     .dest = EHLO_DOMAIN_STATE, .act1 = may_eq}
};

static const struct parser_state_transition ST_46[] = {
  /* TODO: on error (unrecognized command - NEQ)*/
  {.when = ANY,     .dest = 0,         .act1 = neq}
};

static const struct parser_state_transition ST_47[] = {
  {.when = 'R',     .dest = FROM_R,        .act1 = may_eq},
  {.when = 'r',     .dest = FROM_R,        .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,           .act1 = neq}
};

static const struct parser_state_transition ST_48[] = {
  {.when = 'O',     .dest = FROM_O,        .act1 = may_eq},
  {.when = 'o',     .dest = FROM_O,        .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,           .act1 = neq}
};

static const struct parser_state_transition ST_49[] = {
  {.when = 'M',     .dest = FROM_M,        .act1 = may_eq},
  {.when = 'm',     .dest = FROM_M,        .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,           .act1 = neq}
};

static const struct parser_state_transition ST_50[] = {
  {.when = ':',     .dest = FROM_COLON,  .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};


static const struct parser_state_transition ST_51[] = {
  {.when = ' ',     .dest = FROM_SPACE,  .act1 = may_eq},
  {.when = ANY,     .dest = FROM_SPACE,  .act1 = may_eq} // TODO: save character in string with act2
};

static const struct parser_state_transition ST_52[] = {
  {.when = '@',     .dest = MAX_STATES-100,      .act1 = may_eq},
  {.when = ANY,     .dest = FROM_SPACE,          .act1 = may_eq} // TODO: save character in string with act2
};

static const struct parser_state_transition ST_53[] = {
  /* TODO: handle for invalid domain in MAIL FROM */
  {.when = ANY,     .dest = WRONG_DOMAIN,          .act1 = neqDomain} 
};

static const struct parser_state_transition ST_54[] = {
  {.when = 'O',     .dest = TO_O,        .act1 = may_eq},
  {.when = 'o',     .dest = TO_O,        .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,           .act1 = neq}
};

static const struct parser_state_transition ST_55[] = {
  {.when = ':',     .dest = TO_COLON,  .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_56[] = {
  {.when = ' ',     .dest = TO_SPACE,  .act1 = may_eq},
  {.when = ANY,     .dest = TO_SPACE,  .act1 = may_eq} // TODO: save character in string with act2
};

static const struct parser_state_transition ST_57[] = {
  {.when = '\r',    .dest = RCPT_TO_CR_STATE, .act1 = may_eq},
  {.when = ANY,     .dest = TO_SPACE,   .act1 = neq} // TODO: save character in string with act2
};

static const struct parser_state_transition ST_58[] = {
  {.when = '\n',    .dest = RCPT_TO_LF_STATE, .act1 = eqRCPT},
  {.when = ANY,     .dest = TO_SPACE,   .act1 = neq} // TODO: save character in string with act2
};

static const struct parser_state_transition ST_59[] = {
  {.when = ANY,     .dest = RCPT_TO_LF_STATE,  .act1 = eqRCPT}
};

static const struct parser_state_transition ST_60[] = {
  /* TODO: define what to do when client writes QUIT */
  {.when = '\n',    .dest = QUIT_LF_STATE,  .act1 = eqQUIT},
  
  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};

static const struct parser_state_transition ST_61[] = {
  
  {.when = ANY,     .dest = NEQ,         .act1 = eqQUIT}
};

static const struct parser_state_transition ST_62[] = {
  {.when = '\n',    .dest = DATA_LF_STATE,     .act1 = eqDATA},

  {.when = ANY,     .dest = EHLO_DOMAIN_STATE, .act1 = may_eq}
};

static const struct parser_state_transition ST_63[] = {
  
  {.when = ANY,     .dest = NEQ,         .act1 = eqDATA} // TODO: save data with act2
};


size_t states_amount[MAX_STATES] = {
    N(ST_0),
    N(ST_1),
    N(ST_2),
    N(ST_3),
    N(ST_4),
    N(ST_5),
    N(ST_6),
    N(ST_7),
    N(ST_8),
    N(ST_9),
    N(ST_10),
    N(ST_11),
    N(ST_12),
    N(ST_13),
    N(ST_14),
    N(ST_15),
    N(ST_16),
    N(ST_17),
    N(ST_18),
    N(ST_19),
    N(ST_20),
    N(ST_21),
    N(ST_22),
    N(ST_23),
    N(ST_24),
    N(ST_25),
    N(ST_26),
    N(ST_27),
    N(ST_28),
    N(ST_29),
    N(ST_30),
    N(ST_31),
    N(ST_32),
    N(ST_33),
    N(ST_34),
    N(ST_35),
    N(ST_36),
    N(ST_37),
    N(ST_38),
    N(ST_39),
    N(ST_40),
    N(ST_41),
    N(ST_42),
    N(ST_43),
    N(ST_44),
    N(ST_45),
    N(ST_46),
    N(ST_47),
    N(ST_48),
    N(ST_49),
    N(ST_50),
    N(ST_51),
    N(ST_52),
    N(ST_53),
    N(ST_54),
    N(ST_55),
    N(ST_56),
    N(ST_57),
    N(ST_58),
    N(ST_59),
    N(ST_60),
    N(ST_61),
    N(ST_62),
    N(ST_63)
};

const struct parser_state_transition * states[MAX_STATES] = {
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
    ST_39,
    ST_40,
    ST_41,
    ST_42,
    ST_43,
    ST_44,
    ST_45,
    ST_46,
    ST_47,
    ST_48,
    ST_49,
    ST_50,
    ST_51,
    ST_52,
    ST_53,
    ST_54,
    ST_55,
    ST_56,
    ST_57,
    ST_58,
    ST_59,
    ST_60,
    ST_61,
    ST_62,
    ST_63
  };


struct parser * smtp_parser_init() {
  struct parser_definition * def = calloc(1, sizeof(*def));

  FILE * domainFile = fopen("../domain.txt","rb");
  if (domainFile == NULL)
    domainFile = fopen("./domain.txt","rb");
  if (domainFile != NULL) {
    fread(domain, sizeof(char), BUFFER_MAX_SIZE, domainFile);
  }
  int i = MAX_STATES - 100;
  for(int j = 0;domain[j];j++, i++) {
    domain_states[j] = calloc(2, sizeof(struct parser_state_transition));
    domain_states[j][0].when = domain[j]; 
    domain_states[j][0].dest = i+1; 
    domain_states[j][0].act1 = may_eq; 

    domain_states[j][1].when = ANY; 
    domain_states[j][1].dest = WRONG_DOMAIN; 
    domain_states[j][1].act1 = neqDomain; 
    states[i] = domain_states[j];
    states_amount[i] = 2;
  }
  setCRDest(i+1);
  states[i] = STATE_DOMAIN_CR;
  states_amount[i++] = 2;
  setLFDest(i+1);
  states[i] = STATE_DOMAIN_LF;
  states_amount[i++] = 2;
  setAcceptDest(i);
  states[i] = STATE_ACCEPT;
  states_amount[i] = 1;

  def->states = states;
  def->states_n = states_amount;
  
  return parser_init(parser_no_classes(), def);
}

const struct parser_event * smtp_parser_feed(struct parser * p, const uint8_t c) {
  return parser_feed(p, c);
}

const struct parser_event * smtp_parser_consume(buffer * buff, struct parser * p) {
  const struct parser_event * e1;
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