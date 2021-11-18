#include <openssl/pem.h>
#include <openssl/err.h>
#include <string.h>
RSA *find_pkey1(char *name){
    BIO *bio = NULL;
    EVP_PKEY *pkey = NULL; //可以存放各种私钥，RSA ECC 都可以 比较有通用性
    int pkey_type; //私钥类型 ECC RSA等
    
    bio = BIO_new_file(name,"r");
    if(!bio) {
        return NULL;
    }

    pkey = PEM_read_bio_PrivateKey(bio,NULL,NULL,NULL);
    if(!pkey) {
        return NULL;
    }
    pkey_type =  EVP_PKEY_id(pkey);


    if(pkey_type != EVP_PKEY_RSA) {
        return NULL;
    }

    BIO_free_all(bio);
    return EVP_PKEY_get0_RSA(pkey);
}

RSA *find_pkey2(char *name) {
    BIO *bio = NULL;
    bio = BIO_new_file(name,"r");
    if(!bio) {
        return NULL;
    }
    return PEM_read_bio_RSAPrivateKey(bio,NULL,NULL,NULL);
}

void output_base64(unsigned char *in,int in_len) {
    unsigned char output[1024] = {0};
    int rc = 0;
    rc = EVP_EncodeBlock(output,in,in_len);
    if(!rc) {
        return ;
    }
    printf("原始长度:%d: base64长度: %d: %s\n",in_len,rc,output);
}
void hexToString(unsigned char *hex,char *str,int len){
    for (size_t i = 0; i < len; i++){
        sprintf(str+i*2,"%02x",hex[i]);
    }
    return;
}

int main(int argc, char const *argv[])
{
    unsigned char data[] = "hello world";
    unsigned char md[EVP_MAX_MD_SIZE] = {0};
    
    unsigned int md_size = 0;
    RSA *rsa = NULL;
    int rc = 0;
    int rsa_size = 0;
    rsa = find_pkey1("rsa_private_web.pem");
    if(!rsa) {
        return -1;
    }
    rsa_size = RSA_size(rsa);//RSA长度 产生的密文长度不会超过这个长度
    // EVP_PKEY_size(pkey);

    int data_len = strlen((char *)data);
    //signed_data 指向的空间 必须为rsa_size大小
    unsigned char *signed_data = malloc(rsa_size);

    rc = EVP_Digest(data,data_len,md,&md_size,EVP_md5(),NULL);
    if(!rc) {
        return -1;
    }
    printf("hello world\n");
    output_base64(data,data_len);
    printf("hello world md5\n");
    output_base64(md,md_size);



    //需要保证数据的长度小于等于RSA_size(rsa);
    printf("使用RSA_PKCS1_PADDING模式签名原始数据\n");
    rc = RSA_private_encrypt(data_len,data,signed_data,rsa,RSA_PKCS1_PADDING); //256
    if(rc <= 0) {
        return -1;
    }
    output_base64(signed_data,rc);
    
    // char signed_data_base64[1000] = {0};
    // char signed_data_str[1000] = {0};
    // char signed_data2[1000]  = {0};
    // hexToString(signed_data,signed_data_str,rc);
    // printf("%s\n",signed_data_str);

    // rc = EVP_EncodeBlock(signed_data_base64,signed_data,rc);

    // rc = EVP_DecodeBlock(signed_data2,signed_data_base64,rc);
    // hexToString(signed_data2,signed_data_str,rc);
    // printf("%s\n",signed_data_str);


    

    // char o[100] = {0};
    // rc = RSA_public_decrypt(rc,signed_data,o,rsa,RSA_PKCS1_PADDING);
    // printf("%s\n",o);
    // // output_base64(o,rc);


    printf("使用RSA_PKCS1_PADDING模式签名MD5后数据\n");
    rc = RSA_private_encrypt(md_size,md,signed_data,rsa,RSA_PKCS1_PADDING); //256
    output_base64(signed_data,rc);



    if(rc <= 0) {
        ERR_load_CRYPTO_strings();
        char buf[100] = {0};
        int e = ERR_get_error();
        ERR_error_string_n(e,buf,100);
        printf("%s\n",buf);
        goto error;
    }   
    return 0;
error:
    return -1;
}


