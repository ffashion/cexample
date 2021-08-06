#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


//way 1
#define _init __attribute__((constructor)) _INIT
void _init(void){
    printf("hello world\n");
}

//way 2
//_init 是gcc 的函数名 不能随便用
void __attribute__((constructor)) _test_init(void){
    printf("hello world\n");
}

void __attribute__((destructor)) _test_destory(void){
    printf("destory\n");
}