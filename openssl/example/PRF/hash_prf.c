#include <stdio.h>
#include <string.h>
#include <stdlib.h>
char helloworld[] = "hello world";

void set_seed(char *seed) {
    seed = malloc(strlen(helloworld) * 10);
    for(int i=0; i< 10 ;i++) {
        strcpy(seed + i* strlen(helloworld),helloworld);
    }
    return;
}

int round_up(int x,int y){
    if(x % y  == 0) {
        return x / y;
    }
    else {
        return x / y + 1;
    }
}

int main(int argc, char const *argv[])
{
    char *seed = NULL;
    int wanted_output_size = 1024; //需要输出的位数
    int hash_size = 16;
    
    set_seed(seed);

    for(int i = 0;i < round_up(wanted_output_size,hash_size);i++) {
        
    }

    return 0;
}
