#ifndef HASH_H
#define HASH_H

// -----------------------------------------------------------------------------
// sort/less

int less(const void *a, const void *b);
int lessf(const void *a, const void *b);
int less32(const void *a, const void *b);
int less64(const void *a, const void *b);

// -----------------------------------------------------------------------------
// un/hash

uint64_t hash(const void *str);
uint64_t hashf(const void *f);
uint64_t hashd(const void *d);
uint64_t hash32(const void *i);
uint64_t hash64(const void *I);

uint64_t unhash32(const void *i);
uint64_t unhash64(const void *I);
uint64_t memhash(const void *ptr, unsigned sz);

#elif KIT_CODE
#pragma once

// -----------------------------------------------------------------------------
// sort/less

int less(const void *a, const void *b) {
    return strcmp((const char *)a, (const char *)b);
}

int lessf(const void *a, const void *b) {
    float va = *(const float *)a, vb = *(const float *)b;
    return (va < vb) ? -1 : (vb < va) ? 1 : 0; // return va > vb ? +1 : -!!(va - vb);
}

int less32(const void *a, const void *b) {
    int va = *(const int *)a, vb = *(const int *)b;
    return (va < vb) ? -1 : (vb < va) ? 1 : 0; // return va > vb ? +1 : -!!(va - vb);
}

int less64(const void *a, const void *b) {
    int64_t va = *(const int64_t *)a, vb = *(const int64_t *)b;
    return (va < vb) ? -1 : (vb < va) ? 1 : 0; // return va > vb ? +1 : -!!(va - vb);
}

// -----------------------------------------------------------------------------
// un/hash

uint64_t hash(const void* str) { // fnv1a
    uint64_t x = 0xcbf29ce484222325ull;
    for (unsigned i = 0; ((char*)str)[i]; i++) {
        x ^= (uint64_t)((char*)str)[i];
        x *= 0x100000001b3ull;
        x ^= x >> 32;
    }
    return x;
}

uint64_t hashf(const void *f) {
    float x = *(float*)f;
    union { float d; uint32_t i; } c;
    return c.d = x, hash32(&c.i);
}

uint64_t hashd(const void *d) {
    double x = *(double*)d;
    union { double d; uint64_t i; } c;
    return c.d = x, hash64(&c.i);
}

uint64_t hash32(const void *k) { // Thomas Mueller at https://stackoverflow.com/questions/664014/ - says no collisions for 32bits!
    uint32_t x = *(uint32_t*)k;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

uint64_t hash64(const void *k) {
    uint64_t x = *(uint64_t*)k;
    uint32_t hi = x >> 32ull, lo = x & ~0u;
    return (hash32(&hi) << 32ull) | hash32(&lo);
}

uint64_t unhash32(const void *k) { // Thomas Mueller at https://stackoverflow.com/questions/664014/ - says no collisions for 32bits!
    uint32_t x = *(uint32_t*)k;
    x = ((x >> 16) ^ x) * 0x119de1f3;
    x = ((x >> 16) ^ x) * 0x119de1f3;
    x = (x >> 16) ^ x;
    return x;
}

uint64_t unhash64(const void *k) { // @todo: testme
    uint64_t x = *(uint64_t*)k;
    uint32_t hi = x >> 32ull, lo = x & ~0u;
    return (unhash32(&hi) << 32ull) | unhash32(&lo);
}

uint64_t memhash(const void* ptr, unsigned sz) { // fnv1a
    const char* buf = (const char*)ptr;
    uint64_t x = 0xcbf29ce484222325ull;
    for (unsigned i = 0; i < sz; i++) {
        x ^= (uint64_t)buf[i];
        x *= 0x100000001b3ull;
        x ^= x >> 32;
    }
    return x;
}

#endif
