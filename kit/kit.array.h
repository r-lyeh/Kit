#ifndef array_
#define array_(t) t*
#define  array_push(t, ...) ( array_realloc_((t),array_count(t)+1), (t)[ array_count(t) - 1 ] = (__VA_ARGS__) )
#define  array_pushfront(arr,c) do { array_realloc_((arr),array_count(arr)+1); for( int ii = array_count(arr); --ii >= 1; ) arr[ii] = arr[ii-1]; 0[arr] = (c); } while(0)
#define   array_back(t) ( &(t)[ array_count(t)-1 ] )
#define   array_pop(t) ( array_c_ = array_count(t), array_c_ ? array_realloc_((t), array_c_-1) : array_cast_(t)(NULL), &(t)[array_c_-1] )
#define  array_resize(t, n) ( array_c_ = array_count(t), array_n_ = (n), array_realloc_((t),array_n_), (array_n_>array_c_? memset(array_c_+(t),0,(array_n_-array_c_)*sizeof(0[t])) : (void*)0), (t) )
#define  array_count(t) (int)( (t) ? ( vlen(t) - sizeof(0[t]) ) / sizeof(0[t]) : 0u )
#define  array_swap(t,i,j) ( array_push(t,0[t]), *array_back(t) = (i)[t], (i)[t] = (j)[t], (j)[t] = *array_back(t), array_pop(t) )
#define  array_delswap(t,i) ( array_c_ = array_count(t), array_c_ ? ((i)[t] = (t)[array_c_-1], array_pop(t)) : NULL )
#define  array_concat(a,str) do { const char *s_ = (str); int l_ = (int)strlen(str); int c_ = array_count(a); array_resize(a,c_+l_); memcpy(a+c_,s_,l_+1); } while(0)
#define  array_shuffle(arr, begin, end, randi_range_fn) for(int b_ = (begin), e_ = (end), i_ = b_; i_ != e_; ++i_) { int j_ = randi_range_fn(b_, e_); array_swap(arr, i_, j_); }
#define  array_clear(t) ( array_resize(t,0) ) // @fixme
#define array_free(t) ( array_realloc_((t),-1), (t) = 0 ) // -1
#define  array_cast_(x) ifdef(KIT_CPP,(decltype(&0[x])))(void *) // cpp: (decltype x)
#define  array_realloc_(t,n)  ( (t) = array_cast_(t) vrealloc((t), ((n)+1) * sizeof(0[t])) ) // +1
static   THREAD unsigned array_c_, array_n_;

#define array_sort(a,cmpfn) qsort((a), array_count(a), sizeof(0[a]), cmpfn)
#define array_sortstr(a)    array_sort(a, qstrcmp)
#define array_sortstri(a)   array_sort(a, qstrcmpi)
static int qstrcmp(const void *a, const void *b) { return strcmp(*(const char **)a, *(const char **)b); }
static int qstrcmpi(const void *a, const void *b) { return strcmpi(*(const char **)a, *(const char **)b); }

#define each_array(arr, idx) (int idx = 0, end_ = array_count(arr); idx < end_; ++idx)
#define each_array_random(arr, idx) \
    (int N = array_count(arr); N; N = 0) \
        for(int step = get_random_coprime_step(N), start = rand() % N; step; step = 0 ) \
            for(int k = 0, idx; (idx = (start + (long long)k * step) % N, k < N); ++k )

static int gcd(int a, int b) { // Euclidean algorithm to compute GCD
    while (b != 0) { int temp = b; b = a % b; a = temp; }
    return a;
}
static int get_random_coprime_step(int N) { // Generate a random coprime step size (biased toward larger values for better mixing)
    if (N <= 1) return 1;
    int step; do { step = (rand() % (N / 2)) + (N / 2); /*< [N/2..N] */ if (step == 0) step = 1; } while (gcd(step, N) != 1);
    return step;
}

#endif

#if KIT_CODE
#pragma once
AUTOTEST { // unit_test_array

    // test suite extracted from https://github.com/RandyGaul/ckit.h (Public Domain)

    // build
    array_(int) a = NULL;
    const int N = 1000;
    for (int i = 0; i < N; ++i) array_push(a, i);
    test(array_count(a) == N);
    for (int i = 0; i < N; ++i) test(a[i] == i);
    test(*array_back(a) == N - 1);
    test(array_back(a)+1 == a + N);

    // pop
    for (int i = N - 1; i >= 0; --i) {
        int x = *array_pop(a);
        test(x == i);
    }
    test(array_count(a) == 0);

    // reuse
    for (int i = 0; i < 32; ++i) array_push(a, i * 2);
    array_clear(a);
    test(array_count(a) == 0);

    for (int i = 0; i < 10; ++i) array_push(a, i); // [0..9]
    array_delswap(a, 3);
    test(array_count(a) == 9);
    test(a[3] == 9);

    array_free(a);
}
#endif
