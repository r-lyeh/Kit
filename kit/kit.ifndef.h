// ----------------------------------------------------------------------------
// if/n/def hell
// ----------------------------------------------------------------------------

#define ifdef(arg, then, /*else*/...)        ifd3f(arg, then, __VA_ARGS__)
#define ifd3f(arg, then, /*else*/...)        ifdef_##arg(then, ##__VA_ARGS__)

#define ifndef(arg, then, /*else*/...)       ifnd3f(arg, then, __VA_ARGS__)
#define ifnd3f(arg, then, /*else*/...)       ifdef_##arg(__VA_ARGS__, then)

#define ifdef_1(then, /*else*/...)           then
#define ifdef_0(then, /*else*/...)           __VA_ARGS__

#define ifdef_true(then, /*else*/...)        then
#define ifdef_false(then, /*else*/...)       __VA_ARGS__

// ----------------------------------------------------------------------------
// if/n/def hell
// ----------------------------------------------------------------------------

#ifndef __cplusplus
#define KIT_C          1
#define KIT_CPP        0
#else
#define KIT_C          0
#define KIT_CPP        1
#endif

#if defined(_M_IA64) || defined(__ia64)
#define KIT_IA64       1
#else
#define KIT_IA64       0
#endif

#if defined(_M_X64) || (defined(__x86_64__) && !defined(__ILP32__))
#define KIT_X64        1
#define KIT_X86        0
#elif defined(_M_IX86) || defined(__i386) || defined(__i386__)
#define KIT_X64        0
#define KIT_X86        1
#else
#define KIT_X64        0
#define KIT_X86        0
#endif

#if defined(_M_ARM64) || defined(__arm64) || defined(__arm64__) || defined(__aarch64__)
#define KIT_ARM64      1
#define KIT_ARM32      0
#elif defined(_M_ARM) || defined(__arm__)
#define KIT_ARM64      0
#define KIT_ARM32      1
#else
#define KIT_ARM64      0
#define KIT_ARM32      0
#endif

#if defined(_ARCH_PPC64) || defined(__PPC64__) || defined(__ppc64__) || defined(__powerpc64__)
#define KIT_PPC64      1
#define KIT_PPC32      0
#elif defined(_ARCH_PPC) || defined(__PPC__) || defined(__ppc__) || defined(__powerpc__) || defined(__ppc) || defined(__powerpc)
#define KIT_PPC64      0
#define KIT_PPC32      1
#else
#define KIT_PPC64      0
#define KIT_PPC32      0
#endif

#if KIT_IA64 || KIT_X64 || KIT_ARM64 || KIT_PPC64 // || defined(__LP64__) || defined(_LP64) || defined(_WIN64)
#define KIT_64         1
#define KIT_32         0
#else
#define KIT_64         0
#define KIT_32         1
#endif

#if (__BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__) || KIT_PPC32 || KIT_PPC64
#define KIT_BIG        1
#define KIT_LITTLE     0
#else
#define KIT_BIG        0
#define KIT_LITTLE     1
#endif

#if defined _WIN32
#define KIT_WINDOWS    1
#else
#define KIT_WINDOWS    0
#endif

#if defined __linux__ || defined __linux
#define KIT_LINUX      1
#else
#define KIT_LINUX      0
#endif

#if defined __APPLE__
#define KIT_MACOS      1
#else
#define KIT_MACOS      0
#endif

#if defined __FreeBSD__ || defined __NetBSD__ || defined __OpenBSD__ || defined __APPLE__
#define KIT_BSD        1
#else
#define KIT_BSD        0
#endif

#if defined __EMSCRIPTEN__
#define KIT_WASM       1
#else
#define KIT_WASM       0
#endif

#if defined __ANDROID_API__
#define KIT_ANDROID    1
#else
#define KIT_ANDROID    0
#endif

#if defined __IPHONEOS__
#define KIT_IOS        1
#else
#define KIT_IOS        0
#endif

#if KIT_IOS || KIT_ANDROID
#define KIT_MOBILE     1
#define KIT_DESKTOP    0
#else
#define KIT_MOBILE     0
#define KIT_DESKTOP    1
#endif

#if KIT_WINDOWS || KIT_ANDROID || KIT_WASM
#define KIT_POSIX      0
#else
#define KIT_POSIX      1
#endif

#  if defined __TINYC__
#define KIT_TCC        1
#define KIT_GCC        0
#elif defined __GNUC__
#define KIT_TCC        0
#define KIT_GCC        1
#else
#define KIT_TCC        0
#define KIT_GCC        0
#endif

#if defined __clang__ && defined _MSC_VER
#define KIT_CLANG      1
#define KIT_CLANGCL    1
#elif defined __clang__
#define KIT_CLANG      1
#define KIT_CLANGCL    0
#else
#define KIT_CLANG      0
#define KIT_CLANGCL    0
#endif

#if defined _MSC_VER
#define KIT_CL         1
#define KIT_MINGW      0
#elif defined __MINGW64__ || defined __MINGW32__
#define KIT_CL         0
#define KIT_MINGW      1
#else
#define KIT_CL         0
#define KIT_MINGW      0
#endif

// rely on NDEBUG as the official/portable way to disable asserts
// we extend NDEBUG=[0,1,2,3] to signal the compiler optimization flags O0,O1,O2,O3
#if   defined NDEBUG && NDEBUG >= 2
#define KIT_DEBUG      0
#define KIT_RELEASE    0
#define KIT_RETAIL     1
#elif defined NDEBUG && NDEBUG >= 1
#define KIT_DEBUG      0
#define KIT_RELEASE    1
#define KIT_RETAIL     0
#else
#define KIT_DEBUG      1
#define KIT_RELEASE    0
#define KIT_RETAIL     0
#endif

#ifdef _USRDLL // detect /LD or /DLL flag (cl)
#define KIT_DLL 1
#define KIT_EXE 0
#else
#define KIT_DLL 0
#define KIT_EXE 1
#endif

#ifdef _DLL // detect /MD flag (cl CRT)
#define KIT_MD 1
#define KIT_MT 0
#else
#define KIT_MD 0
#define KIT_MT 1
#endif
