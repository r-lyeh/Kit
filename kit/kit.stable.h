#ifndef STABLE_H
#define STABLE_H

// stable pointers which can be compared for equality, aka interned strings
// original implementation by Per Vognsen: https://github.com/pervognsen/bitwise
// ideas and tests from RandyGaul.
//
// - Interned strings can be compared by pointer directly, instead of by string contents.
// - Interned strings can be put directly into hash tables as keys by casting them to uint64_t.
// - Reduced RAM consumption by compressing redundant strings down to a single copy.

const char* stable(const char* s);
const char* stable_len(const char* s, int len);
const char* stable_range(const char* start, const char* end);
void        stable_nuke(); // Frees all memory used by string interning so far. All prior strings are now invalid.

#elif KIT_CODE
#pragma once

typedef struct unique_string_t {
    size_t len;
    struct unique_string_t* next;
    char* str;
} unique_string_t;

map_(uint64_t,unique_string_t*) g_interned = {less64,hash64};

const char* stable_range(const char* start, const char* end) {
    size_t len = (size_t)(end - start);
    uint64_t key = memhash((void*)start, len);

    unique_string_t* head = map_get(g_interned, key);
    for( unique_string_t* it = head; it; it = it->next ) {
        if( it->len == len && memcmp(it->str, start, len) == 0 ) {
            return it->str;
        }
    }

    size_t bytes = sizeof(unique_string_t) + len + 1;
    unique_string_t* node = (unique_string_t*)malloc(bytes);
    node->len = len;
    node->next = head;
    node->str = (char*)(node + 1);
    memcpy(node->str, start, len);
    node->str[len] = '\0';
    map_add(g_interned, key, node);

    return node->str;
}
const char *stable_len(const char *s, int len) {
    return stable_range(s, s + len);
}
const char *stable(const char *s) {
    return stable_range(s, s + strlen(s));
}
void stable_nuke() {
    for( int i = 0; i < map_count(g_interned); ++i ) {
        unique_string_t* it = g_interned.values[i];
        while (it) {
            unique_string_t* next = it->next;
            free(it);
            it = next;
        }
    }
    map_free(g_interned);
}


AUTOTEST { // unit_test_intern

    // test suite extracted from https://github.com/RandyGaul/ckit.h (Public Domain)

    const char *a = stable("hello");
    const char *b = stable("he" "llo");
    const char *c = stable("world");
    test(a == b);
    test(a != c);
    test(strcmp(a, "hello") == 0);
    test(strcmp(c, "world") == 0);

    const char *hw = "helloworld";
    const char *h2 = stable_len(hw, 5); // "hello"
    test(h2 == a);

    // different content -> different pointer
    const char *d = stable("HELLO");
    test(d != a);

    // long/random-ish strings
    char buf[256];
    for (int i = 0; i < 200; ++i) {
        int n = snprintf(buf, sizeof(buf), "str-%d-%d-%d", i, i*i, 12345);
        (void)n;
        const char *s1 = stable(buf);
        const char *s2 = stable(buf);
        test(s1 == s2);
        test(strcmp(s1, buf) == 0);
    }
}

AUTOTEST { // demo
    const char *h = stable("hello");
    const char *w = stable("world");
    const char *H = stable("hello");

    test( h == H );
    test( h != w );
    test( h != "hello" );
    test(~puts("Ok"));
}

#endif
