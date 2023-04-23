#ifndef __TOKEN__H__
#define __TOKEN__H__

#include "list.h"
#include "mpool.h"
#include <stddef.h>

typedef struct token Token;

typedef enum tokentype{
    TK_NUM,
    TK_NUM_FLOAT,
    TK_PUNCT,
    TK_KEYWORD,
    TK_EOF,
}tokentype_t;

struct token {
    tokentype_t kind;
    struct list_head list;
    char *loc;
    char *end;
    size_t len;
    int val;
    long double fval;
};



Token *tokenize(char *p, mpool_t *pool);
Token *tokenize_file(char *file, mpool_t *pool);
#endif  /*__TOKEN__H__*/