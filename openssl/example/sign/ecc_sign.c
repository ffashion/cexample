#include <openssl/pem.h>
#include <openssl/err.h>
#include <string.h>
EC_KEY *find_pkey1(char *name){
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


    if(pkey_type != EVP_PKEY_EC) {
        return NULL;
    }

    return EVP_PKEY_get0_EC_KEY(pkey);
}

EC_KEY *find_pkey2(char *name) {
    BIO *bio = NULL;
    bio = BIO_new_file(name,"r");
    if(!bio) {
        return NULL;
    }
    return PEM_read_bio_ECPrivateKey(bio,NULL,NULL,NULL);
}

void output_base64(char *in,int in_len) {
    char output[10240] = {0};
    int rc = 0;
    rc = EVP_EncodeBlock(output,in,in_len);
    if(!rc) {
        return ;
    }
    printf("原始长度:%d: %s\n",in_len,output);
}

int main(int argc, char const *argv[])
{
    EC_KEY *ec_key = NULL;
    ECDSA_SIG *ecdsa_sig = NULL;
    int ecdsa_key_size = 0;
        
    unsigned char *sig = NULL;
    int sig_size = 0;

    //  ECDSA_SIG *ecdsa_sig = NULL;
    char data[] = "hello world";
    int data_len = strlen(data);
    char md[EVP_MAX_MD_SIZE] = {0};
    
    int md_size = 0;
   
    int rc = 0;
    ec_key = find_pkey1("ecc_private_web.pem");
    if(!ec_key) {
        return -1;
    }
    output_base64(data,data_len);
  
    rc = EVP_Digest(data,data_len,md,&md_size,EVP_md5(),NULL);
    
    if (!rc) {
        return -1;
    }
    printf("签名原始数据\n");
    output_base64(md,md_size);

    ecdsa_key_size =  ECDSA_size(ec_key);
  
    // ecdsa_sig = ECDSA_do_sign(md,md_size,ec_key);
    // if(!ecdsa_sig) {
    //     return -1;
    // }

    printf("签名md5后数据 方式1\n");
    sig = malloc(ecdsa_key_size); //72
    //sig must point to ECDSA_size(eckey) bytes of memory. sig 为DER格式的数据
    ECDSA_sign(EVP_PKEY_EC,md,md_size,sig,&sig_size,ec_key); //ossl_ecdsa_sign 调用ECDSA_do_sign 和i2d_ECDSA_SIG
    output_base64(sig,sig_size);

    free(sig);
    sig = NULL; //很重要 不然走不同的逻辑


    printf("签名md5后数据 方式2\n");
    ecdsa_sig = ECDSA_do_sign(md,md_size,ec_key); //ossl_ecdsa_sign_sig
    //输出DER编码
    sig_size =  i2d_ECDSA_SIG(ecdsa_sig,&sig);
    output_base64(sig,sig_size);


    ECDSA_SIG_free(ecdsa_sig);
    EC_KEY_free(ec_key);
     // ECDSA_SIG_free(ecdsa_sig);
    return 0;
}
