#include <openssl/md5.h>
#include <stdio.h>
#include <string.h>
void md5hexToString(unsigned char *md,char *result){
    // char tmp[3];
    for (size_t i = 0; i <= 15; i++){
        sprintf(result+i*2,"%02x",md[i]);
    }
    return;
}

int main(int argc, char const *argv[])
{	
    //存储md5的hex结果
    unsigned char md[16] = {0};
    //存储hex对应的字符串结果
    char result[33] = {0};
    
    MD5_CTX c;
    MD5_Init(&c);
    
    //这个方式你可以多次执行Update函数
    MD5_Update(&c,"helloworld1",10);
    MD5_Update(&c,"helloworld2",10);
    
    MD5_Final(md,&c);
    
    md5hexToString(md,result);
    printf("%s",result);
    return 0;
}