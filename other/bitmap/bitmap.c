/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright(c) 2021 Sanpe <sanpeqf@gmail.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define TEST_LOOP   100
#define TEST_ARRAY  4096

#define ___stringify(x...)      #x
#define __stringify(x...)       ___stringify(x)

#define __ASM_FORM(x)           " " __stringify(x) " "
#define __ASM_FORM_RAW(x)       __stringify(x)
#define __ASM_FORM_COMMA(x)     " " __stringify(x) ","

#ifdef __i386__
# define __ASM_SEL(a, b)        __ASM_FORM(a)
# define __ASM_SEL_RAW(a, b)    __ASM_FORM_RAW(a)
#else
# define __ASM_SEL(a, b)        __ASM_FORM(b)
# define __ASM_SEL_RAW(a, b)    __ASM_FORM_RAW(b)
#endif

#define __ASM_SIZE(inst, ...)   __ASM_SEL(inst##l##__VA_ARGS__, inst##q##__VA_ARGS__)
#define __ASM_REG(reg)          __ASM_SEL_RAW(e##reg, r##reg)

#ifdef __GCC_ASM_FLAG_OUTPUTS__
# define CC_SET(c) "\n\t/* output condition code " #c "*/\n"
# define CC_OUT(c) "=@cc" #c
#else
# define CC_SET(c) "\n\tset" #c " %[_cc_" #c "]\n"
# define CC_OUT(c) [_cc_ ## c] "=qm"
#endif

#define RLONG_ADDR(x)               "m" (*(volatile long *) (x))
#define ADDR(addr)                  RLONG_ADDR(addr)

static void bit_set(volatile unsigned long *addr, long bit)
{
    asm volatile (
        __ASM_SIZE(bts) " %1,%0"
        : : ADDR(addr), "Ir" (bit)
        : "memory"
    );
}

static bool bit_test(volatile unsigned long *addr, long bit)
{
    bool oldbit;

    asm volatile (
        __ASM_SIZE(bt) " %2,%1"
        CC_SET(c)
        : CC_OUT(c) (oldbit)
        : ADDR(addr), "Ir"(bit)
        : "memory"
    );

    return oldbit;
}

void main(void)
{
    unsigned long array[TEST_ARRAY / (__SIZEOF_POINTER__ * 8)];
    unsigned int count;
    int bit;

    memset(array, 0, TEST_ARRAY / (__SIZEOF_POINTER__ * 8));

    for (count = 0; count < TEST_ARRAY; ++count) {
        bit = rand();
        bit %= TEST_ARRAY;
        bit_set(array, bit);
        printf("bitset (%d):\t", bit);
        if (!bit_test(array, bit)) {
            printf("fault\n", bit);
            return;
        }
        printf("pass\n", bit);
    }
}
