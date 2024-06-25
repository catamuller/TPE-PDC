#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "headers/parser_utils.h"

const char *
parser_utils_strcmpi_event(const enum string_cmp_event_types type) {
    const char *ret;

    switch(type) {
        case STRING_CMP_MAYEQ:
            ret = "wait(c)";
            break;
        case STRING_CMP_EQ:
            ret = "eq(c)";
            break;
        case STRING_CMP_NEQ:
            ret = "neq(c)";
            break;
        default:
            ret = "not handled(c)";
            break;
    }
    return ret;
}

void may_eq(struct parser_event *ret, const uint8_t c) {
    ret->type    = STRING_CMP_MAYEQ;
    ret->n       = 1;
    ret->data[0] = c;
}

void eq(struct parser_event *ret, const uint8_t c) {
    ret->type    = STRING_CMP_EQ;
    ret->n       = 1;
    ret->data[0] = c;
}

static void type(enum string_cmp_event_types type, struct parser_event *ret, const uint8_t c) {
    ret->type    = type;
    ret->n       = 1;
    ret->data[0] = c;
}

void eqHELO(struct parser_event *ret, const uint8_t c) {
    type(HELO_CMP_EQ, ret, c);
}

void eqEHLO(struct parser_event *ret, const uint8_t c) {
    type(EHLO_CMP_EQ, ret, c);
}

void eqMAILFROM(struct parser_event *ret, const uint8_t c) {
    type(MAIL_FROM_CMP_EQ, ret, c);
}

void neqSTAT(struct parser_event *ret, const uint8_t c) {
    type(STAT_CMP_NEQ, ret, c);
}

void eqRCPT(struct parser_event *ret, const uint8_t c) {
    type(RCPT_TO_CMP_EQ, ret, c);
}

void eqQUIT(struct parser_event *ret, const uint8_t c) {
    type(QUIT_CMP_EQ, ret, c);
}

void eqDATA(struct parser_event *ret, const uint8_t c) {
    type(DATA_CMP_EQ, ret, c);
}

void eqPARSERRST(struct parser_event *ret, const uint8_t c) {
    type(PARSER_RESET_CMP_EQ, ret, c);
}

void eqCLIENTDATA(struct parser_event *ret, const uint8_t c) {
    type(CLIENT_DATA_CMP_EQ, ret, c);
}

void eqCURRENT(struct parser_event *ret, const uint8_t c) {
    type(CURRENT_CMP_EQ, ret, c);
}

void eqTOTAL(struct parser_event *ret, const uint8_t c) {
    type(TOTAL_CMP_EQ, ret, c);
}

void eqBYTES(struct parser_event *ret, const uint8_t c) {
    type(BYTES_CMP_EQ, ret, c);
}

void eqMAILS(struct parser_event *ret, const uint8_t c) {
    type(MAILS_CMP_EQ, ret, c);
}

void eqRSET(struct parser_event *ret, const uint8_t c) {
    type(RSET_CMP_EQ, ret, c);
}

void eqNOOP(struct parser_event *ret, const uint8_t c) {
    type(NOOP_CMP_EQ, ret, c);
}

void eqHELP(struct parser_event *ret, const uint8_t c) {
    type(HELP_CMP_EQ, ret, c);
}

void eqVRFY(struct parser_event *ret, const uint8_t c) {
    type(VRFY_CMP_EQ, ret, c);
}

void neqMAIL(struct parser_event *ret, const uint8_t c) {
    type(MAIL_CMP_NEQ, ret, c);
}

void neqHELO(struct parser_event *ret, const uint8_t c) {
    type(HELO_CMP_NEQ, ret, c);
}

void neqEHLO(struct parser_event *ret, const uint8_t c) {
    type(EHLO_CMP_NEQ, ret, c);
}

void neqRCPT(struct parser_event *ret, const uint8_t c) {
    type(RCPT_CMP_NEQ, ret, c);
}

void neqDomain(struct parser_event *ret, const uint8_t c) {
    type(NEQ_DOMAIN, ret, c);
}

void neqCMD(struct parser_event *ret, const uint8_t c) {
    type(CMD_CMP_NEQ, ret, c);
}

void USERSave(struct parser_event *ret, const uint8_t c) {
    type(USERSAVE_CMP_EQ, ret, c);
}

void RCPTTOSave(struct parser_event *ret, const uint8_t c) {
    type(RCPTTOSAVE_CMP_EQ, ret, c);
}

void MAILFROMSave(struct parser_event *ret, const uint8_t c) {
    type(MAILFROMSAVE_CMP_EQ, ret, c);
}

void CLIENTDATASave(struct parser_event *ret, const uint8_t c) {
    type(DATASAVE_CMP_EQ, ret, c);
}

void neq(struct parser_event *ret, const uint8_t c) {
    ret->type    = STRING_CMP_NEQ;
    ret->n       = 1;
    ret->data[0] = c;
}

/*
 * para comparar "foo" (length 3) necesitamos 3 + 2 estados.
 * Los útimos dos, son el sumidero de comparación fallida, y
 * el estado donde se llegó a la comparación completa.
 *
 * static const struct parser_state_transition ST_0 [] =  {
 *   {.when = 'F',        .dest = 1,         .action1 = may_eq, },
 *   {.when = 'f',        .dest = 1,         .action1 = may_eq, },
 *   {.when = ANY,        .dest = NEQ,       .action1 = neq,},
 * };
 * static const struct parser_state_transition ST_1 [] =  {
 *   {.when = 'O',        .dest = 2,         .action1 = may_eq, },
 *   {.when = 'o',        .dest = 2,         .action1 = may_eq, },
 *   {.when = ANY,        .dest = NEQ,       .action1 = neq,},
 * };
 * static const struct parser_state_transition ST_2 [] =  {
 *   {.when = 'O',        .dest = EQ,        .action1 = eq, },
 *   {.when = 'o',        .dest = EQ,        .action1 = eq, },
 *   {.when = ANY,        .dest = NEQ,       .action1 = neq,},
 * };
 * static const struct parser_state_transition ST_EQ  (3) [] =  {
 *   {.when = ANY,        .dest = NEQ,       .action1 = neq,},
 * };
 * static const struct parser_state_transition ST_NEQ (4) [] =  {
 *   {.when = ANY,        .dest = NEQ,       .action1 = neq,},
 * };
 *
 */
struct parser_definition
parser_utils_strcmpi(const char *s) {
    const size_t n = strlen(s);

    struct parser_state_transition **states   = calloc(n + 2, sizeof(*states));
    size_t *nstates                           = calloc(n + 2, sizeof(*nstates));
    struct parser_state_transition *transitions= calloc(3 *(n + 2),
                                                        sizeof(*transitions));
    if(states == NULL || nstates == NULL || transitions == NULL) {
        free(states);
        free(nstates);
        free(transitions);

        struct parser_definition def = {
            .start_state   = 0,
            .states_count  = 0,
            .states        = NULL,
            .states_n      = NULL,
        };
        return def;
    }

    // estados fijos
    const size_t st_eq  = n;
    const size_t st_neq = n + 1;

    for(size_t i = 0; i < n; i++) {
        const size_t dest = (i + 1 == n) ? st_eq : i + 1;

        transitions[i * 3 + 0].when = tolower(s[i]);
        transitions[i * 3 + 0].dest = dest;
        transitions[i * 3 + 0].act1 = i + 1 == n ? eq : may_eq;
        transitions[i * 3 + 1].when = toupper(s[i]);
        transitions[i * 3 + 1].dest = dest;
        transitions[i * 3 + 1].act1 = i + 1 == n ? eq : may_eq;
        transitions[i * 3 + 2].when = ANY;
        transitions[i * 3 + 2].dest = st_neq;
        transitions[i * 3 + 2].act1 = neq;
        states     [i]              = transitions + (i * 3 + 0);
        nstates    [i]              = 3;
    }
    // EQ
    transitions[(n + 0) * 3].when   = ANY;
    transitions[(n + 0) * 3].dest   = st_neq;
    transitions[(n + 0) * 3].act1   = neq;
    states     [(n + 0)]            = transitions + ((n + 0) * 3 + 0);
    nstates    [(n + 0)]            = 1;
    // NEQ
    transitions[(n + 1) * 3].when   = ANY;
    transitions[(n + 1) * 3].dest   = st_neq;
    transitions[(n + 1) * 3].act1   = neq;
    states     [(n + 1)]            = transitions + ((n + 1) * 3 + 0);
    nstates    [(n + 1)]            = 1;


    struct parser_definition def = {
        .start_state   = 0,
        .states_count  = n + 2,
        .states        = (const struct parser_state_transition **) states,
        .states_n      = (const size_t *) nstates,
    };

    return def;
}

void
parser_utils_strcmpi_destroy(const struct parser_definition *p) {
    free((void *)p->states[0]);
    free((void *)p->states);
    free((void *)p->states_n);
}
