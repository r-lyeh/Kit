#ifndef AUTORUN // execute following global scope before main() is reached
#define AUTORUN AUTORUN_( UNIQUE(fn) )
#ifdef __cplusplus
#define AUTORUN_(fn) \
    static void fn(void); \
    static const int JOIN(fn,__1) = (fn(), 1); \
    static void fn(void)
#elif defined _MSC_VER && !defined(__clang__) // cl, but not clang-cl
#define AUTORUN_(fn) \
    static void fn(void); \
    static int JOIN(fn,__1) (){ fn(); return 0; } \
    __pragma(section(".CRT$XIU", long, read)) \
    __declspec(allocate(".CRT$XIU")) \
    static int(* JOIN(fn,__2) )() = JOIN(fn,__1); \
    static void fn(void)
#elif defined __TINYC__ // tcc...
#define AUTORUN_(fn) \
    __attribute((constructor)) \
    static void fn(void)
#else // gcc,clang,clang-cl...
#define AUTORUN_(fn) \
    __attribute__((constructor(__COUNTER__+101))) \
    static void fn(void)
#endif
// note: based on code by Joe Lowe (public domain).
// note: XIU for C initializers, XCU for C++ initializers, XTU for C deinitializers
#endif

#ifndef AUTOEND
#define AUTOEND void MACRO(end)(void); \
    AUTORUN { extern array_(autoend_fn) autoends; array_push(autoends, MACRO(end)); } void MACRO(end)(void)
typedef void(*autoend_fn)(void);
#elif KIT_CODE
array_(autoend_fn) autoends;
void autoends_do(void) { for( int i = 0, ii = array_count(autoends); i < ii; ++i) autoends[i](); }
AUTORUN { atexit(autoends_do); }
#endif

#ifndef ASSERT // assert() that works in retail builds. beware!
#define ASSERT(x,...) ( !!(x) ? 1 : (perror("Assertion failed `" #x "` ("__FILE__":"STRINGIZE(__LINE__) "); errno"), printf(""__VA_ARGS__), abort(), exit(-__LINE__)))
#endif

#ifndef CAST
#define CAST(type) ifdef(KIT_C,(type),type)
#endif

#ifndef COUNTOF // count items in an array at compile time
#define COUNTOF(x) ((int)(sizeof(x)/sizeof(0[x])))
#endif

#ifndef INLINED
#define INLINED ifdef(KIT_CL, static __forceinline, static __attribute__((always_inline)) inline)
#endif

#ifndef JOIN // joins two tokens at compile time
#define JOIN(x,y) J0IN(x,y)
#define J0IN(x,y) x##y
#endif

#ifndef LINE_STR // provide line number in string form
#define LINE_STR STRINGIZE(__LINE__)
#endif

#ifndef MACRO // create unique identifier valid per line basis
#define MACRO(name) JOIN(name, __LINE__)
#endif

#ifndef ONCE // execute next scope only once
#define ONCE static int MACRO(once) = 1; if(MACRO(once)) if(MACRO(once)=0,1) // for(;MACRO(once);MACRO(once)=0)
#endif

#ifndef STATIC_ASSERT // asserts at compile time
#define STATIC_ASSERT(x) typedef int UNIQUE(static_assert_)[(x)?1:-1]
#endif

#ifndef STRINGIZE // convert a token into a string at compile time
#define STRINGIZE(...) STRINGIZ3(__VA_ARGS__)
#define STRINGIZ3(...) #__VA_ARGS__
#endif

#ifndef THREAD
#define THREAD ifdef(KIT_CL,__declspec(thread),__thread)
#endif
    
#ifndef UNIQUE // create unique identifier valid per line + compilation unit
#define UNIQUE(name) JOIN( JOIN( name##_L, __LINE__ ), __COUNTER__ )
#endif

// ----------------------------------------------------------------------------

// leading and trailing zeros count. UB if x == 0
#if defined _MSC_VER && !defined __clang__
#include <immintrin.h>
#define __builtin_clz(x)   _lzcnt_u32(x)
#define __builtin_clzll(x) _lzcnt_u64(x)
#define __builtin_ctz(x)   _tzcnt_u32(x)
#define __builtin_ctzll(x) _tzcnt_u64(x)
#endif

// safe leading and trailing zeros count. no UB
#define CLZ32(x) ((x) ? __builtin_clz(x)   : 32)
#define CLZ64(x) ((x) ? __builtin_clzll(x) : 64)
#define CTZ32(x) ((x) ? __builtin_ctz(x)   : 32)
#define CTZ64(x) ((x) ? __builtin_ctzll(x) : 64)

// byte swapping
#define BSWAP16 ifdef(KIT_CL, _byteswap_ushort, __builtin_bswap16)
#define BSWAP32 ifdef(KIT_CL, _byteswap_ulong,  __builtin_bswap32)
#define BSWAP64 ifdef(KIT_CL, _byteswap_uint64, __builtin_bswap64)
