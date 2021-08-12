#include <dlfcn.h>
#include <unistd.h>
#include <stdio.h>
int main(int argc, char const *argv[])
{   
    void *handle;
    //all undefined symbols in the shared object are resolved before dlopen() returns
    handle = dlopen("./lib.so",RTLD_NOW);
    
    dlclose(handle);
    return 0;
}
