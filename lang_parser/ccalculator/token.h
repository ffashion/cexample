#ifndef __TOKEN__H__
#define __TOKEN__H__

#include "list.h"
#include "mpool.h"
#include <stddef.h>
#include "type.h"

typedef struct str {
    size_t len;
    char *data;
}string_t;

#define string(str)     { sizeof(str) - 1, (char *) str }

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) ( \
    sizeof(arr) / sizeof((arr)[0]) \
)
#endif

typedef struct token Token;

typedef enum {
    TK_NUM,
    TK_PUNCT,
    TK_KEYWORD,
    TK_EOF,
}TokenKind;

struct token {
    TokenKind kind;
    

    Type *ty; //used for TK_NUM
    int64_t val;
    long double fval;

    const char *loc;
    const char *end;
    size_t len;

    struct list_head list;
};


Token *tokenize(char *p, mpool_t *pool);
Token *tokenize_file(char *file, mpool_t *pool);
#endif  /*__TOKEN__H__*/