
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "calculator.h"


//tokens stream have stat, so we need a next function

/*
    1. number, op 
    2. op, number | (
    3. (, number
speical:
    (20 + 5)
*/
enum token_type {Num, Add, Sub, Mul, Div};

struct token {
    char c;
    enum token_type type;
};

struct token tokens[] = {
    {
        .c = 0xff,
        .type = Num
    },
    {
        .c = '+',
        .type = Add
    }, 
    {
        .c = '-',
        .type = Sub
    }, 
    {
        .c = '*',
        .type = Mul
    }, 
    {
        .c = '/',
        .type = Div
    }, 
};

void token_try_number(token_obj_t *obj) {

}


int expr(token_obj_t *obj);
void next(token_obj_t *obj);

void match(token_obj_t *obj, int tk) {

    if (obj->token != tk) {
        printf("expected token: %d(%c), got: %d(%c)\n", tk, tk, obj->token, obj->token);
        exit(-1);
    }
    next(obj);
}

/*
    factor process this two conditions 
        condition 1: 1 + 2, now token is 1
        condition 2: (1 + 2), now token is (
*/
int factor(token_obj_t *obj) {
    int value = obj->value;
    char token = obj->token;
    int ret = 0;
    if (token == '(') {
        //also call next() read token..
        match(obj, '(');
        ret = expr(obj);
        match(obj, ')');
    } else {
        ret = value;
        match(obj, Num);
    }
    return ret;
}

int term_tail(token_obj_t *obj, int lvalue) {
    char token = obj->token;
    if (token == '*') {
        match(obj, '*');
        int value = lvalue * factor(obj);
        return term_tail(obj, value);
    } else if (token == '/') {
        match(obj, '/');
        int value = lvalue / factor(obj);
        return term_tail(obj, value);
    } else {
        return lvalue;
    }
}

int term(token_obj_t *obj) {
    
    int lvalue = factor(obj);

    return term_tail(obj, lvalue);
}

int expr_tail(token_obj_t *obj, int lvalue) {
    char token = obj->token;
    if (token == '+') {
        match(obj, '+');
        int value = lvalue + term(obj);
        return expr_tail(obj, value);
    } else if (token == '-') {
        match(obj, '-');
        int value = lvalue - term(obj);
        return expr_tail(obj, value);
    } else {
        return lvalue;
    }
}

int expr(token_obj_t *obj) {
    int lvalue = term(obj);
    return expr_tail(obj, lvalue);
}

void next(token_obj_t *obj) {
    // skip white space
    char *start = obj->start;
    while (*start == ' ' || *start == '\t') {
        start++;
    }

    obj->token = *start++;

    if (obj->token >= '0' && obj->token <= '9' ) {
        obj->value = obj->token - '0';
        obj->token = Num;

        while (*start >= '0' && *start <= '9') {
            obj->value = obj->value*10 + *start - '0';
            start++;
        }
        obj->start = start;
        return;
    }
    obj->start = start;

    //not number token return
    
}


void *malloc_safe(size_t size) {
    void *p;
    p = malloc(size);
    assert(p);
    return p;
}

void open_token_file(token_obj_t *obj, char *name) {
    assert(strlen(name) < MAX_FILE_NAME_SIZE);
    int flen;
    strcpy(obj->file_name, name);
    obj->f = fopen(name, "r");
    assert(obj->f);

    fseek(obj->f,0,2);
    flen  = ftell(obj->f);
    rewind(obj->f);
    
    obj->fbuf =  malloc_safe(flen + 1);
    obj->f_buflen = flen + 1;

    assert(fread(obj->fbuf, 1, obj->f_buflen, obj->f) == obj->f_buflen -1);
    rewind(obj->f);
}


void close_token_file(token_obj_t *obj) {
    assert(obj);
    fclose(obj->f);
    free(obj->fbuf);

}

#define MAX_LINE 1024
int main(int argc, char *argv[])
{
    size_t linecap = 0;
    ssize_t linelen;
    char buf[MAX_LINE];

    token_obj_t obj;
    open_token_file(&obj, "test.calculator");

    for (;;) {
        if (fgets(buf, MAX_LINE, obj.f) == NULL) {
            if (feof(obj.f)) {
                break;
            }else {
                //empty line
                continue;
            }
        }
        if (buf[0] == '\n' || buf[0] == '\r' || (buf[0] == '\r' && buf[1] == '\n')) {
            continue;
        }
        obj.start = buf;
        next(&obj);
        printf("%d\n", expr(&obj));
    }

    close_token_file(&obj);
    return 0;
}

