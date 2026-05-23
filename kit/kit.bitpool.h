// [ref] https://jakubtomsu.github.io/posts/bit_pools/

#ifndef BITPOOL_H
#define BITPOOL_H

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#ifndef BITPOOL_CTZ64
    #if defined(__GNUC__) || defined(__clang__)
        #define BITPOOL_CTZ64(x) (x ? __builtin_ctzll(x) : 64)
    #elif defined(_MSC_VER)
        #include <immintrin.h>
        #define BITPOOL_CTZ64(x) (x ? _tzcnt_u64(x) : 64)
    #else
        static inline int BITPOOL_CTZ64(uint64_t x) {
            if( x ) {
            int n = 0;
            while (((x >> n) & 1ULL) == 0 && n < 64) n++;
            return n;
            } return 64;
        }
    #endif
#endif

#define BITPOOL_L0_SIZE(N) ((N) / 64)
#define BITPOOL_L1_SIZE(N) (((N) + 4095) / 4096)
#define BITPOOL_L2_SIZE(N) (((N) + 262143) / 262144)

// Declare a bitpool type
#define BITPOOL_DECLARE(name, N)                                       \
    typedef struct {                                                   \
        uint64_t l2[BITPOOL_L2_SIZE(N)];                               \
        uint64_t l1[BITPOOL_L1_SIZE(N)];                               \
        uint64_t l0[BITPOOL_L0_SIZE(N)];                               \
    } name;                                                            \
    bool name##_find0(const name *bp, unsigned *out_index);            \
    bool name##_alloc(name *bp, unsigned *out_index);                  \
    void name##_set(name *bp, unsigned index, bool on);                \
    bool name##_get(const name *bp, unsigned index);

// Define and expand functions for a bitpool type
#define BITPOOL_DEFINE(name, N)                                        \
    /* Find first zero bit */                                          \
    bool name##_find0(const name *bp, unsigned *out_index) {           \
        int l1_index = -1;                                             \
        int l0_index = -1;                                             \
                                                                       \
        if ((N) > 4096) {                                              \
            /* L2 > L1 */                                              \
            for (int i = 0; i < BITPOOL_L2_SIZE(N); i++) {             \
                uint64_t inv = ~bp->l2[i];                             \
                int l2_slot = BITPOOL_CTZ64(inv);                      \
                if (l2_slot != 64) {                                   \
                    l1_index = i * 64 + l2_slot;                       \
                    break;                                             \
                }                                                      \
            }                                                          \
        } else if ((N) > 64) {                                         \
            l1_index = 0;                                              \
        } else {                                                       \
            l0_index = 0;                                              \
        }                                                              \
                                                                       \
        if (l0_index == -1) {                                          \
            if (l1_index == -1 || l1_index >= BITPOOL_L1_SIZE(N))      \
                return false;                                          \
                                                                       \
            /* L1 > L0 */                                              \
            uint64_t inv = ~bp->l1[l1_index];                          \
            int l1_slot = BITPOOL_CTZ64(inv);                          \
            if (l1_slot == 64) return false;                           \
                                                                       \
            l0_index = l1_index * 64 + l1_slot;                        \
        }                                                              \
                                                                       \
        if (l0_index >= BITPOOL_L0_SIZE(N))                            \
            return false;                                              \
                                                                       \
        /* L0 > bit */                                                 \
        uint64_t inv = ~bp->l0[l0_index];                              \
        int l0_slot = BITPOOL_CTZ64(inv);                              \
                                                                       \
        if (l0_slot == 64) return false;                               \
                                                                       \
        *out_index = l0_index * 64 + l0_slot;                          \
        return true;                                                   \
    }                                                                  \
    /* Allocate (find + set) */                                        \
    bool name##_alloc(name *bp, unsigned *out_index) {                 \
        if (!name##_find0(bp, out_index))                              \
            return false;                                              \
        name##_set(bp, (uint64_t)*out_index, 1);                       \
        return true;                                                   \
    }                                                                  \
    /* Check bit */                                                    \
    bool name##_get(const name *bp, unsigned index) {                  \
        assert(index < (N));                                           \
                                                                       \
        uint64_t l0_index = index / 64;                                \
        uint64_t l0_slot  = index % 64;                                \
                                                                       \
        return (bp->l0[l0_index] & (1ULL << l0_slot)) != 0;            \
    }                                                                  \
    /* Set bit */                                                      \
    void name##_set(name *bp, unsigned index, bool on) {               \
        assert(index < (N));                                           \
                                                                       \
        uint64_t l0_index = index / 64;                                \
        uint64_t l0_slot  = index % 64;                                \
                                                                       \
        uint64_t l1_index = l0_index / 64;                             \
        uint64_t l1_slot  = l0_index % 64;                             \
                                                                       \
        uint64_t l2_index = l1_index / 64;                             \
        uint64_t l2_slot  = l1_index % 64;                             \
                                                                       \
        if( on ) {                                                     \
            uint64_t bucket = bp->l0[l0_index] | (1ULL << l0_slot);    \
            bp->l0[l0_index] = bucket;                                 \
                                                                       \
            if( bucket == UINT64_MAX ) {                               \
                uint64_t l1_bucket = bp->l1[l1_index] | (1ULL << l1_slot); \
                bp->l1[l1_index] = l1_bucket;                          \
                                                                       \
                if( l1_bucket == UINT64_MAX ) {                        \
                    bp->l2[l2_index] |= (1ULL << l2_slot);             \
                }                                                      \
            }                                                          \
            return;                                                    \
        }                                                              \
                                                                       \
        bp->l0[l0_index] &= ~(1ULL << l0_slot);                        \
                                                                       \
        /* pessimistic clear upward (simple + correct) */              \
        bp->l1[l1_index] &= ~(1ULL << l1_slot);                        \
        bp->l2[l2_index] &= ~(1ULL << l2_slot);                        \
    }


// Declare pool types with different sizes (up to 262144 entries max)

BITPOOL_DECLARE(bitpool1K, 1024)
BITPOOL_DECLARE(bitpool2K, 2048)
BITPOOL_DECLARE(bitpool4K, 4096)
BITPOOL_DECLARE(bitpool8K, 8192)
BITPOOL_DECLARE(bitpool16K, 16384)
BITPOOL_DECLARE(bitpool32K, 32768)
BITPOOL_DECLARE(bitpool64K, 65536)
BITPOOL_DECLARE(bitpool128K, 131072)
BITPOOL_DECLARE(bitpool256K, 262144)

#elif KIT_CODE
#pragma once

// impl

// Expand implementations for the different pool types we declared

BITPOOL_DEFINE(bitpool1K, 1024)
BITPOOL_DEFINE(bitpool2K, 2048)
BITPOOL_DEFINE(bitpool4K, 4096)
BITPOOL_DEFINE(bitpool8K, 8192)
BITPOOL_DEFINE(bitpool16K, 16384)
BITPOOL_DEFINE(bitpool32K, 32768)
BITPOOL_DEFINE(bitpool64K, 65536)
BITPOOL_DEFINE(bitpool128K, 131072)
BITPOOL_DEFINE(bitpool256K, 262144)

// demo
AUTOTEST {
    // Initialize 1K pool (bits == 0 are free to use)
    bitpool1K pool = {0};

    unsigned index;

    // Allocate a few slots
    for (int i = 0; i < 5; i++) {
        if (bitpool1K_alloc(&pool, &index)) {
            printf("Allocated index: %d\n", index);
        } else {
            printf("Pool full!\n");
        }
    }

    // Check a bit
    if (bitpool1K_get(&pool, 2)) {
        printf("Index 2 is allocated\n");
    }

    // Free a bit
    bitpool1K_set(&pool, 2, 0);
    printf("Freed index 2\n");

    // Allocate again (should reuse index 2)
    if (bitpool1K_alloc(&pool, &index)) {
        printf("Allocated index: %d\n", index);
    }
}

#endif
