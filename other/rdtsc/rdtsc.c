/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright(c) 2021 Sanpe <sanpeqf@gmail.com>
 */

#include <stdio.h>
#include <stdint.h>

#ifdef __i386__
# define DECLARE_ARGS(val, low, high)   unsigned long long val
# define EAX_EDX_VAL(val, low, high)    (val)
# define EAX_EDX_RET(val, low, high)    "=A" (val)
#else
# define DECLARE_ARGS(val, low, high)   unsigned long low, high
# define EAX_EDX_VAL(val, low, high)    ((low) | (high) << 32)
# define EAX_EDX_RET(val, low, high)    "=a" (low), "=d" (high)
#endif

static inline uint64_t tsc_get(void)
{
    DECLARE_ARGS(val, low, high);

    asm volatile (
        "rdtsc\n"
        : EAX_EDX_RET(val, low, high)
    );

    return EAX_EDX_VAL(val, low, high);
}

void main(void)
{
    uint64_t time;
    time = tsc_get();
    printf("0x%016llx\n", time);
}
