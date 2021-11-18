#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <stdio.h>
#include <string.h>
void hexToString(unsigned char *hex,char *str,int len){

    for (size_t i = 0; i < len; i++){
        sprintf(str+i*2,"%02x",hex[i]);
    }
    return;
}
int main(int argc, char const *argv[])
{
    unsigned int md_len = 0;
    char *md = NULL;
    char o[100] = {0};

    HMAC(EVP_md5(),"password",7,"hello world",10,NULL,&md_len);
    md =  malloc((size_t)md_len);

    HMAC(EVP_md5(),"password",7,"hello world",10,md,&md_len);

    hexToString(md,o,md_len);

    printf("%d: %s\n",md_len,o);

    free(md);
    return 0;
}
