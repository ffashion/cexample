#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
extern void _init(void);
int main(int argc, char const *argv[])
{
    //all undefined symbols in the shared object are resolved before dlopen() returns
    dlopen("./lib.so",RTLD_NOW);
    perror("dlopen ");
    return 0;
}
