#pragma once

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

#define round_down(a, n)              \
    ({                                \
        size_t __a = (size_t)(a);     \
        (__typeof__(a))(__a - __a % (n)); \
    })

#define round_up(a, n)                                       \
    ({                                                       \
        size_t __n = (size_t)n;                              \
        (__typeof__(a))(round_down((size_t)(a) + __n - 1, __n)); \
    })

