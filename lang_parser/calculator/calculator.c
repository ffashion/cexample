
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "calculator.h"

/**
    Todo:
        support op priority

*/
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
    int priority;
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
        .priority = 0,
        .type = Add
    }, 
    {
        .c = '-',
        .priority = 0,
        .type = Sub
    }, 
    {
        .c = '*',
        .priority = 1,
        .type = Mul
    }, 
    {
        .c = '/',
        .priority = 1,
        .type = Div
    },

};

void token_try_number(token_obj_t *obj) {

}


int expr_value(token_obj_t *obj);
void next(token_obj_t *obj);
int first_priority(token_obj_t *obj);
int second_priority(token_obj_t *obj, int lvalue);
int third_priority(token_obj_t *obj, int lvalue);

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

    //other token return    
}

void match_next(token_obj_t *obj, int tk) {

    if (obj->token != tk) {
        printf("expected token: %d(%c), got: %d(%c)\n", tk, tk, obj->token, obj->token);
        exit(-1);
    }
    next(obj);
}

int subexpr_value(token_obj_t *obj) {
    //fist priority is (
    int lvalue = first_priority(obj);
    //second priority is * and /
    return second_priority(obj, lvalue);
}

int expr_value(token_obj_t *obj) {
    int lvalue = subexpr_value(obj);
    //the last  priority is + and /
    return third_priority(obj, lvalue);
}

/*
    factor process this two conditions 
        condition 1: 1 + 2, now token is 1
        condition 2: (1 + 2), now token is (
*/
int first_priority(token_obj_t *obj) {
    int value = obj->value;
    char token = obj->token;
    int ret = 0;
    if (token == '(') {
        //also call next() read token..
        match_next(obj, '(');
        ret = expr_value(obj);
        match_next(obj, ')');
    } else {
        ret = value;
        match_next(obj, Num);
    }
    return ret;
}

int second_priority(token_obj_t *obj, int lvalue) {
    char token = obj->token;
    if (token == '*') {
        match_next(obj, '*');
        int value = lvalue * first_priority(obj);
        return second_priority(obj, value);
    } else if (token == '/') {
        match_next(obj, '/');
        int value = lvalue / first_priority(obj);
        return second_priority(obj, value);
    } else {
        return lvalue;
    }
}

int third_priority(token_obj_t *obj, int lvalue) {
    char token = obj->token;
    if (token == '+') {
        match_next(obj, '+');
        int value = lvalue + subexpr_value(obj);
        return third_priority(obj, value);
    } else if (token == '-') {
        match_next(obj, '-');
        int value = lvalue - subexpr_value(obj);
        return third_priority(obj, value);
    } else {
        return lvalue;
    }
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
                //error
                assert(0);
            }
        }
        //skip whitw space line
        if (buf[0] == '\n' || buf[0] == '\r' || (buf[0] == '\r' && buf[1] == '\n')) {
            continue;
        }
        //skip comment line 
        if (buf[0] == '/' && buf[1] == '/') {
            continue;
        }
        obj.start = buf;
        next(&obj);
        printf("%d\n", expr_value(&obj));
    }

    close_token_file(&obj);
    return 0;
}

