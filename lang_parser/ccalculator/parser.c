#include "parser.h"
#include "list.h"
#include "mpool.h"
#include "token.h"
#include <stdio.h>
#include <stdlib.h>

static Node *primary(Token **rest, Token *tok, mpool_t *pool);
static Node *mul(Token **rest, Token *tok, mpool_t *pool);
static Node *expr(Token **rest, Token *tok, mpool_t *pool);


/*
    + - * / ! ~ 
    
    unary 
    - +
    
    >= > <= <
    
    || &&
    
    | & ^

    shift
    << >>


    // do we need this?
    +, +=
    ++, --
    ? :
*/
/*

stmt  = ...| ident ":" stmt｜  "{" compound-stmt | expr-stmt

compound-stmt = (typedef | declaration | stmt)* "}"

expr-stmt = expr? ";"

expr = assign ("," expr)?

assign = conditional (assign-op assign)?

assign-op = "=" | "+=" ....

conditional = logor ("?" expr? ":" conditional)?

logor = logand ("||" logand)*

logand = bitor ("&&" bitor)*

bitor = bitxor ("|" bitxor)*

bitxor = bitand ("^" bitand)*

bitand = equality ("&" equality)*

equality = relational ("==" relational | "!=" relational)*

relational = shift ("<" shift | "<=" shift | ">" shift | ">=" shift)*

 shift = add ("<<" add | ">>" add)*

add = mul ("+" mul | "-" mul)*

mul = cast ("\*" cast | "/" cast | "%" cast)*

cast = "(" type-name ")" cast | unary

unary = ("+" | "-" | "*" | "&" | "!" | "~") cast | ("++" | "--") unary | "&&" ident | postfix

postfix  = "(" type-name ")" "{" initializer-list "}"

​		 	=  ident "(" func-args ")" postfix-tail* ｜ primary postfix-tail*

postfix-tail = "[" expr "]" | "(" func-args ")"| "." ident| "->" ident| "++"| "--"

primary  "(" "{" stmt+ "}" ")" | "(" expr ")"|| "sizeof" "(" type-name ")"|| "sizeof" unary|"_Alignof" "(" type-name ")"| "_Alignof" unary| ident| str| num



for example 
x = 3; 
conditional (assign-op assign) ==> primary assign-op primary



*/

// static void error(char *fmt, ...) {
//   va_list ap;
//   va_start(ap, fmt);
//   vfprintf(stderr, fmt, ap);
//   fprintf(stderr, "\n");
//   exit(1);
// }

// // Reports an error location and exit.
// static void verror_at(char *loc, char *fmt, va_list ap) {
//   int pos = loc - current_input;
//   fprintf(stderr, "%s\n", current_input);
//   fprintf(stderr, "%*s", pos, ""); // print pos spaces.
//   fprintf(stderr, "^ ");
//   vfprintf(stderr, fmt, ap);
//   fprintf(stderr, "\n");
//   exit(1);
// }

// static void error_at(char *loc, char *fmt, ...) {
//   va_list ap;
//   va_start(ap, fmt);
//   verror_at(loc, fmt, ap);
// }

// static void error_tok(Token *tok, char *fmt, ...) {
//   va_list ap;
//   va_start(ap, fmt);
//   verror_at(tok->loc, fmt, ap);
// }



static void error_log(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    vprintf(fmt, ap);

    exit(-1);
}

static bool equal(Token *tok, char *op) {
    return memcmp(tok->loc, op, tok->len) == 0 && op[tok->len] == '\0';
}

static Token *skip(Token *tok, char *s) {
    if (!equal(tok, s)) {
        return NULL;
    }
    return list_next_entry(tok, list);
}


static Node *new_node(NodeKind kind, mpool_t *pool) {
    Node *node = mpool_calloc(pool, sizeof(Node));
    if (node == NULL) {
        return NULL;
    }

    node->kind = kind;
    return node;
}

static Node *new_num(int val, mpool_t *pool) {
    Node *node = new_node(ND_NUM, pool);
    node->val = val;
    return node;
}


static Node *new_binary(NodeKind kind, Node *lhs, Node *rhs, mpool_t *pool) {
    Node *node = new_node(kind, pool);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

static Node *new_add(Node *lhs, Node *rhs, mpool_t *pool) {
    return new_binary(ND_ADD, lhs, rhs, pool);
}

static Node *new_sub(Node *lhs, Node *rhs, mpool_t *pool) {
    return new_binary(ND_SUB, lhs, rhs, pool);
}

static Node *new_mul(Node *lhs, Node *rhs, mpool_t *pool) {
    return new_binary(ND_MUL, lhs, rhs, pool);
}

static Node *new_div(Node *lhs, Node *rhs, mpool_t *pool) {
    return new_binary(ND_DIV, lhs, rhs, pool);
}

//primary is the highest priority
static Node *primary(Token **rest, Token *tok, mpool_t *pool) {
    if (equal(tok, "(")) {
        Node *node = expr(&tok, list_next_entry(tok, list), pool);
        *rest = skip(tok, ")");
        return node;
    }

    if (tok->kind == TK_NUM) {
        Node *node = new_num(tok->val, pool);

        *rest = list_next_entry(tok, list);
        return node;
    }

    error_log("expected an expression");
    return NULL;
}

static Node *mul(Token **rest, Token *tok, mpool_t *pool) {
    Node *node = primary(&tok, tok, pool);

    for (;;) {
        if (equal(tok, "*")) {
            node = new_mul(node, primary(&tok, list_next_entry(tok, list), pool), pool);
            continue;
        }

        if (equal(tok, "/")) {
            node = new_div(node, primary(&tok, list_next_entry(tok, list), pool), pool);
            continue;
        }

        *rest = tok;
        return node;
    }
}

static Node *add(Token **rest, Token *tok, mpool_t *pool) {
    Node *node, *rnode;

    node = mul(&tok, tok, pool);

    for (;;) {

        if (equal(tok, "+")) {
            rnode = mul(&tok, list_next_entry(tok, list), pool);

            node = new_add(node, rnode, pool);
            continue;
        }

        if (equal(tok, "-")) {
            rnode = mul(&tok, list_next_entry(tok, list), pool);
            node = new_sub(node, rnode, pool);
            continue;
        }

        *rest = tok;
        return node;
    }
}

static Node *logand(Token **rest, Token *tok, mpool_t *pool) {
    Node *node, *rnode;
    node = add(&tok, tok, pool);

    for (;;) {
        if (equal(tok, "&&")) {
            rnode = add(&tok, list_next_entry(tok, list), pool);

            node = new_binary(ND_LOGAND, node, rnode, pool);
        }

        *rest = tok;
        return node;
    }
}

static Node *logor(Token **rest, Token *tok, mpool_t *pool) {
    Node *node, *rnode;
    node = logand(&tok, tok, pool);

    for (;;) {
        if (equal(tok, "||")) {
            rnode = logand(&tok, list_next_entry(tok, list), pool);

            node = new_binary(ND_LOGOR, node, rnode, pool);
        }

        *rest = tok;
        return node;
    }
    
}

static Node *expr(Token **rest, Token *tok, mpool_t *pool) {
    return logor(rest, tok, pool);
}


Node* parser(Token *tok, mpool_t *pool) {

    return expr(&tok, tok, pool);
    
    // return NULL;
}


int compute(Node *node) {

    if (node->kind == ND_ADD) {
        return compute(node->lhs) + compute(node->rhs);
    }

    if (node->kind == ND_SUB) {
        return compute(node->lhs) - compute(node->rhs);
    }

    if (node->kind == ND_MUL) {
        return compute(node->lhs) * compute(node->rhs);
    }

    if (node->kind == ND_DIV) {
        return compute(node->lhs) / compute(node->rhs);
    }

    if (node->kind == ND_LOGAND) {
        return compute(node->lhs) && compute(node->rhs);
    }

    if (node->kind == ND_LOGOR) {
        return compute(node->lhs) || compute(node->rhs);
    }

    if (node->kind == ND_NUM) {
        return node->val;
    }

    return 0;
}
