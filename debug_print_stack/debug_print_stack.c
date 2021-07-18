#include <unistd.h>
#include <signal.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#define BACKTRACE_SIZE 256
void segv_handler(int sig)
{
    void *func[BACKTRACE_SIZE];
    char **symb = NULL;
    int size;
    size = backtrace(func, BACKTRACE_SIZE);
    backtrace_symbols_fd(func, size, STDERR_FILENO);
    exit(1);
}


int main(void)
{
    int *p = NULL;
    signal(SIGSEGV, segv_handler);
    *p = 0xdeadbeef;
    return 0;
}