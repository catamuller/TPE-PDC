#include "headers/smtp_parsing.h"
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <pthread.h>
#include "headers/smtp.h"
#include "headers/logger.h"

#define CR '\r'
#define LF '\n'
#define BUFFER_MAX_SIZE 1024
#define ID_LEN 33

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

void _randomID(char* buffer) {
    char charSet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890";
    int setSize = strlen(charSet);

    int idx;
    int i=0;
    for(; i<ID_LEN - 1; i++) {
        idx = rand() % setSize;
        buffer[i] = charSet[idx];
    }
    buffer[i] = '\0';
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

  {.when = 'X',     .dest = XSTAT_X,      .act1 = may_eq},
  {.when = 'x',     .dest = XSTAT_X,      .act1 = may_eq},

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

  {.when = '\r', .dest = RSET_CR_STATE, .act1 = may_eq},

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
  {.when = '\r',     .dest = NOOP_CR_STATE,      .act1 = may_eq},
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
  {.when = ANY,     .dest = HELO_DOMAIN_STATE,         .act1 = USERSave}
};

static const struct parser_state_transition ST_37[] = {
  /* TODO: define what to do when client writes EHLO\s*/

  {.when = ANY,     .dest = EHLO_DOMAIN_STATE,         .act1 = USERSave}
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

  {.when = ANY,     .dest = HELO_DOMAIN_STATE, .act1 = USERSave}
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

  {.when = ANY,     .dest = EHLO_DOMAIN_STATE, .act1 = USERSave}
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
  {.when = '\r',    .dest = PARSER_RESET_CR_STATE,    .act1 = may_eq},
  {.when = ANY,     .dest = NEQ,         .act1 = neq}
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
  {.when = '@',     .dest = MAX_STATES-100,      .act1 =  MAILFROMSave},
  {.when = '\r',    .dest = WRONG_DOMAIN,          .act1 = neqDomain},
  {.when = ANY,     .dest = FROM_SPACE,          .act1 = MAILFROMSave}
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
  {.when = ' ',     .dest = TO_COLON,  .act1 = may_eq},
  {.when = '@',     .dest = NEQ,       .act1 = neq},
  {.when = ANY,     .dest = TO_SPACE,  .act1 = RCPTTOSave} // TODO: save character in string with act2
};

static const struct parser_state_transition ST_57[] = {
  {.when = '\r',    .dest = RCPT_TO_CR_STATE, .act1 = may_eq},
  {.when = ' ',     .dest = NEQ,              .act1 = neq},
  {.when = ANY,     .dest = TO_SPACE,   .act1 = RCPTTOSave}
};

static const struct parser_state_transition ST_58[] = {
  {.when = '\n',    .dest = RCPT_TO_LF_STATE, .act1 = eqRCPT},
  {.when = ANY,     .dest = TO_SPACE,   .act1 = eqRCPT} 
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
  
  {.when = ANY,     .dest = QUIT_LF_STATE,     .act1 = eqQUIT}
};

static const struct parser_state_transition ST_62[] = {
  {.when = '\n',    .dest = DATA_LF_STATE,     .act1 = eqDATA},

  {.when = ANY,     .dest = EHLO_DOMAIN_STATE, .act1 = may_eq}
};

static const struct parser_state_transition ST_63[] = {
  
  {.when = ANY,     .dest = DATA_LF_STATE,         .act1 = eqDATA} // TODO: save data with act2
};

static const struct parser_state_transition ST_64[] = {
  /* TODO: on error (unrecognized command - NEQ)*/
  {.when = '\n',    .dest = PARSER_RESET_LF_STATE,    .act1 = eqPARSERRST},
  {.when = ANY,     .dest = NEQ,         .act1 = neq}
};


static const struct parser_state_transition ST_65[] = {
  
  {.when = ANY,     .dest = PARSER_RESET_LF_STATE,         .act1 = eqPARSERRST} // TODO: save data with act2
};

static const struct parser_state_transition ST_66[] = {
  {.when = 'T',     .dest = STAT_T,       .act1 = may_eq},
  {.when = 't',     .dest = STAT_T,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_67[] = {
  {.when = 'A',     .dest = STAT_A,       .act1 = may_eq},
  {.when = 'a',     .dest = STAT_A,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_68[] = {
  {.when = 'T',     .dest = STAT_SECOND_T,       .act1 = may_eq},
  {.when = 't',     .dest = STAT_SECOND_T,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_69[] = {
  {.when = ' ',     .dest = STAT_SPACE,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_70[] = {
  {.when = 'C',     .dest = CURRENT_C,       .act1 = may_eq},
  {.when = 'c',     .dest = CURRENT_C,       .act1 = may_eq},

  {.when = 'T',     .dest = TOTAL_T,       .act1 = may_eq},
  {.when = 't',     .dest = TOTAL_T,       .act1 = may_eq},

  {.when = 'B',     .dest = BYTES_B,       .act1 = may_eq},
  {.when = 'b',     .dest = BYTES_B,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_71[] = {
  {.when = 'U',     .dest = CURRENT_U,       .act1 = may_eq},
  {.when = 'u',     .dest = CURRENT_U,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_72[] = {
  {.when = 'R',     .dest = CURRENT_R,       .act1 = may_eq},
  {.when = 'r',     .dest = CURRENT_R,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_73[] = {
  {.when = 'R',     .dest = CURRENT_SECOND_R,       .act1 = may_eq},
  {.when = 'r',     .dest = CURRENT_SECOND_R,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_74[] = {
  {.when = 'E',     .dest = CURRENT_E,       .act1 = may_eq},
  {.when = 'e',     .dest = CURRENT_E,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_75[] = {
  {.when = 'N',     .dest = CURRENT_N,       .act1 = may_eq},
  {.when = 'n',     .dest = CURRENT_N,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_76[] = {
  {.when = 'T',     .dest = CURRENT_T,       .act1 = may_eq},
  {.when = 't',     .dest = CURRENT_T,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_77[] = {
  {.when = '\r',     .dest = CURRENT_CR_STATE,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_78[] = {
  {.when = 'O',     .dest = TOTAL_O,       .act1 = may_eq},
  {.when = 'o',     .dest = TOTAL_O,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_79[] = {
  {.when = 'T',     .dest = TOTAL_SECOND_T,       .act1 = may_eq},
  {.when = 't',     .dest = TOTAL_SECOND_T,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_80[] = {
  {.when = 'A',     .dest = TOTAL_A,       .act1 = may_eq},
  {.when = 'a',     .dest = TOTAL_A,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_81[] = {
  {.when = 'L',     .dest = TOTAL_L,       .act1 = may_eq},
  {.when = 'l',     .dest = TOTAL_L,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_82[] = {
  {.when = '\r',     .dest = TOTAL_CR_STATE,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_83[] = {
  {.when = 'Y',     .dest = BYTES_Y,       .act1 = may_eq},
  {.when = 'y',     .dest = BYTES_Y,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_84[] = {
  {.when = 'T',     .dest = BYTES_T,       .act1 = may_eq},
  {.when = 't',     .dest = BYTES_T,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_85[] = {
  {.when = 'E',     .dest = BYTES_E,       .act1 = may_eq},
  {.when = 'e',     .dest = BYTES_E,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_86[] = {
  {.when = 'S',     .dest = BYTES_S,       .act1 = may_eq},
  {.when = 's',     .dest = BYTES_S,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_87[] = {
  {.when = '\r',     .dest = BYTES_CR_STATE,       .act1 = may_eq},

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_88[] = {
  {.when = '\n',     .dest = CURRENT_LF_STATE,       .act1 = eqCURRENT}, 

  {.when = ANY,     .dest = NEQ,          .act1 = neq} 
};

static const struct parser_state_transition ST_89[] = {

  {.when = ANY,     .dest = CURRENT_LF_STATE,          .act1 = eqCURRENT} 
};

static const struct parser_state_transition ST_90[] = {
  {.when = '\n',     .dest = TOTAL_LF_STATE,       .act1 = eqTOTAL}, 

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_91[] = {

  {.when = ANY,     .dest = TOTAL_LF_STATE,          .act1 = eqTOTAL}
};

static const struct parser_state_transition ST_92[] = {
  {.when = '\n',     .dest = BYTES_LF_STATE,       .act1 = eqBYTES}, 

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_93[] = {

  {.when = ANY,     .dest = BYTES_LF_STATE,          .act1 = eqBYTES}
};

static const struct parser_state_transition ST_94[] = {
  {.when = '\n',     .dest = NOOP_LF_STATE,       .act1 = eqNOOP},

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_95[] = {

  {.when = ANY,     .dest = NOOP_LF_STATE,          .act1 = eqNOOP}
};

static const struct parser_state_transition ST_96[] = {
  {.when = '\n',     .dest = RSET_LF_STATE,  .act1 = eqRSET},

  {.when = ANY,     .dest = NEQ,          .act1 = neq}
};

static const struct parser_state_transition ST_97[] = {

  {.when = ANY,     .dest = RSET_LF_STATE,  .act1 = eqRSET}
};

static const struct parser_state_transition ST_98[] = {
  {.when = 'S',     .dest = STAT_S,         .act1 = may_eq},
  {.when = 's',     .dest = STAT_S,         .act1 = may_eq},

  {.when = ANY,     .dest = CMD_NEQ,          .act1 = neqCMD}

};

static const struct parser_state_transition ST_99[] = {

  {.when = ANY,     .dest = CMD_NEQ,          .act1 = neqCMD}

};

static const struct parser_state_transition DT_0[] = {
  {.when = '\r',    .dest = DATA_FIRST_CR_STATE, .act1 = CLIENTDATASave},
  {.when = ANY,     .dest = DATA_ANY,     .act1 = CLIENTDATASave}
};

static const struct parser_state_transition DT_1[] = {
  {.when = '\n',    .dest = DATA_FIRST_LF_STATE, .act1 = CLIENTDATASave},
  {.when = ANY,     .dest = DATA_ANY,     .act1 = CLIENTDATASave}
};

static const struct parser_state_transition DT_2[] = {
  {.when = '.',    .dest = DATA_DOT, .act1 = CLIENTDATASave},
  {.when = ANY,     .dest = DATA_ANY,     .act1 = CLIENTDATASave}
};

static const struct parser_state_transition DT_3[] = {
  {.when = '\r',    .dest = DATA_SECOND_CR_STATE, .act1 = CLIENTDATASave},
  {.when = ANY,     .dest = DATA_ANY,     .act1 = CLIENTDATASave}
};

static const struct parser_state_transition DT_4[] = {
  {.when = '\n',    .dest = DATA_SECOND_LF_STATE, .act1 = eqCLIENTDATA},
  {.when = ANY,     .dest = DATA_ANY,     .act1 = CLIENTDATASave}
};

static const struct parser_state_transition DT_5[] = {
  {.when = ANY,     .dest = DATA_SECOND_LF_STATE,     .act1 = eqCLIENTDATA}
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
    N(ST_63),
    N(ST_64),
    N(ST_65),
    N(ST_66),
    N(ST_67),
    N(ST_68),
    N(ST_69),
    N(ST_70),
    N(ST_71),
    N(ST_72),
    N(ST_73),
    N(ST_74),
    N(ST_75),
    N(ST_76),
    N(ST_77),
    N(ST_78),
    N(ST_79),
    N(ST_80),
    N(ST_81),
    N(ST_82),
    N(ST_83),
    N(ST_84),
    N(ST_85),
    N(ST_86),
    N(ST_87),
    N(ST_88),
    N(ST_89),
    N(ST_90),
    N(ST_91),
    N(ST_92),
    N(ST_93),
    N(ST_94),
    N(ST_95),
    N(ST_96),
    N(ST_97),
    N(ST_98),
    N(ST_99)
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
    ST_63,
    ST_64,
    ST_65,
    ST_66,
    ST_67,
    ST_68,
    ST_69,
    ST_70,
    ST_71,
    ST_72,
    ST_73,
    ST_74,
    ST_75,
    ST_76,
    ST_77,
    ST_78,
    ST_79,
    ST_80,
    ST_81,
    ST_82,
    ST_83,
    ST_84,
    ST_85,
    ST_86,
    ST_87,
    ST_88,
    ST_89,
    ST_90,
    ST_91,
    ST_92,
    ST_93,
    ST_94,
    ST_95,
    ST_96,
    ST_97,
    ST_98,
    ST_99
  };

const struct parser_state_transition * data_states[MAX_STATES] = {
  DT_0,
  DT_1,
  DT_2,
  DT_3,
  DT_4,
  DT_5
};

size_t data_states_amount[MAX_STATES] = {
  N(DT_0),
  N(DT_1),
  N(DT_2),
  N(DT_3),
  N(DT_4),
  N(DT_5)
};

struct parser * smtp_parser_init(client_state* state) {
  struct parser_definition * def = calloc(1, sizeof(*def));

  for(int i=0;i<MAX_RCPT_TO;i++) {
    for(int j=0;j<BUFFER_MAX_SIZE;j++) {
      state->rcptTo[i][j] = 0;
    }
  }
  for (int j=0;j<BUFFER_MAX_SIZE;j++) {
    state->mailFrom[j] = 0;
  }

  FILE * domainFile = fopen("../domain.txt","rb");
  
  if (domainFile == NULL)
    domainFile = fopen("./domain.txt","rb");
  if (domainFile != NULL) {
    domain[fread(domain, sizeof(char), BUFFER_MAX_SIZE, domainFile)] = '\0';
  }
  fclose(domainFile);
  int i = MAX_STATES - 100;
  for(int j = 0;domain[j];j++, i++) {
    domain_states[j] = calloc(2, sizeof(struct parser_state_transition));
    domain_states[j][0].when = domain[j]; 
    domain_states[j][0].dest = i+1; 
    domain_states[j][0].act1 = MAILFROMSave; 

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

struct parser * smtp_data_parser_init(client_state * state) {
  for (int j=0;j<4*BUFFER_MAX_SIZE;j++) {
    state->data[j] = 0;
  }

  struct parser_definition * def = calloc(1, sizeof(*def));

  def->states = data_states;
  def->states_n = data_states_amount;
  
  return parser_init(parser_no_classes(), def);
}

const struct parser_event * smtp_parser_feed(struct parser * p, const uint8_t c) {
  return parser_feed(p, c);
}

const struct parser_event * smtp_parser_consume(buffer * buff, struct parser * p, client_state * state) {
  const struct parser_event * e1;
  bool CRflag = false;


  while(buffer_can_read(buff)) {
    const uint8_t c = buffer_read(buff);
    e1 = smtp_parser_feed(p, c);
    switch(e1->type) {
      case HELO_CMP_EQ:
        state->user[state->userIndex] = '\0';
        // smtp->user[smtp->userIndex] = '\0';
        // user[userIndex] = '\0';
        break;
      case RCPTTOSAVE_CMP_EQ:
        state->rcptTo[state->clientRcptToIndex][state->rcptToIndex++]=e1->data[0];
        //rcptTo[clientRcptToIndex][rcptToIndex++] = e1->data[0];
        break;
      case RCPT_TO_CMP_EQ:
        state->rcptTo[state->clientRcptToIndex++][state->rcptToIndex]='\0';
        //rcptTo[clientRcptToIndex++][rcptToIndex] = '\0';
        state->rcptToIndex = 0;
        break;
      case MAILFROMSAVE_CMP_EQ:
        state->mailFrom[state->mailFromIndex++]=e1->data[0];
        //mailFrom[mailFromIndex++] = e1->data[0];
        break;
      case MAIL_FROM_CMP_EQ:
        state->mailFrom[state->mailFromIndex]='\0';
        //mailFrom[mailFromIndex] = '\0';
        if (state->currentState <= SERVER_MAIL_FROM)
          state->clientRcptToIndex = 0;
        state->rcptToIndex = 0;
        state->mailFromIndex = 0;
        break;
      case STRING_CMP_NEQ:
        if (state->currentState <= SERVER_MAIL_FROM)
          state->clientRcptToIndex = 0;
        //rcptToIndex = 0;
        state->rcptToIndex = 0;
        state->mailFromIndex = 0;
        //mailFromIndex = 0;
        break;
      case NEQ_DOMAIN:
        state->mailFromIndex = 0;
        //mailFromIndex = 0;
        break;
      case USERSAVE_CMP_EQ:
        state->user[state->userIndex++] = e1->data[0];
        //user[userIndex++] = e1->data[0];
        break;
      default: break;
    }

    if (CRflag && c == LF)
      break;
    if (c == CR)
      CRflag = true;
  }
  return e1;
}

const struct parser_event * smtp_data_parser_consume(buffer * buff, struct parser * p, client_state * state, size_t * data_size) {
  const struct parser_event * e1;
  bool CRflag = false;
  /*
  1- read first \r\n
  2- read .
  */
  int exitStage = 0;

  while(buffer_can_read(buff)) {
    const uint8_t c = buffer_read(buff);
    e1 = smtp_parser_feed(p, c);

    if (e1->type == DATASAVE_CMP_EQ) {
      state->data[state->dataIndex++] = e1->data[0];
    } else if (e1->type == CLIENT_DATA_CMP_EQ) {
        state->rcptToTotal = state->clientRcptToIndex;
        state->rcptToIndex = 0;
        state->mailFromIndex = 0;
    }

    (*data_size)++;

    if (c == CR) {
      CRflag = true;
      continue;
    }
    if (CRflag && c == LF) {
      //primer CRLF
      if(exitStage == 0){
        CRflag = false;
        exitStage++;
        continue;
      }
      //segundo CRLF
      else if(exitStage == 2){
        if (state->dataIndex >= 2)
          state->data[state->dataIndex-2] = '\0';
        break;
      }
    }
    if (c == '.' && exitStage == 1) {
      exitStage++;
      continue;
    }
    //si es cualquier otra cosa
    exitStage = 0;
    CRflag = false;

  }
  return e1;
}

void nextComputation(char *query, int *next) {
    int len = strlen(query);

    int border=0;  // Length of the current border

    int rec=1;
    while(rec < len){
        if(query[rec]!=query[border]){
            if(border!=0)
                border=next[border-1];
            else
                next[rec++]=0;
        }
        else{
            border++;
            next[rec]=border;
            rec++;
        }
    }
}
 
int KMPSearch(char *query, char *target) {

    int qlen = strlen(query);
    int tlen = strlen(target);
    int *next = (int *) malloc(sizeof(int) * qlen);

    nextComputation(query, next);
    int pquery=0, rec=0;
    while(rec<tlen && pquery<qlen){
        if(target[rec] == query[pquery]){
            rec++;
            pquery++;
        }
        else {
            if (pquery==0){
                rec++;
            }
            else {
                pquery = next[pquery-1];
            }
        }
    }
    if (pquery == qlen){
        return rec-pquery;
    }

    return -1;

}



void parseSubject(client_state * state) {
  strcpy(state->subject, "EMPTY");
  int index = KMPSearch("Subject:", state->data);
  if (index == -1)
    return;
  int i = 0;
  index += strlen("Subject:");
  while(state->data[index] == ' ') index++;
  for(;state->data[index] != '\n';index++, i++) {
    state->subject[i] = state->data[index];
  }
  if (state->subject[i-1] == '\r')
    state->subject[i-1] = '\0';
  state->subject[i] = '\0';
}

void parseRcptTo(char rcptto[][BUFFER_MAX_SIZE],size_t index,client_state* state) {
  size_t i = 0;
  for(; rcptto[index][i] != '@' && rcptto[index][i]; i++) {
    state->shortRcptTo[index][i] = rcptto[index][i];
  }
  state->shortRcptTo[index][i] = '\0';
}

void sendMail(client_state * state) {
  struct stat st = {0};
  parseSubject(state);
  for(size_t i=0;i<state->rcptToTotal;i++) {
    if (state->rcptTo[i][0] == '\0')
      continue;
    parseRcptTo(state->rcptTo, i,state);
    char dirname[4*BUFFER_MAX_SIZE] = "mail_dir/";
    char * dirnameWithName = strcat(dirname, state->shortRcptTo[i]);
    if (stat(dirname, &st) == -1) {
      if (mkdir(dirname, 0700) == -1) {// if directory does not exist
        log_data("Could not create mail directory for %s. Already exists.", dirname);
        continue;     
      }
    }
      log_data("Created the mail directory for %s.", dirname);
    
    dirnameWithName = strcat(dirnameWithName, "/");
    srand(time(NULL));
    _randomID(state->randomId);
    char * mailPlusSubject = strcat(dirnameWithName, state->subject);
    mailPlusSubject = strcat(mailPlusSubject, "_");
    char * subjectPlusRandomId = strcat(mailPlusSubject, state->randomId);
    FILE * mail = fopen(subjectPlusRandomId, "wa");
    if (mail == NULL) {
      log_data("Could not create the mail file for %s.", dirname);
      continue;
    }
      log_data("Created the mail file for %s.", dirname);
    

    char buffer[4*BUFFER_MAX_SIZE] = {0};
    for(size_t k=0; k<state->rcptToTotal;k++) {
      strcat(buffer, "RCPT TO: ");
      strcat(buffer, state->rcptTo[k]);
      strcat(buffer, "\n");
    }
    fprintf(mail, "MAIL FROM: %s\n%s\n%s\n", state->mailFrom, buffer, state->data);
    fclose(mail);
  }
  state->rcptToTotal = 0;
  state->clientRcptToIndex = 0;
  //free(mailPlusSubject);
  //free(subjectPlusRandomId);
}



