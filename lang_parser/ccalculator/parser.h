#ifndef __PARSER__H__
#define __PARSER__H__

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "token.h"
#include "type.h"
#include <assert.h>
typedef struct ast_node_s Node;

typedef enum {
    ND_NUM,
    ND_NEG,       // unary -
    ND_NOT,       // unary !
    ND_EQ,        // ==
    ND_NE,        // !=
    ND_LT,        // <
    ND_LE,        // <=
    ND_SHL,       // <<
    ND_SHR,       // >>
    ND_BITNOT,     // unary ~
    ND_MUL,       // *
    ND_DIV,       // /
    ND_MOD,       // %
    ND_ADD,       // +
    ND_SUB,       // -
    ND_BITAND,    // &
    ND_BITXOR,    // ^
    ND_BITOR,     // |
    ND_LOGAND,    // &&
    ND_LOGOR,     // ||
    ND_COMMA,     // ,
    ND_COND,      // ?:
}NodeKind;

struct ast_node_s {
    NodeKind kind;
    union {
        struct {
            Node *lhs;
            Node *rhs;
        };
        struct {
            Node *cond;
            Node *then;
            Node *els;
        };
    };

    /*No use now*/
    Type *ty;

    int64_t val;
    long double fval;
};

Node* parser(Token *tok, mpool_t *pool);

int64_t compute(Node *node);

void error_log(char *fmt, ...);

#endif  /*__PARSER__H__*/
