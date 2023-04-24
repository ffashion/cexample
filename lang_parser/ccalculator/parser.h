#ifndef __PARSER__H__
#define __PARSER__H__

#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "token.h"
typedef struct ast_node_s Node;

typedef enum {
    ND_ADD,       // +
    ND_SUB,       // -
    ND_MUL,       // *
    ND_DIV,       // /
    ND_COND,      // ?:
    ND_LOGOR,
    ND_LOGAND,
    ND_NUM,
}NodeKind;

struct ast_node_s {
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    long val;
};

Node* parser(Token *tok, mpool_t *pool);

int compute(Node *node);

#endif  /*__PARSER__H__*/