#include "token.h"
#include "list.h"
#include "mpool.h"
#include "parser.h"
#include "type.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

static bool startswith(const char *p, char *q) {
  return strncmp(p, q, strlen(q)) == 0;
}

char *token_read_file(char *path) {
    FILE *fp;

    if (strcmp(path, "-") == 0) {
        // By convention, read from stdin if a given filename is "-".
        fp = stdin;
    } else {
        fp = fopen(path, "r");
        if (!fp)
        return NULL;
    }

    char *buf;
    size_t buflen;
    FILE *out = open_memstream(&buf, &buflen);

    // Read the entire file.
    for (;;) {
        char buf2[4096];
        int n = fread(buf2, 1, sizeof(buf2), fp);
        if (n == 0) {
            break;
        }

        fwrite(buf2, 1, n, out);
    }

    if (fp != stdin)
        fclose(fp);

    // Make sure that the last line is properly terminated with '\n'.
    fflush(out);
    if (buflen == 0 || buf[buflen - 1] != '\n')
        fputc('\n', out);
    fputc('\0', out);
    fclose(out);
    return buf;
}

typedef struct {
    string_t name;
    bool l;
    bool u;
}lu_t;

static bool convert_int(Token *tok) {
    const char *p;
    int base = 10;
    int64_t val;
    bool l = false, u = false;
    size_t i;
    Type *ty;


    p = tok->loc;

    if (strncasecmp(p, "0b", 2) == 0) {
        p += 2;
        base = 16;
    }else if (strncasecmp(p, "0b", 2) == 0) {
        p += 2;
        base = 2;
    }else if (*p == '0') {
        p += 1;
        base = 8;
    }

    val = strtoul(p, (char **)&p, base);


    lu_t lus[] = {
        { .name = string("LLU"), .l = true, .u = true },
        { string("LLu"), true, true },
        { string("llU"), true, true },
        { string("llu"), true, true },
        { string("ULL"), true, true },
        { string("Ull"), true, true },
        { string("ull"), true, true },
        { string("uLL"), true, true },
        { string("lu"), true, true },
        { string("ul"), true, true },
        { string("L"), true, false },
        { string("l"), true, false },
        { string("U"), false, true },
        { string("u"), false, true }
    };

    for (i = 0; i < ARRAY_SIZE(lus); i++) {
        if (startswith(p, lus[i].name.data)) {
            p += lus[i].name.len;
            l = lus[i].l;
            u = lus[i].u;
            break;
        }
    }
    
    if (p != tok->end) {
        //may be is a float or a illigal number
        return false;
    }

    if (base == 10) {
      if (l && u) {
        ty = ty_ulong;
      } else if (l) {
        ty = ty_long;
      } else if (u) {
        ty = (val >> 32) ? ty_ulong : ty_uint;
      }
      else {
        ty = (val >> 31) ? ty_long : ty_int;
      }

    } else {
      if (l && u) {
        ty = ty_ulong;
      }

      else if (l) {
        ty = (val >> 63) ? ty_ulong : ty_long;
      }
      
      else if (u) {
        ty = (val >> 32) ? ty_ulong : ty_uint;
      }

      else if (val >> 63) {
        ty = ty_ulong;
      }

      else if (val >> 32) {
        ty = ty_long;
      }

      else if (val >> 31) {
        ty = ty_uint;
      }
      else {
        ty = ty_int;
      }
    }

    assert(tok->kind == TK_NUM);
    tok->val = val;
    tok->ty = ty;

    return true;
}

static bool convert_number(Token *tok) {
    long double val;
    char *end;
    Type *ty;
    if (convert_int(tok)) {
        return true;
    }

    val = strtold(tok->loc, &end);

    ty = ty_ldouble;

    if (end[0] == 'f' || end[0] == 'F') {
        ty = ty_float;
        end++;
    }else if (end[0] == 'l' || end[0] == 'L') {
        ty = ty_ldouble;
        end++;
    }

    if (end != tok->end) {
        error_log("invalid numeric constant");
    }

    assert(tok->kind == TK_NUM);
    tok->fval = val;
    tok->ty = ty;
    return true;
}

Token *new_token(TokenKind type, char *start, char *end, mpool_t *pool) {
    Token *t;
    t = mpool_calloc(pool, sizeof(Token));
    if (t == NULL) {
        return NULL;
    }
    t->kind = type;
    t->loc = start;
    t->end = end;
    t->len = end - start;
    
    return t;
}

Token *new_token_num(char *start, char *end, mpool_t *pool) {
    Token *t;
    t = new_token(TK_NUM, start, end, pool);
    if (t == NULL) {
        return NULL;
    }

    if (!convert_number(t)) {
        return NULL;
    }

    return t;
}

Token *new_token_eof(char *end, mpool_t *pool) {
    Token *t;
    t = new_token(TK_EOF, end , end, pool);
    if (t == NULL) {
        return NULL;
    }
    return t;
}

Token * new_token_punct(char *start, char *end, mpool_t *pool) {
    Token *t;
    t = new_token(TK_PUNCT, start , end, pool);
    if (t == NULL) {
        return NULL;
    }

    return t;
}

static int read_punct(char *p) {
    static char *kw[] = {
        "<<=", ">>=", "...", "==", "!=", "<=", ">=", "->", "+=",
        "-=", "*=", "/=", "++", "--", "%=", "&=", "|=", "^=", "&&",
        "||", "<<", ">>", "##",
    };

    for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++) {
        if (startswith(p, kw[i])) {
            return strlen(kw[i]);
        }
    }

    return ispunct(*p) ? 1 : 0;
    
}

Token *tokenize(char *p, mpool_t *pool) {
    Token *head, *t;
    int punct_len;
    head = new_token(0, NULL, NULL, pool);
    if (head == NULL) {
        return NULL;
    }

    INIT_LIST_HEAD(&head->list);

    
    for (; *p ;) {

        if (startswith(p, "//")) {
            p+=2;
            while (*p != '\n') {
                p++;
            }
            continue;
        }

        // Skip block comments.
        if (startswith(p, "/*")) {
            char *q = strstr(p + 2, "*/");
            if (!q) {
                printf("unclosed block comment\n");
                goto err;
            }
            p = q + 2;
            continue;
        }

        if (*p == '\n') {
            p++;
            continue;
        }

        if (isspace(*p)) {
            p++;
            continue;
        }

        // Numeric literal
        if (isdigit(*p) || (*p == '.' && isdigit(p[1]))) {
            char *q = p++;

            for (;;) {

                if (p[0] && p[1] && strchr("eEpP", p[0]) && strchr("+-", p[1])) {
                    p += 2;
                }

                else if (isalnum(*p) || *p == '.') {
                    p++;
                }else {
                    break;
                }
            }

            t = new_token_num(q, p, pool);
            if (t == NULL) {
                goto err;
            }

            list_add_tail(&t->list, &head->list);

            continue;
        }

        //keyword

        //Punctuators
        punct_len = read_punct(p);
        if (punct_len) {
            t = new_token_punct(p, p + punct_len, pool);
            if (t == NULL) {
                goto err;
            }

            p += punct_len;
            list_add_tail(&t->list, &head->list);

            continue;
        }

        printf("invalid token\n");
        goto err;
    }

    t = new_token_eof(p, pool);
    if (t == NULL) {
        goto err;
    }
    list_add_tail(&t->list, &head->list);

    return head;
err:

    return NULL;
}

Token *tokenize_file(char *file, mpool_t *pool) {
    char *p = token_read_file(file);
    if (p == NULL) {
        return NULL;
    }

    return tokenize(p, pool);
}