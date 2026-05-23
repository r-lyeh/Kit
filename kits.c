#include <SDL3/sdl.h>

#define assert      SDL_assert
#define calloc      SDL_calloc
#define malloc      SDL_malloc
#define realloc     SDL_realloc
#define free        SDL_free
#define strlen      SDL_strlen
#define strcmp      SDL_strcmp
#define strncmp     SDL_strncmp
#define strdup      SDL_strdup
#define strtol      SDL_strtol
#define strstr      SDL_strstr
#define strchr      SDL_strchr
//#define strcspn     SDL_strcspn
//#define snprintf    SDL_snprintf
#define sscanf      SDL_sscanf
#define memcmp      SDL_memcmp
#define memcpy      SDL_memcpy
#define memmove     SDL_memmove
#define memset      SDL_memset
#define isspace     SDL_isspace
#define isalnum     SDL_isalnum
#define isalpha     SDL_isalpha
#define isdigit     SDL_isdigit
#define atoi        SDL_atoi
#define atof        SDL_atof

#if __has_include("3rd/3rd_stb_image.h") // JPG, PNG, BMP, TGA, PSD, GIF
#define STB_IMAGE_IMPLEMENTATION
#include "3rd/3rd_stb_image.h"
#endif
#if __has_include("3rd/3rd_stb_image_write.h") // JPG, PNG, BMP, TGA, HDR
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "3rd/3rd_stb_image_write.h"
#endif
#if __has_include("3rd/3rd_koi_image.h") // QOI
#define KOI_IMAGE_IMPLEMENTATION
#include "3rd/3rd_koi_image.h"
#endif
#if __has_include("3rd/3rd_tiny_webp.h") // WEBP
#define twp_IMPLEMENTATION
#include "3rd/3rd_tiny_webp.h"
#endif

#if __has_include("3rd/3rd_minilua.h")
#define LUA_IMPL
#include "3rd/3rd_minilua.h" 
#endif

//#include "kit/kit.h"
//#include "kits/kit.data/kit.c"

#if __has_include("kits/kit.data/3rd_xml.h")
#define XML_C
#include "kits/kit.data/3rd_xml.h"
#endif

#if __has_include("kits/kit.data/3rd_json5.h")
#define JSON5_C
#include "kits/kit.data/3rd_json5.h"
#endif
