// ----------------------------------------------------------------------------
// some required defines before any other header is included
// ----------------------------------------------------------------------------

// #define _POSIX_C_SOURCE 199309L

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef _CRT_NONSTDC_NO_DEPRECATE
#define _CRT_NONSTDC_NO_DEPRECATE
#endif

#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#endif

#ifndef GL_SILENCE_DEPRECATION
#define GL_SILENCE_DEPRECATION
#endif

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif

#ifndef OEMRESOURCE
#define OEMRESOURCE
#endif

#ifndef _ALLOW_KEYWORD_MACROS
#define _ALLOW_KEYWORD_MACROS
#endif

#ifndef UNICODE
#define UNICODE 1
#endif

// ----------------------------------------------------------------------------
// standard C headers we commonly use in all platforms
// ----------------------------------------------------------------------------

#include <errno.h>
#include <float.h>
#include <limits.h>
#include <signal.h>
#include <time.h>

#include <assert.h>  // assert()
#include <ctype.h>   // tolower()
#include <math.h>    // NAN (C99: nan()nanf() vs sqrt(-1), 0.0/0.0) isnan() INFINITY -INFINITY
#include <stdarg.h>  // va_list, va_copy()
#include <stdbool.h> // bool
#include <stddef.h>  // offsetof()
#include <stdint.h>  // u/int64_t
#include <stdio.h>   // fopen()
#include <stdlib.h>  // realloc()
#include <string.h>  // strdup(), strcmp(), memcmp()

#ifndef _WIN32
#include <strings.h>
#endif
