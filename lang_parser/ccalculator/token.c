#include "token.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

static bool startswith(char *p, char *q) {
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

Token *new_token(tokentype_t type, char *start, char *end) {
    Token *t;
    t = malloc(sizeof(Token));
    if (t == NULL) {
        return NULL;
    }
    t->kind = type;
    t->loc = start;
    t->end = end;
    t->len = end - start;
    
    return t;
}

Token *new_token_num(char *start, char *end) {
    Token *t;
    t = new_token(TK_NUM, start, end);
    if (t == NULL) {
        return NULL;
    }
    //now we assume is int number

    t->val = atoi(start);

    return t;
}

Token *new_token_eof(char *end) {
    Token *t;
    t = new_token(TK_EOF, end , end);
    if (t == NULL) {
        return NULL;
    }
    return t;
}

Token * new_token_punct(char *start, char *end) {
    Token *t;
    t = new_token(TK_PUNCT, start , end);
    if (t == NULL) {
        return NULL;
    }

    return t;
}



Token *tokenize(char *p) {
    Token *head, *t;

    head = new_token(0, NULL, NULL);
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

            t = new_token_num(q, p);
            if (t == NULL) {
                goto err;
            }

            list_add_tail(&t->list, &head->list);

            continue;
        }


        //keyword

        //Punctuators
        if (ispunct(*p)) {
            t = new_token_punct(p, p +1);
            if (t == NULL) {
                goto err;
            }
            p++;
            
            list_add_tail(&t->list, &head->list);

            continue;
        }

        printf("invalid token\n");
        goto err;
    }

    t = new_token_eof(p);
    if (t == NULL) {
        goto err;
    }
    list_add_tail(&t->list, &head->list);

    return head;
err:

    return NULL;
}

Token *tokenize_file(char *file) {
    char *p = token_read_file(file);
    if (p == NULL) {
        return NULL;
    }

    return tokenize(p);
}