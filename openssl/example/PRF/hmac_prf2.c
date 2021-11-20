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

#define ROUND_UP(val, div) ({   \
    (((val) - 1) / (div) + 1);  \
})

#define HASH_SIZE 16
#define SEED_SIZE 110
#define TEST_MODE 1

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
    int ramdom, ret;
    void *block;

    block = malloc(size);
    if (!block)
        return NULL;

    ramdom = open("/dev/random", O_RDONLY);
    if (ramdom < 0)
        goto error_free;

    ret = read(ramdom, block, size);
    if (ret < 0)
        goto error_close;

    close(ramdom);
    return block;

error_close:
    close(ramdom);
error_free:
    free(block);

    return NULL;
}
#endif

int compute_prf(unsigned int out_size, uint8_t **pout)
{
    unsigned int count, curr_size = SEED_SIZE;
    uint8_t *buff, *prf, *seed;

    if (!out_size || !pout)
        return -EINVAL;

    seed = get_random(SEED_SIZE);
    if (!seed)
        return -ENOMEM;

    buff = malloc(HASH_SIZE + SEED_SIZE);
    if (!buff)
        goto free_seed;

    *pout = prf = malloc(out_size + HASH_SIZE);
    if (!prf)
        goto free_buff;

    memcpy(buff, seed, SEED_SIZE);

    for (count = 0; count < ROUND_UP(out_size, HASH_SIZE); ++count) {
        unsigned int once_size;

        HMAC(EVP_md5(), "password", 8, buff, curr_size, buff, &curr_size);
        memcpy(buff + curr_size, seed, SEED_SIZE);

        HMAC(EVP_md5(), "password", 8, buff, HASH_SIZE + SEED_SIZE, prf, &once_size);
        prf += once_size;
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

int main(int argc, const char *argv[])
{
    uint8_t *data;
    int len, ret = 0;

    len = atoi(argv[1]);
    if ((ret = compute_prf(len, &data)))
        return ret;

    ret = htoa(data, NULL, len, 1);

    free(data);
    return ret;
}
