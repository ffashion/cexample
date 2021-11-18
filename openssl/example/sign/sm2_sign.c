#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/ec.h>
#include <openssl/ecdh.h>
#include <openssl/ecdsa.h>
#include <string.h>
extern int sm2_sign(const unsigned char *dgst, int dgstlen,
             unsigned char *sig, unsigned int *siglen, EC_KEY *eckey);

extern ECDSA_SIG *sm2_do_sign(const EC_KEY *key,
                       const EVP_MD *digest,
                       const uint8_t *id,
                       const size_t id_len,
                       const uint8_t *msg, size_t msg_len);
EC_KEY *find_pkey1(char *name){
    BIO *bio = NULL;
    EVP_PKEY *pkey = NULL; //可以存放各种私钥，RSA ECC 都可以 比较有通用性
    EC_KEY *ec_key = NULL;
    const EC_GROUP *ec_group = NULL;
    int pkey_type; //私钥类型 ECC RSA等
    
    bio = BIO_new_file(name,"r");
    if(!bio) {
        goto error;
    }

    pkey = PEM_read_bio_PrivateKey(bio,NULL,NULL,NULL);
    if(!pkey) {
        goto error;
    }
    pkey_type =  EVP_PKEY_id(pkey);


    if(pkey_type != EVP_PKEY_EC) {
        goto error;
    }


    ec_key = EVP_PKEY_get0_EC_KEY(pkey); //不能释放pkey
    if(!ec_key) {
        goto error;
    }

    ec_group =  EC_KEY_get0_group(ec_key);
    if(!ec_group){
        goto error;
    }

    if(EC_GROUP_get_curve_name(ec_group) != NID_sm2) {
        printf("此私钥不是SM2私钥\n");
        goto error;
    }

    BIO_free(bio);
    return ec_key;
error:
    BIO_free(bio);
    EVP_PKEY_free(pkey);
    EC_KEY_free(ec_key);
    return NULL;   
}

EC_KEY *find_pkey2(char *name) {
    BIO *bio = NULL;
    bio = BIO_new_file(name,"r");
    if(!bio) {
        return NULL;
    }
    return PEM_read_bio_ECPrivateKey(bio,NULL,NULL,NULL);
}

void output_base64(unsigned char *in,int in_len) {
    unsigned char output[1024] = {0};
    int rc = 0;
    rc = EVP_EncodeBlock(output,in,in_len);
    if(!rc) {
        return ;
    }
    printf("原始长度:%d: %s\n",in_len,output);
}

void sm2_sign1(const unsigned char *data, int data_len,EC_KEY *eckey) {
    unsigned char *sig = NULL;
    unsigned int sig_len = 0;
    unsigned char md[EVP_MAX_MD_SIZE] = {0};
    unsigned  int md_size = 0;
    int ec_key_size = ECDSA_size(eckey);
    sig = malloc(ec_key_size);
    
    EVP_Digest(data,data_len,md,&md_size,EVP_sm3(),NULL);

    sm2_sign(md,md_size,sig,&sig_len,eckey);
    
    output_base64(sig,sig_len);
    
    free(sig);
}
void sm2_sign2(const unsigned char *dgst, int dgstlen,EC_KEY *eckey) {
    ECDSA_SIG *ecdsa_sig = NULL;
    unsigned char *sig = NULL;
    unsigned int sig_len = 0;

    // ecdsa_sig =  sm2_do_sign(eckey,EVP_sm3(),(uint8_t *)"1234567812345678",16,dgst,dgstlen);
    ecdsa_sig =  sm2_do_sign(eckey,EVP_sm3(),NULL,0,dgst,dgstlen);
    sig_len = i2d_ECDSA_SIG(ecdsa_sig,&sig);
    output_base64(sig,sig_len);
    ECDSA_SIG_free(ecdsa_sig);
}
int main(int argc, char const *argv[])
{
    EC_KEY *ec_pkey = NULL;
    unsigned char data[] = "hello world";
    int data_len = strlen((char *)data);

    ec_pkey =  find_pkey1("sm2_private_web.pem");
    
    if(!ec_pkey) {
        return -1;
    }


    printf("输出方式1 原始签名数据\n");
    sm2_sign1(data,data_len,ec_pkey);


    printf("输出方式2 原始签名数据\n");
    sm2_sign2(data,data_len,ec_pkey);

    return 0;
}
