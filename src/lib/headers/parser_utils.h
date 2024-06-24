#ifndef PARSER_UTILS_H_c2f29bb6482d34fc6f94a09046bbd65a5f668acf
#define PARSER_UTILS_H_c2f29bb6482d34fc6f94a09046bbd65a5f668acf

/*
 * parser_utils.c -- factory de ciertos parsers típicos
 *
 * Provee parsers reusables, como por ejemplo para verificar que
 * un string es igual a otro de forma case insensitive.
 */
#include "parser.h"

enum string_cmp_event_types {
    STRING_CMP_MAYEQ,
    /** hay posibilidades de que el string sea igual */
    STRING_CMP_EQ,
    /** NO hay posibilidades de que el string sea igual */
    STRING_CMP_NEQ,

    HELO_CMP_EQ,
    EHLO_CMP_EQ,
    MAIL_FROM_CMP_EQ,
    RCPT_TO_CMP_EQ,
    DATA_CMP_EQ,
    CLIENT_DATA_CMP_EQ,
    QUIT_CMP_EQ,
    NEQ_DOMAIN,
    PARSER_RESET_CMP_EQ,
    USERSAVE_CMP_EQ,
    CURRENT_CMP_EQ,
    TOTAL_CMP_EQ,
    BYTES_CMP_EQ,
    RSET_CMP_EQ,
    NOOP_CMP_EQ,
    RCPTTOSAVE_CMP_EQ,
    MAILFROMSAVE_CMP_EQ,
    DATASAVE_CMP_EQ
};

void may_eq(struct parser_event *ret, const uint8_t c);


void eq(struct parser_event *ret, const uint8_t c);

static void type(enum string_cmp_event_types type, struct parser_event *ret, const uint8_t c);

void eqHELO(struct parser_event *ret, const uint8_t c);

void eqEHLO(struct parser_event *ret, const uint8_t c);

void eqMAILFROM(struct parser_event *ret, const uint8_t c);

void eqRCPT(struct parser_event *ret, const uint8_t c);

void eqQUIT(struct parser_event *ret, const uint8_t c);

void eqDATA(struct parser_event *ret, const uint8_t c);

void eqPARSERRST(struct parser_event *ret, const uint8_t c);

void eqCLIENTDATA(struct parser_event *ret, const uint8_t c);

void eqCURRENT(struct parser_event *ret, const uint8_t c);

void eqTOTAL(struct parser_event *ret, const uint8_t c);

void eqBYTES(struct parser_event *ret, const uint8_t c);

void eqNOOP(struct parser_event *ret, const uint8_t c);

void eqRSET(struct parser_event *ret, const uint8_t c);

void USERSave(struct parser_event *ret, const uint8_t c);

void RCPTTOSave(struct parser_event *ret, const uint8_t c);

void MAILFROMSave(struct parser_event *ret, const uint8_t c);

void CLIENTDATASave(struct parser_event *ret, const uint8_t c);

void neq(struct parser_event *ret, const uint8_t c);

void neqDomain(struct parser_event *ret, const uint8_t c);
const char *
parser_utils_strcmpi_event(const enum string_cmp_event_types type);


/*
 * Crea un parser que verifica que los caracteres recibidos forment el texto
 * descripto por `s'.
 *
 * Si se recibe el evento `STRING_CMP_NEQ' el texto entrado no matchea.
 */
struct parser_definition
parser_utils_strcmpi(const char *s);

/**
 * libera recursos asociado a una llamada de `parser_utils_strcmpi'
 */
void
parser_utils_strcmpi_destroy(const struct parser_definition *p);

#endif
