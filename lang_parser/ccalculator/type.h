#ifndef __TYPE__H__
#define __TYPE__H__

#include <stdbool.h>

typedef struct Type Type;

typedef enum {
    TY_BOOL,
    TY_CHAR,
    TY_SHORT,
    TY_INT,
    TY_LONG,
    TY_FLOAT,
    TY_DOUBLE,
    TY_LDOUBLE,
}TypeKind;

struct Type { 
    TypeKind kind;
    int size;           // sizeof() value
    int align;          // alignment
    bool is_unsigned;   // unsigned or signed
};


extern Type *ty_char;
extern Type *ty_short;
extern Type *ty_int;
extern Type *ty_long;

extern Type *ty_uchar;
extern Type *ty_ushort;
extern Type *ty_uint;
extern Type *ty_ulong;

extern Type *ty_float;
extern Type *ty_double;
extern Type *ty_ldouble;


#endif  /*__TYPE__H__*/