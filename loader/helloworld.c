#include <stdio.h>
#include <unistd.h>

int main(void)
{
    for (;;) {
        printf("helloworld\n");
        sleep(1);
    }
    return 0;
}
