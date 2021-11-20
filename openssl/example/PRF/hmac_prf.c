#include <openssl/hmac.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define A_CRYPTO_MAX_MD_LEN 64
char helloworld[] = "hello world";

void set_seed(char *seed) {
  for (int i = 0; i < 10; i++) {
    strcpy(seed + i * 11, helloworld);
  }
  return;
}

int round_up(int x, int y) {
    // return x % y == 0 ? x /y : x /y + 1;
    // return (x + y - 1)  / y;
    return (x - 1 )/ y + 1;
}

void hexToString(unsigned char *hex, char *str, int len, int output) {
  if (str == NULL) {
    str = malloc(2 * len) ;
  }
  for (size_t i = 0; i < len; i++) {
    sprintf(str + i * 2, "%02x", hex[i]);
  }
  if (output) {
    printf("%s\n", str);
  }
  free(str);
  return;
}

uint32_t phash(const EVP_MD *op, unsigned char *secret, int secret_len,
               char *seed, uint32_t seed_len, char *prf, uint32_t olen) {
  int hash_size = 16;
  char *p = NULL, A1[A_CRYPTO_MAX_MD_LEN];
  uint32_t ret;

  p = malloc(seed_len + hash_size);

  HMAC(op, secret, secret_len, seed, seed_len, A1, NULL);

  for (;;) {
    memcpy(p, A1, hash_size);
    memcpy(p + hash_size, seed, seed_len);

    if (olen > hash_size) {
      HMAC(op, secret, secret_len, p, seed_len + hash_size, prf,
           NULL);  //  prf = HMAC(sec,HMAC(sec,seed),seed)

      prf += hash_size;
      olen -= hash_size;

      HMAC(op, secret, secret_len, A1, hash_size, A1, NULL);

    } else {  //最后一次 不够长度的HMAC
      HMAC(op, secret, secret_len, p, seed_len + hash_size, A1, NULL);

      memcpy(prf, A1, olen);
      break;
    }
  }

  free(p);
}

int compute_prf2() {
  char *seed = NULL;
  int seed_size = 0;
  seed = malloc(strlen(helloworld) * 10);
  set_seed(seed);
  seed_size = strlen(seed);
  char *prf = malloc(1024);
  phash(EVP_md5(), "password", 8, seed, seed_size, prf, 1024);
  hexToString(prf, NULL, 1024, 1);
}


int compute_prf(int wanted_output_size) {
  char *seed = NULL;
  char *prf = NULL;
  char *current_prf = NULL;
  int seed_size = 0;
  int hash_size = 16;
  

  seed = malloc(strlen(helloworld) * 10);

  prf = malloc(wanted_output_size + hash_size);

  set_seed(seed);
  seed_size = strlen(seed);

  char *previous_md = malloc(hash_size + seed_size);
  int previous_md_size = 0;

  char *current_md = malloc(hash_size + seed_size);
  int current_md_size = 0;

  current_prf = prf;

  strcpy(previous_md, seed);
  previous_md_size = seed_size;
  for (int i = 0; i < round_up(wanted_output_size, hash_size); i++) {
    int prf_once_size = 0;
    HMAC(EVP_md5(), "password", 8, previous_md, previous_md_size, current_md,
         &current_md_size);

    memcpy(current_md + current_md_size, seed, seed_size);
    HMAC(EVP_md5(), "password", 8, current_md, hash_size + seed_size,
         current_prf,
         &prf_once_size);  // current_prf = HMAC(sec,HMAC(sec,seed))
    current_prf += prf_once_size;

    //为下一个循环准备
    memcpy(previous_md, current_md, current_md_size);
    previous_md_size = current_md_size;
  }

  hexToString(prf, NULL, wanted_output_size, 1);

  free(seed);
  free(prf);
  free(previous_md);
  free(current_md);
  return 0;
}

int main(int argc, char const *argv[]) { 
    compute_prf(2);
    compute_prf2();

}
