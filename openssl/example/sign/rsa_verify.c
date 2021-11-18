#include <openssl/pem.h>
#include <openssl/err.h>
#include <string.h>
RSA *find_pubkey1(char *name) {
    BIO *bio = NULL;
    bio = BIO_new_file(name,"r");

    return PEM_read_bio_RSA_PUBKEY(bio,NULL,NULL,NULL);

}
void hexToString(unsigned char *hex,char *str,int len){
    // char tmp[3];
    for (size_t i = 0; i < len; i++){
        sprintf(str+i*2,"%02x",hex[i]);
    }
    return;
}
//344 对的
unsigned char signed_data_base64[] = "j9SEFxa5Q4rbHlqiwCr14p9KNQAuHFF5uItVzHy6lKI2iCptyU2yjWXHbIpgtN6Kah09OlA71sjqT1XN7HGULWa1MY/hIoEYJv3zAmW0gC9tvjCCj/4fwRmmnDGE4muycoDyIY3jenlrOPP3j3SoBIwTdVhLv1STorqkbL9KVe/s95i0awvLAZIzmvGqbDSQ3f2rxZFBJgTnbqHosx/ozFcXhKbw7yMBNFPeUCeEqbL2is1Nz09LhpKeTRiNK9OeSjYjOAkgrk/711bsH5yl/flKiS5G5MZogiCQ7Ai1TLMrgKlhklpX1meNVRfHRaQRhw5bhbSBqlKuSKedfu9iHQ==";
int main(int argc, char const *argv[])
{
    RSA *rsa = NULL;
    int rc = 0;
    rsa = find_pubkey1("rsa_public_web.pem");
    unsigned char data[100];

    unsigned char *signed_data = NULL;
    signed_data = malloc(RSA_size(rsa));



    //f中的n字节 解码到t
    rc = EVP_DecodeBlock(signed_data,signed_data_base64,strlen((char *)signed_data_base64));

    char reseult[1000] = {0};
    hexToString(signed_data,reseult,rc); 
    printf("%s\n",reseult);

    

    if(rc <= 0) {
        return -1;
    }
    //按照base64的算法，任何长度的串编码后长度均为4的倍数，解码后均未3的倍数。理论上，encode时，如果输入串不是3的倍数，会在后面补0,以保持3的倍数
    rc = RSA_public_decrypt(rc-2,signed_data,data,rsa,RSA_PKCS1_PADDING); //最后多了2个字节的0
    if(rc <= 0 ) {
        ERR_load_CRYPTO_strings();
        char buf[100] = {0};
        int e = ERR_get_error();
        ERR_error_string_n(e,buf,100);
        printf("%s\n",buf);
        return -1;
    }
    printf("%s\n",data);


    return 0;
}
