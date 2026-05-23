#ifndef MAP_H
#define MAP_H

#define map_(K,V)       struct { struct mapbase_ M; K tmpk; V empty; V *found; array_(K) keys; array_(V) values; }
#define  map_count(m)   (array_count((m).keys) - (m).M.deletes)
#define  map_find_or_add(m,k,v) (map_find(m,k), (m).found ? (m).found : map_add(m,k,v))
#define  map_find(m,k)  ((m).tmpk = (k), (m).found = mapbase_opcode(&(m).M,&(m).tmpk,0,(m).keys,(m).values,sizeof (m).tmpk,sizeof (m).empty,1))
#define  map_add(m,k,v) (map_find(m,k), (m).found ? (*(m).found = (v), (m).found) : (mapbase_insert(&(m).M,&(m).tmpk,array_count((m).values)), array_push((m).keys,(m).tmpk), array_push((m).values,v), (m).found = array_back((m).values)))
#define   map_get(m,k)  (map_find(m,k), (m).found ? *(m).found : (m).empty)
#define   map_del(m,k)  ((m).tmpk = (k), !!mapbase_opcode(&(m).M,&(m).tmpk,0,(m).keys,(m).values,sizeof (m).tmpk,sizeof (m).empty,2|4))
#define   map_swap(m,i,j) (!!mapbase_opcode(&(m).M,&(m).keys[i],&(m).keys[j],(m).keys,(m).values,sizeof (m).tmpk,sizeof (m).empty,2) ? (array_swap(m.keys,i,j),array_swap(m.values,i,j),1) : 0)
#define   map_sort_ptr(m,fn) do { for( int i, j = map_count(m), f = 1; f; --j ) for( f = 0, i = 1; i < j; i++ ) if( fn(&(m).keys[i-1], &(m).keys[i]) > 0 ) f = 1, map_swap((m), i-1, i); } while(0) // @todo: qsort()
#define   map_sort_ref(m,fn) do { for( int i, j = map_count(m), f = 1; f; --j ) for( f = 0, i = 1; i < j; i++ ) if( fn( (m).keys[i-1],  (m).keys[i]) > 0 ) f = 1, map_swap((m), i-1, i); } while(0) // @todo: qsort()
#define  map_clear(m)   map_free(m) // (mapbase_free(&(m).M, 1), array_clear((m).keys), array_clear((m).values), 1)
#define map_free(m)     (mapbase_free(&(m).M, 0), array_free((m).keys), array_free((m).values), 1)

#define each_map(m,i)   (int i = 0, end_ = array_count((m).keys); i < end_; ++i)

#define set_(K)         map_(K,bool)
#define  set_count(s)   map_count(s)
#define  set_find(s,k)  map_find(s,k)
#define  set_add(s,k)   map_add(s,k,1)
#define  set_del(s,k)   map_del(s,k)
#define  set_clear(s)   map_clear(s)
#define set_free(s)     map_free(s)

struct mapbase_ {
    int (*less)(const void *k1, const void *k2);  // must be configured
    uint64_t (*hash)(const void *k);              // must be configured
    int num_buckets;                              // optional. must be pow2-1. eg, [3, 7, 15, 31, 63, 127, 511, 1023, 2047]
    array_(uint64_t) buckets[2048];               // each bucket is an array of tuples: {hash,index},{hash,index}...
    int deletes;
};

static inline void mapbase_free(struct mapbase_ *M, int keep) {
    for( int i = 0; i < sizeof(M->buckets)/sizeof(M->buckets[0]); ++i )
        if( keep ) array_clear(M->buckets[i]); else array_free(M->buckets[i]);
    M->deletes = 0;
}

static inline void mapbase_insert(struct mapbase_ *M,void *k,uint64_t index) {
    uint64_t hash = M->hash(k);
    array_(uint64_t) *bucket = &M->buckets[ hash & (M->num_buckets + 255 * !M->num_buckets) ];
    array_push(*bucket, hash);
    array_push(*bucket, index);
}

static inline void* mapbase_opcode(struct mapbase_ *M, void *k1, void *k2, void *keys, void *values, size_t key_size, size_t value_size, int opcode) {
    uint64_t hash = M->hash(k1);
    array_(uint64_t) *b = &M->buckets[ hash & (M->num_buckets + 255 * !M->num_buckets) ];

    for( int i = 0, end = array_count(*b); i < end; i += 2 ) {
        uint64_t hashed = (*b)[i];
        if( hash == hashed ) { // if hash matches...
            uint64_t index = (*b)[i + 1];
            void *stored_key = (char*)keys + index * key_size;
            if( 0 == M->less(k1, stored_key) ) { // ...and key matches too
                // either find opcode...
                if( opcode & 1 ) return (char*)values + index * value_size;
                if( opcode & 2 ) if( k2 ) {
                    uint64_t hash2 = M->hash(k2);
                    array_(uint64_t) *b2 = &M->buckets[ hash2 & (M->num_buckets + 255 * !M->num_buckets) ];
                    for( int j = 0, jend = array_count(*b2); j < jend; j += 2 ) {
                        uint64_t hashed2 = (*b2)[j];
                        if( hash2 == hashed2 ) { // if hash matches...
                            uint64_t index2 = (*b2)[j + 1];
                            void *stored_key2 = (char*)keys + index2 * key_size;
                            if( 0 == M->less(k2, stored_key2) ) { // ...and key matches too
                                //(*b2)[j] = (*b)[i]; (*b)[i] = hashed2;
                                (*b2)[j+1] = (*b)[i+1]; (*b)[i+1] = index2;
                                return (void*)-1;
                            }
                        }
                    }
                }
                // ...or swap opcode
                if( opcode & 2 ) if(!k2) (*b)[i] = (*b)[end-2], (*b)[i+1] = (*b)[end-1]; 
                // ...or destroy opcode
                if( opcode & 4 ) (void)array_pop(*b), (void)array_pop(*b), M->deletes++;
                return (void*)-1;
            }
        }
    }
    return 0;
}

#elif KIT_CODE
#pragma once

AUTOTEST { // unit_test_map_basic

    // test suite extracted from https://github.com/RandyGaul/ckit.h (Public Domain)

    map_(int64_t,int64_t) m = { less64,hash64 };
    test(map_get(m, 1234) == 0);

    const int N = 2000;
    for (uint64_t i = 0; i < (uint64_t)N; ++i) {
        uint64_t key = (i % 2 == 0) ? i : (i * 2654435761u); // mix to create dispersion
        uint64_t val = key * 3u + 1u;
        map_add(m, key, val);
        uint64_t got = map_get(m, key);
        test(got == val);
    }
    test(map_count(m) >= (N > 0 ? 1 : 0));

    // update existing
    for (uint64_t i = 0; i < (uint64_t)N; i += 5) {
        uint64_t key = (i % 2 == 0) ? i : (i * 2654435761u);
        uint64_t val2 = (key * 7u) ^ 0xBEEF;
        map_add(m, key, val2);
        test(map_get(m, key) == val2);
    }

    // verify all present
    for (uint64_t i = 0; i < (uint64_t)N; ++i) {
        uint64_t key = (i % 2 == 0) ? i : (i * 2654435761u);
        uint64_t v = map_get(m, key);
        uint64_t updated = (i % 5 == 0);
        uint64_t expect = updated ? ((key * 7u) ^ 0xBEEF) : (key * 3u + 1u);
        test(v == expect);
    }

    // test delete a subset
    int deleted = 0;
    for (uint64_t i = 0; i < (uint64_t)N; i += 3) {
        uint64_t key = (i % 2 == 0) ? i : (i * 2654435761u);
        int ok = map_del(m, key);
        if (ok) {
            ++deleted;
            test(map_get(m, key) == 0);
        }
    }
    test(map_count(m) == N - deleted);

    // remaining still fetch
    for (uint64_t i = 0; i < (uint64_t)N; ++i) {
        uint64_t key = (i % 2 == 0) ? i : (i * 2654435761u);
        uint64_t v = map_get(m, key);
        if (i % 3 == 0) {
            test(v == 0);
        } else {
            uint64_t updated = (i % 5 == 0);
            uint64_t expect = updated ? ((key * 7u) ^ 0xBEEF) : (key * 3u + 1u);
            test(v == expect);
        }
    }

    // clear and reuse
    map_clear(m);
    test(map_count(m) == 0);
    for (int i = 0; i < 100; ++i) map_add(m, (uint64_t)i, (uint64_t)(i + 42));
    for (int i = 0; i < 100; ++i) test(map_get(m, (uint64_t)i) == (uint64_t)(i + 42));

    map_free(m);
}

AUTOTEST { // unit_test_map_swap

    // test suite extracted from https://github.com/RandyGaul/ckit.h (Public Domain)

    map_(int,int) m = { less32,hash32 };

    // Insert predictable keys/vals so we can track by index.
    const int N = 256;
    for (int i = 0; i < N; ++i) {
        uint64_t key = (uint64_t)(i + 1000);
        uint64_t val = (uint64_t)(key * 11 + 7);
        map_add(m, key, val);
    }
    test(map_count(m) == N);

    // grab two indices to swap
    int i = 3, j = 200;
    uint64_t ki = m.keys[i], kj = m.keys[j];
    uint64_t vi = map_get(m, ki), vj = map_get(m, kj);

    // swap items (O(1) expected)
    map_swap(m, i, j);

    // keys in dense arrays should have swapped
    test(m.keys[i] == kj && m.keys[j] == ki);
    test(m.values[i] == vj && m.values[j] == vi);

    // lookups by key must still return the correct value
    test(map_get(m, ki) == vi);
    test(map_get(m, kj) == vj);

    // Do a bunch of random swaps; ensure lookups are consistent.
    for (int t = 0; t < 1000; ++t) {
        int a = (t * 17) & (N - 1);
        int b = (t * 113 + 7) & (N - 1);
        if (a == b) continue;
        uint64_t ka = m.keys[a], kb = m.keys[b];
        uint64_t va = map_get(m, ka), vb = map_get(m, kb);
        map_swap(m, a, b);
        test(map_get(m, ka) == va);
        test(map_get(m, kb) == vb);
    }

    map_free(m);
}

AUTOTEST { // unit_test_ssort

    // test suite extracted from https://github.com/RandyGaul/ckit.h (Public Domain)

    map_(const char *,uint64_t) m = { less,hash };
    const char* keys[] = { "Banana", "apple", "carrot", "Apple", "banana" };

    for (int i = 0; i < sizeof(keys)/sizeof(keys[0]); ++i) {
        const char* stable(const char*);
        const char* s = stable(keys[i]);
        map_add(m, s, i);
    }

    for (int i = 0, end = sizeof(keys)/sizeof(keys[0]); i < end; ++i) {
        printf("%s=%d%s", m.keys[i], (int)m.values[i], i == end-1 ? "\n":",");
    }

    // Sort case sensitive.
    map_sort_ref(m,strcmp);

    for (int i = 1; i < map_count(m); ++i) {
        const char* prev = m.keys[i - 1];
        const char* curr = m.keys[i];
        test(strcmp(prev, curr) <= 0);
    }

    for (int i = 0, end = sizeof(keys)/sizeof(keys[0]); i < end; ++i) {
        printf("%s=%d%s", m.keys[i], (int)m.values[i], i == end-1 ? "\n":",");
    }

    map_free(m);
}

AUTOTEST {
    map_(int, int) m = {less32, hash32};

    test(map_count(m) == 0);
    test(map_find(m,1) == 0);
    test(map_find(m,5) == 0);
    test(map_find(m,-1) == 0);
    test(map_count(m) == 0);

    test(map_add(m,1,10));
    test(map_get(m,1) == 10);
    test(map_count(m) == 1);

    test(map_add(m,2,21));
    test(map_get(m,2) == 21);
    test(map_count(m) == 2);

    test(map_add(m,2,20)); // update existing
    test(map_get(m,2) == 20);
    test(map_count(m) == 2);

    test(map_find_or_add(m,3,30));
    test(map_get(m,3) == 30);
    test(map_count(m) == 3);

    test(map_find_or_add(m,3,50)); // do not update existing
    test(map_get(m,3) == 30);
    test(map_count(m) == 3);

    // iterate
    for( int i = 0; i < map_count(m); ++i ) {
        // printf("[%d]=%d\n", m.keys[i], m.values[i]);
        test(m.keys[i] == (1+i));
        test(m.values[i] == (1+i)*10);
        test(map_get(m,i+1) == (1+i)*10);
    }

    map_free(m);
    test(map_count(m) == 0);
    test(map_find(m,0) == 0);
    test(map_find(m,1) == 0);
    test(map_find(m,2) == 0);
    test(map_find(m,3) == 0);

    map_free(m);
    test(map_count(m) == 0);

    map_(float, double) n = {lessf, hashf, 511};
}

#endif
