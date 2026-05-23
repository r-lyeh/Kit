// ----------------------------------------------------------------------------
// macros in alphabetical order

#define THREAD __declspec(thread)

#ifdef __cplusplus
#define KIT_CPP 1
#else
#define KIT_CPP 0
#endif

#define KIT_WINDOWS 1
#define KIT_BSD 0
#define KIT_ANDROID 0
#define KIT_LINUX 0
#define KIT_MACOS 0

#define ifdef(arg, then, /*else*/...)        ifd3f(arg, then, __VA_ARGS__)
#define ifd3f(arg, then, /*else*/...)        ifdef_##arg(then, ##__VA_ARGS__)

#define ifndef(arg, then, /*else*/...)       ifnd3f(arg, then, __VA_ARGS__)
#define ifnd3f(arg, then, /*else*/...)       ifdef_##arg(__VA_ARGS__, then)

#define ifdef_1(then, /*else*/...)           then
#define ifdef_0(then, /*else*/...)           __VA_ARGS__

#define ifdef_true(then, /*else*/...)        then
#define ifdef_false(then, /*else*/...)       __VA_ARGS__

// ----------------------------------------------------------------------------
// allocators

#include ifdef(KIT_BSD, <malloc/malloc.h>, <malloc.h>) // freebsd: <malloc_np.h>
ifdef(KIT_ANDROID, size_t dlmalloc_usable_size(void*));

static void*  (*sys_realloc)(void*, size_t) = realloc;
static size_t (*sys_msize)(void *) = ifdef(KIT_BSD, (size_t (*)(void *))malloc_size, ifdef(KIT_WINDOWS, _msize, ifdef(KIT_ANDROID, dlmalloc_usable_size, malloc_usable_size)));

void* zrealloc(void* p, size_t siz) {
    if( siz != ~0u ) {
        if( siz > 0 ) {
            size_t old = p ? sys_msize(p) : 0u;
            if( siz > old ) {
                size_t diff = siz - old;
                p = sys_realloc(p, siz);
                if( p ) {
                    memset((char*)p + siz - diff, 0, diff);
                } else {
                    die("!Error: no memory enough.");
                }
                return p;
            }
        }
        return sys_realloc(p, siz);
    }
    return (void*)(uintptr_t)sys_msize(p);
}

// ----------------------------------------------------------------------------

void* vrealloc(void* p, size_t sz);
size_t vlen( void* p );

#define strdup(s)         (strcpy(vrealloc(0,strlen(s)+1),s))
#define free(p)           (void)vrealloc((p), 0)
#define malloc(sz)        vrealloc(0, (sz))
#define calloc(n,sz)      vrealloc(0,(n)*(sz)) // no need to memset(malloc(0,n*s),0,n*s) since vrealloc>zrealloc does it for us
#define realloc           vrealloc

// memory leaks ----------------------------------------------------------------

#if REPORT_MEMORY_LEAKS
#define vrealloc(ptr,len) memleak(vrealloc(memleak(ptr,0,0,0,0),len),len,__func__,__FILE__,__LINE__)
void* memleak(void *ptr, int len, const char *func, const char *file, int line);
#endif

#if REPORT_MEMORY_LEAKS && KIT_CODE
void  memreport(void) { system(KIT_WINDOWS ? 
    "type 0*.mem 2> nul       > report.mem && del 0*.mem 2> nul       && type report.mem | sort && find /V /C \"\" report.mem" :
    "cat 0*.mem  2> /dev/null > report.mem ;  rm  0*.mem 2> /dev/null ;  cat  report.mem | sort ;  wc -l report.mem"
); }
void* memleak(void *ptr, int len, const char *func, const char *file, int line) {
    ONCE system(KIT_WINDOWS ? "del *.mem 2> nul" : "rm *.mem 2> /dev/null"), atexit(memreport);
    char n[sizeof(void*)*2+8] = {0}; sprintf(n,"%p.mem",ptr);
    for(FILE *fp = line ? fopen(n,"wb") : 0; fp; fprintf(fp, "%8d bytes, %-20s, %s:%d\n", len,func,file,line), fclose(fp), fp = 0) {}
    return line ? ptr : (unlink(n), ptr);
}
#endif

void* vrealloc(void* p, size_t sz) {
    size_t *ret;
    if( !sz ) {
        if( p ) {
            ret = (size_t*)p - 2;
            ret[0] = 0;
            ret[1] = 0;
            ret = (size_t*)zrealloc( ret, 0 );
        }
        return 0;
    } else {
        if( !p ) {
            ret = (size_t*)zrealloc( 0, sizeof(size_t) * 2 + sz );
            ret[0] = sz;
            ret[1] = 0;
        } else {
            ret = (size_t*)p - 2;
            size_t osz = ret[0], cap = ret[1]; // original size and capacity
            if( sz == (size_t)-1 ) return (void*)osz;
            if( sz <= (osz + cap) ) {
                ret[0] = sz;
                ret[1] = cap - (sz - osz);
            } else {
                ret = (size_t*)zrealloc( ret, sizeof(size_t) * 2 + sz * 1.75 );
                ret[0] = sz;
                ret[1] = (size_t)(sz * 1.75) - sz;
            }
        }
        return &ret[2];
    }
}
size_t vlen( void* p ) {
    return p ? (size_t)vrealloc( p, (size_t)-1 ) : 0;
}


// ----------------------------------------------------------------------------
// containers in alphabetical order

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
                if( opcode & 4 ) array_pop(*b), array_pop(*b), M->deletes++;
                return (void*)-1;
            }
        }
    }
    return 0;
}

