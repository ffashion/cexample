/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright(c) 2021 Sanpe <sanpeqf@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <err.h>
#include <fcntl.h>
#include <unistd.h>
#include <openssl/hmac.h>

#define __noreturn __attribute__((__noreturn__))

#define ARRAY_SIZE(arr) \
    (sizeof(arr) / sizeof((arr)[0]))

#define ROUND_UP(val, div) ({   \
    (((val) - 1) / (div) + 1);  \
})

#define SEED_SIZE 110
#define TEST_MODE 1

struct md_type {
    const char *name;
    const EVP_MD *(*fun)(void);
};

struct md_type md_types[] = {
    {"sha1", EVP_sha1},
    {"sha224", EVP_sha224},
    {"sha256", EVP_sha256},
    {"sha384", EVP_sha384},
    {"sha512", EVP_sha512},
    {"sha512_224", EVP_sha512_224},
    {"sha512_256", EVP_sha512_256},
    {"sha3_224", EVP_sha3_224},
    {"sha3_256", EVP_sha3_256},
    {"sha3_384", EVP_sha3_384},
    {"sha3_512", EVP_sha3_512},
    {"shake128", EVP_shake128},
    {"shake256", EVP_shake256},
#ifndef OPENSSL_NO_MDC2
    {"mdc2", EVP_mdc2},
#endif
    {"ripemd160", EVP_ripemd160},
    {"whirlpool", EVP_whirlpool},
    {"sm3", EVP_sm3},
#ifndef OPENSSL_NO_MD2
    {"md2", EVP_md2},
#endif
#ifndef OPENSSL_NO_MD4
    {"md4", EVP_md4},
#endif
#ifndef OPENSSL_NO_MD5
    {"md5", EVP_md5},
    {"md5_sha1", EVP_md5_sha1},
#endif
#ifndef OPENSSL_NO_BLAKE2
    {"blake2b512", EVP_blake2b512},
    {"blake2s256", EVP_blake2s256},
#endif
};

#if TEST_MODE
static void *get_random(unsigned long size)
{
    char *block, seed[] = "hello world";
    unsigned int count;

    block = malloc(ROUND_UP(size, sizeof(seed)) * sizeof(seed));
    if (!block)
        return NULL;

    for (count = 0; count < ROUND_UP(size, sizeof(seed)); ++count)
        strcpy(block + count * 11, seed);

    return block;
}
#else
static void *get_random(unsigned long size)
{
    int fd, ret;
    void *random;

    random = malloc(size);
    if (!random)
        return NULL;

    fd = open("/dev/random", O_RDONLY);
    if (fd < 0)
        goto error_free;

    ret = read(fd, random, size);
    if (ret < 0)
        goto error_close;

    close(fd);
    return random;

error_close:
    close(fd);
error_free:
    free(random);

    return NULL;
}
#endif

static int compute_prf(const EVP_MD *md, uint8_t **pout, unsigned int size, char *key)
{
    unsigned int klen, count, once, curr_size = SEED_SIZE;
    unsigned int hsize = EVP_MD_size(md);
    uint8_t *buff, *prf, *seed;

    if (!md || !pout || !size || !key)
        return -EINVAL;

    seed = get_random(SEED_SIZE);
    if (!seed)
        return -ENOMEM;

    buff = malloc(hsize + SEED_SIZE);
    if (!buff)
        goto free_seed;

    *pout = prf = malloc(size + hsize);
    if (!prf)
        goto free_buff;

    klen = strnlen(key, ~0U);
    memcpy(buff, seed, SEED_SIZE);

    for (count = 0; count < ROUND_UP(size, hsize); ++count) {
        HMAC(md, key, klen, buff, curr_size, buff, &curr_size);
        memcpy(buff + curr_size, seed, SEED_SIZE);

        HMAC(md, key, klen, buff, hsize + SEED_SIZE, prf, &once);
        prf += once;
    }

    free(buff);
    free(seed);
    return 0;

free_buff:
    free(buff);
free_seed:
    free(seed);

    return -ENOMEM;
}

static __noreturn void usage(void)
{
    unsigned int index;

    fprintf(stderr, "usage: rmdir [-lkt] ...\n");
    fprintf(stderr, "  -l  output length\n");
    fprintf(stderr, "  -k  key\n");
    fprintf(stderr, "  -t  message digest\n");

    fprintf(stderr, "supported message digest:\n");
    for (index = 0; index < ARRAY_SIZE(md_types); ++index)
        printf("  %s\n", md_types[index].name);

    exit(1);
}

static int htoa(uint8_t *hex, char *str, unsigned int len, bool output)
{
    unsigned int count;
    bool state;

    if ((state = !str) && !(str = malloc(2 * len)))
        return -ENOMEM;

    for (count = 0; count < len; ++count)
        sprintf(str + count * 2, "%02x", hex[count]);

    if (output)
        printf("%s\n", str);

    if (state)
        free(str);

    return 0;
}

static struct md_type *get_type(const char *name)
{
    unsigned int index;

    for (index = 0; index < ARRAY_SIZE(md_types); ++index)
        if (!strcasecmp(md_types[index].name, name))
            return &md_types[index];

    return md_types;
}

int main(int argc, char *const *argv)
{
    struct md_type *type = md_types;
    char arg, *key = "password";
    int ret, len = 1024;
    uint8_t *prf;

    while ((arg = getopt(argc, argv, "l:k:t:h-")) != -1) {
        switch (arg) {
            case 'l':
                len = atoi(optarg);
                break;
            case 'k':
                key = optarg;
                break;
            case 't':
                type = get_type(optarg);
                break;
            default:
                usage();
        }
    }

    printf("%s:%d:\"%s\"\n", type->name, len, key);

    if ((ret = compute_prf(type->fun(), &prf, len, key)))
        return ret;

    ret = htoa(prf, NULL, len, 1);

    free(prf);
    return ret;
}
