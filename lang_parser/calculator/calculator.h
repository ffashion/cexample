#ifndef __CALCULATOR__H__
#define __CALCULATOR__H__

#include <stdio.h>

#define MAX_FILE_NAME_SIZE 100
typedef struct token_obj {
    char file_name[MAX_FILE_NAME_SIZE];
    FILE *f;

    char *fbuf; //file content in file
    long f_buflen;


    char *start;
    char *end;
    char *pos;
    char token;
    int value;

}token_obj_t;

#define ARRAY_SIZE(arr) ( \
    sizeof(arr) / sizeof((arr)[0]) \
)

#endif  //!__CALCULATOR__H__