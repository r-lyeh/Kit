// kit game toolkit
// - rlyeh, μLicensed

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SDL_ASSERT_LEVEL
#if NDEBUG
#define SDL_ASSERT_LEVEL 0
#else
#define SDL_ASSERT_LEVEL 2
#endif
#endif

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <SDL3_net/SDL_net.h>

#include "kit/kit.h"
#include "kits/kit.h"

#ifndef KIT_HEADER
#define KIT_HEADER "0.0.0"

// ----------------------------------------------------------------------------
// units and constants

enum { KiB = 1024, MiB = 1024*KiB, GiB = 1024*MiB };

enum {
    #define   P(P) ((255<<24)|(((P)&255)<<16)|((((P)>>8)&255)<<8)|((P)>>16))
    KIT_BLACK=P(0x000000),KIT_GRAY1 =P(0x505050),KIT_GRAY2=P(0xB0B0B0),KIT_WHITE =P(0xFFF5CC), // GRAYSCALE
    KIT_NAVY =P(0x243777),KIT_PURPLE=P(0x89008C),KIT_TEAL =P(0x008750),KIT_BROWN =P(0x801E00), // BRGO DARK
    KIT_BLUE =P(0x0000CC),KIT_RED   =P(0xCC0000),KIT_GREEN=P(0x00CC00),KIT_ORANGE=P(0xFF8000), // BRGO NORMAL
    KIT_CYAN =P(0x0088FF),KIT_PINK  =P(0xFF0050),KIT_AQUA =P(0x00F084),KIT_YELLOW=P(0xFFE600), // BRGO BRIGHT
    #undef    P
};

// ----------------------------------------------------------------------------
// data types

typedef map_(char*,char*) dict;

typedef array_(int) list;

typedef       char *data; // like `void*` but also addressable by [index] access

typedef const char *name; // stable(ptr)

typedef const char *text; // utf8 everywhere

typedef const char *url;  // file;

typedef struct { int x, y; } int2, int2_t;
typedef struct { float x, y, z, w; } quat, quat_t;
typedef struct { float x, y, w, h; } rect, rect_t;
typedef union  { struct { float x,y; }; struct { float r,g;}; struct { float w,h; }; } float2, float2_t;
typedef union  { struct { float x,y,z; }; struct { float r,g,b;}; struct { float w,h,d; }; } float3, float3_t;
typedef union  { struct { float x,y,z,w; }; struct { float r,g,b,a;}; } float4, float4_t;

#define int2(x,y)       ((int2){(int)(x), (int)(y)})
#define rect(x,y,w,h)   ((rect){(float)(x), (float)(y), (float)(w), (float)(h) })
#define quat(x,y,z,w)   ((quat){(float)(x), (float)(y), (float)(z), (float)(w) })
#define float2(x,y)     ((float2){(float)(x), (float)(y)})
#define float3(x,y,z)   ((float3){(float)(x), (float)(y), (float)(z)})
#define float4(x,y,z,w) ((float4){(float)(x), (float)(y), (float)(z), (float)(w) })

// ----------------------------------------------------------------------------
// integer types, that can be transported losslessly in float (int24) and double (int53) variables

typedef uint32_t uint24_t;
typedef uint64_t uint53_t;

#define UINT24_MAX  ((uint24_t)0xFFFFFFu) // 2^24 - 1
#define UINT53_MAX  ((uint53_t)0x1FFFFFFFFFFFFFull)  // 2^53 - 1

static inline uint53_t uint53(uint64_t val) { return val & UINT53_MAX; }
static inline uint24_t uint24(uint64_t val) { return (uint24_t)(val & UINT24_MAX); }

/* typedef struct uint24_t {
    uint32_t value : 24;
    uint32_t pad   : 8;
} uint24_t;

typedef struct uint53_t {
    uint64_t value : 53;
    uint64_t pad   : 11;
} uint53_t; */

// ----------------------------------------------------------------------------
// event struct and modules in alphabetical order

typedef struct event {
uint64_t    frame:62;
uint64_t    init:1; // true if initializing
uint64_t    quit:1; // true if quitting
float       step;   // true if updating at fixed rate. holds fixed delta time.
float       tick;   // true if updating at variable rate. holds variable delta time.
SDL_Event*  emit;   // fired event
unsigned    type;   // type of fired event
} event;

// @todo: .reset(numcode)

extern struct app {
void        (*reload)(void); // reloads app from the very beginning
void        (*quit)(int rc); // quits app gracefully, then returns code to bash 
struct event event;          // this event is repeatedly sent to main() entrypoints
} app;

// @todo: archive.zip, archive.unzip, archive.list ... dir|zip

extern struct archive {
text*       (*dir)(text filter, int *count);
} archive;

// @todo: expose array

extern struct assert {
int         (*enabled)(void);      // returns true on debug builds
int         (*debug)(bool expr);   // assert that works only in debug builds
int         (*release)(bool expr); // assert that works also in release builds
void        (*breakpoint)(void);   // triggers a breakpoint
} assert;

extern struct audio {
unsigned    (*open)(const char *audio_file);
unsigned    (*duration)(unsigned buffer);
void        (*close)(unsigned *sample);
} audio;

extern struct battery {
int         (*time)(void);  // remaining time (seconds)
int         (*power)(void); // power 0..100%
bool        (*inuse)(void); // whether battery is being used and discharging (1), or wallplugged/not available (0).
} battery;

extern struct clipboard {
void        (*set)(text contents);
text        (*get)(void); // either "text" or empty string
bool        (*empty)(void);
} clipboard;

extern struct color {
unsigned    (*hex)(text hexcolor); // "#rgb" "#rgba" "#rrggbb" "#rrggbbaa"
unsigned    (*hsv)(float h255, float s255, float v255);
unsigned    (*hsva)(float h255, float s255, float v255, float a255);
unsigned    (*rgb)(float r255, float g255, float b255);
unsigned    (*rgba)(float r255, float g255, float b255, float a255);
uint8_t     (*r)(unsigned color);
uint8_t     (*g)(unsigned color);
uint8_t     (*b)(unsigned color);
uint8_t     (*a)(unsigned color);
} color;

// @todo: expose dict

// @todo: u64 date.now(), u64 date.today(), date.format(u64)

extern struct dialog {
void        (*alert)(text msg);        // modal with alert icon + OK button
void        (*fatal)(text msg);        // modal with fatal icon + OK button
int         (*prompt)(text head, text body, int buttons, /*texts*/...); // modal with variable number of buttons. returns pressed button (1=1st button,2=2nd buttton...) or 0 if canceled
void        (*loadfile)(text *file);   // async. stores temporary value in `file` when done.
void        (*savefile)(text *file);   // async. stores temporary value in `file` when done.
void        (*folder)(text *path);     // async. stores temporary value in `path` when done.
} dialog;

extern struct display {
unsigned    (*count)(void);                  // returns number of monitors [N]
text        (*name)(unsigned monitor);       // [0=default monitor, 1=first monitor, 2=second monitor .. N=last monitor]
int         (*width)(unsigned monitor);      // [0=default monitor, 1=first monitor, 2=second monitor .. N=last monitor]
int         (*height)(unsigned monitor);     // [0=default monitor, 1=first monitor, 2=second monitor .. N=last monitor]
int         (*bpp)(unsigned monitor);        // [0=default monitor, 1=first monitor, 2=second monitor .. N=last monitor]
float       (*hz)(unsigned monitor);         // [0=default monitor, 1=first monitor, 2=second monitor .. N=last monitor]
float       (*scale)(unsigned monitor);      // [0=default monitor, 1=first monitor, 2=second monitor .. N=last monitor]
float       (*dpi)(unsigned monitor);        // [0=default monitor, 1=first monitor, 2=second monitor .. N=last monitor]
float4_t    (*workarea)(unsigned monitor);   // [0=default monitor, 1=first monitor, 2=second monitor .. N=last monitor]
} display;

extern struct dll {
void*       (*load)(text file, text name);
void        (*unload)(text file);
} dll;

extern struct elapsed {
double      (*hh)(void); //       hours        since app ran
double      (*mm)(void); //       minutes      since app ran
double      (*ss)(void); // 10^0; seconds      since app ran 
double      (*ms)(void); // 10^3; milliseconds since app ran
uint64_t    (*us)(void); // 10^6; microseconds since app ran
uint64_t    (*ns)(void); // 10^9; nanoseconds  since app ran
} elapsed;

extern struct endian {
int         (*big)(void);
int         (*little)(void);
float       (*swapf)(float value);
uint16_t    (*swap16)(uint16_t value);
uint32_t    (*swap32)(uint32_t value);
uint64_t    (*swap64)(uint64_t value);
} endian;

extern struct file {
data        (*read)(text url, int *len); // @fixme: SDL_free after use
bool        (*write)(text url, const void *data, int len);
bool        (*exists)(text url);
int         (*size)(text url);
} file;

extern struct folder {            //                  | Windows | macOS/iOS  | tvOS | Unix/XDG | Haiku | Emscripten |
text        (*home)(void);        // USER HOME        |    X    |     X      |      |     X    |   X   |     X      |
text        (*desktop)(void);     // USER DESKTOP     |    X    |     X      |      |     X    |   X   |            |
text        (*documents)(void);   // USER DOCUMENTS   |    X    |     X      |      |     X    |       |            |
text        (*downloads)(void);   // USER DOWNLOADS   | Vista+  |     X      |      |     X    |       |            |
text        (*music)(void);       // USER MUSIC       |    X    |     X      |      |     X    |       |            |
text        (*pictures)(void);    // USER PICTURES    |    X    |     X      |      |     X    |       |            |
text        (*shared)(void);      // USER PUBLICSHARE |         |     X      |      |     X    |       |            |
text        (*savedgames)(void);  // USER SAVEDGAMES  | Vista+  |            |      |          |       |            |
text        (*screenshots)(void); // USER SCREENSHOTS | Vista+  |            |      |          |       |            |
text        (*templates)(void);   // USER TEMPLATES   |    X    |     X      |      |     X    |       |            |
text        (*videos)(void);      // USER VIDEOS      |    X    |     X*     |      |     X    |       |            |
text        (*working)(void);     // current working directory (cwd). changed with chdir()
text        (*install)(void);     // wherever the executable is installed. read-only usually. (*exe) ?
text        (*storage)(text your_company, text your_appname); // wherever you can write files into
} folder;

extern struct font {
unsigned    (*open)(float ptsize, text files);
float       (*size)(unsigned handle, float size);
bool        (*bold)(unsigned handle, int bold);
bool        (*italic)(unsigned handle, int italic);
bool        (*strike)(unsigned handle, int strike);
bool        (*underline)(unsigned handle, int underline);
int         (*outline)(unsigned handle, int outline);
int         (*spacing)(unsigned handle, float spacing); // accepts integers only and sets spacing. returns current spacing in any case
char        (*align)(unsigned handle, text mode); // (l)eft,(c)enter,(r)ight
char        (*direction)(unsigned handle, text mode); // (l)tr, (r)tl, (b)ottomtop, (t)opbottom
char        (*hinting)(unsigned handle, text mode); // (m)onochrome, (n)ormal, (0)none, (l)ight, (s)ubpixel
SDL_Surface*(*bake)(unsigned handle, unsigned quality03, unsigned rgba_fg, unsigned rgba_bg, int width, text string); // width: <0 unlimited, 0 break on linefeeds, >0 specific width in pixels
TTF_Font*   (*handle)(unsigned handle);
void        (*close)(unsigned *handle);
} font;

extern struct fx {
bool            (*open)(void);  // call after render.open(). Picks up render.handle + window.handle.
int             (*add)(url frag_spv, url vert_spv); // adds a pass. frag_spv is required. if vert_spv is NULL, the built-in fullscreen triangle will be used. Returns pass index or -1.
bool            (*enable)(int idx, int on); // toggles a pass on or off, if valid argument is supplied. Returns current state in any case
float*          (*uniform)(int idx, int slot, float v[4]); // sets a per-pass uniform vec4 slot, if valid argument is supplied. Returns current float4 uniform in any case
SDL_GPUTexture* (*begin)(void); // call at start of tick, before rendering your scene. Returns the SDL_GPUTexture to render your scene into, or NULL if no passes are enabled
void            (*end)(void);   // call after your scene is rendered into the RT returned by fx_begin(). Runs the chain and blits the result via render.handle.
void            (*close)(void); // frees all resources.
SDL_GPUDevice    *handle;
} fx;

// @todo: expose hash

extern struct image {
void*       (*open)(url path, int *w, int *h, int *n);
void        (*close)(void** pixels);
} image;

extern struct imgui {
bool        (*open)(void);
SDL_Event*  (*event)(SDL_Event*);
void        (*begin)(void);
void        (*end)(void);
bool        (*close)(void);
} imgui;

extern struct kit {
text        (*version)(void);
text        (*backends)(void);
bool        (*loop)(bool pumps); // kit pre-tick handler, with optional events pump. returns true if app is running, or false if app is being closed
bool        (*post)(void);       // kit post-tick handler. returns true if app is running, or false if app is being closed
} kit;

extern struct language {
bool        (*import)(url file);    // imports a .mo file with key/value translations
text        (*translate)(text key); // returns translated .mo key if exists, or "[[key]]" otherwise
} language;

// @todo: lerp.

extern struct listener {
void        (*volume)(const float volume); // ~master volume
void        (*position)(const float position[3]);
void        (*velocity)(const float velocity[3]);
void        (*orientation)(const float normdir[3], const float normup[3]);
void        (*doppler)(float factor, float speed_of_sound_meters_sec); // 1.0 + 343.3 m/s
void        (*model)(const char *distance_model); // "none|inverse|linear|exponent"|"clamped"
} listener;

extern struct locale {
text        (*date)(void); // returns YYYY/MM/DD, DD/MM/YYYY or MM/DD/YYYY 
text        (*time)(void); // returns hh:mm:ss in 12H or 24H format
} locale;

extern struct lua {
unsigned    (*open)(url file_lua); // input file can be null
void        (*read)(unsigned L, url file_lua);
void        (*call)(unsigned L, url func);
void        (*reload)(unsigned L);
void        (*close)(unsigned *L);
} lua;

// @todo: expose (hash)map

// @todo: unsigned  (*find)(unsigned color); // find best color match within palette

extern struct palette {
void        (*set)(unsigned index, unsigned color);
unsigned    (*get)(unsigned index);
} palette;

extern struct os {
text        (*arg)(text key_value); // "--key" or "--key=value" formats
unsigned    (*argc)(void);
text        (*argv)(unsigned index);
int         (*memory)(void);  // MiB, to memory.
int         (*storage)(void); // MiB, to save.
text        (*name)(void);
void        (*browse)(text url); // tells os to open a local file or remote url
void        (*beep)(void);
void        (*die)(text msg);
void        (*log)(text fmt, /*args*/...);
} os;

// @todo: pcg
//    text (*string)(dict); // @todo: markov
//   float (*noise)(int x, int y); // @todo: perlin

extern struct random {
void        (*seed)(int n); // 0 to seed time; specific [n] seed otherwise.
bool        (*boolean)(void);
int         (*integer)(int max); // [0..max]
double      (*floating)(double max); // [0..max]
double      (*range)(double min, double max); // [min..max]
} random;

extern struct render {
bool        (*open)(int w, int h, float area, unsigned flags);
void        (*clear)(unsigned color); // clears with color. beware: alpha can be effective on some platforms
int         (*vsync)(int mode);       // -1:adaptive,0:unlimited fps,1:match monitor hz, >1: capped [5:fps, 30:fps, 60:fps, 144:fps, etc]
int         (*fps)(void);
float       (*delta)(void);
float2_t    (*size)(void);
bool        (*save)(url file_png);
void        (*present)(void);         // swap buffers
SDL_Renderer *handle;
} render;

// @todo: expose resize allocator: p = resize(p, sz);

extern struct sleep {
void        (*ns)(double ns);
void        (*us)(double us);
void        (*ms)(double ms);
void        (*ss)(double ss);
} sleep;

extern struct speaker {
unsigned    (*open)(const float position[3]);
unsigned    (*range)(unsigned source, float mindistance, float maxdistance, float rollOff); // how and where sound attenuates over distance
unsigned    (*relative)(unsigned source, bool relative_pos); // relative to listener. true for 2D audio
unsigned    (*position)(unsigned source, const float position[3]); // sets panning (audio2d) or position (audio3d)
unsigned    (*velocity)(unsigned source, const float velocity[3]);
unsigned    (*direction)(unsigned source, const float direction[3]);
unsigned    (*loop)(unsigned source, bool loop);
unsigned    (*volume)(unsigned source, float volume);
unsigned    (*pitch)(unsigned source, float pitch);
unsigned    (*play)(unsigned source, unsigned sample);
unsigned    (*pause)(unsigned source);
unsigned    (*resume)(unsigned source);
unsigned    (*stop)(unsigned source);
unsigned    (*playing)(unsigned source); // returns 0 if not playing, source otherwise (>0)
unsigned    (*stopped)(unsigned source); // returns 0 if not stopped, source otherwise (>0)
unsigned    (*paused)(unsigned source);  // returns 0 if not paused,  source otherwise (>0)
void        (*close)(unsigned *source);
} speaker;

#if 0 // @todo: expand string
    // append, replace

    char *(*strsep)(char **sp, text sep); // better than strtok(), as it preserves empty strings within delimiters
    char *(*swap)(char *text, text target, text replacement); // replaced only if replacement is shorter than target

    void (*zalloc)();

    void (*quark)();

    void (*eval)();

    // strstr, strstri
    // strbeg, strendi
    // split, join
    // each
#endif

extern struct string {
data        (*va)(text fmt, ...);   // temporary
data        (*dup)(text src);       // copy

data        (*open)(text fmt, ...);  // heap. array_push() instead?
void        (*close)(text *str);     // heap free

int         (*count)(text s, int ch);          // count repetitions of a character in a string
int         (*match)(text s, text wildcard);   // returns true if wildcard matches
int         (*matchi)(text s, text wildcard);  // returns true if wildcard matches (case insensitive)
text        (*valid)(text str);                // returns "" if NULL, string contents otherwise.
text        (*stable)(text str);               // returns interned stable pointer that can be checked for equality
uint64_t    (*hash)(text str);
} string;

extern struct texture {
unsigned    (*open)(url path);
unsigned    (*copy)(void *pixels, int w, int h, int n);
void*       (*handle)(unsigned texture);
unsigned    (*width)(unsigned texture);
unsigned    (*height)(unsigned texture);
unsigned    (*pitch)(unsigned texture);
void        (*close)(unsigned *texture);
} texture;

// @todo: .detach()
// @todo: .mutex(), lock()
// @todo: .open(), .close()

extern struct thread {
int         (*cores)(void);
void        (*yield)(void);
bool        (*primary)(void); // is main thread
uint64_t    (*self)(void); // return current thread id. 0 if threads are not supported on this platform
uint64_t    (*create)(text name, int (*fn)(void*), void *userdata);
text        (*name)(uint64_t tid);
int         (*join)(uint64_t tid); // return code from awaited thread. -1 if not reachable (dettached or invalid)
// void     (*fork)(void (*fn)(void*), void *userdata); // run on main thread
} thread;

// @todo: .cancel(id)

extern struct timer {
void        (*once)(double interval_ss, void (*fn)(void));
void        (*every)(double interval_ss, void (*fn)(void));
} timer;

// @todo: tween.

// @todo: ui.notify
// @todo: ui.dialog
// @todo: ui.text
// @todo: ui.textedit
// @todo: ui.string
// @todo: ui.color3
// @todo: ui.color4
// @todo: ui.transform
// @todo: ui.menuL
// @todo: ui.menuR

extern struct ui {
int         (*window)(text, int *flags);
int         (* tree)(text);
int         (*  label)(text);
int         (*  button)(text);
int         (*  buttons)(int num, /*texts*/...);
int         (*  section)(text, int open);
int         (*  integer)(text, int *i, int lo, int hi);
int         (*  floating)(text, float *f, float lo, float hi);
int         (*  boolean)(text, void *b);
int         (*  rgba8)(text, void *f);
int         (*  texture)(void *handle);
int         (*  label3)(text, float v[3]);
int         (*  label4)(text, float v[4]);
int         (*  vec2)(text, float v[2], float lo, float hi);
int         (*  vec3)(text, float v[3], float lo, float hi);
int         (*  vec4)(text, float v[4], float lo, float hi);
int         (* tree_end)(void);
int         (* popup)(text);
int         (* popup_end)(void);
int         (*window_end)(void);

int         (*hovered)(void);
} ui;

// @todo: .keyboard() ; returns keyboard layout

extern struct user {
text        (*name)(void);     // returns user's name from environment ($USER or $USERNAME)
text        (*language)(void); // returns user's preferred language ("xx"). eg: "en" "es" "it" "fr" etc.
text        (*country)(void);  // returns user's country variant ("XX"), or empty string. eg: "UK" "US" "CA" "" etc.
} user;

extern struct webcam {
unsigned    (*count)(void);
unsigned    (*capture)(unsigned camid);  // returns 0 if camera not available or not ready. updated texture handle otherwise.
void        (*close)(unsigned *camid);
} webcam;

extern struct window {
text        (*icon)(text pngfile); // loads png as window icon. returns pngfile
text        (*title)(text string); // sets title if arg is valid string. returns window title in any case
bool        (*show)(int on); // sets visibility if arg is valid boolean. returns current visibility in any case
bool        (*ontop)(int on); // sets on-top flag if arg is valid boolean. returns current on-top flag in any case
bool        (*flash)(int on); // sets flash flag if arg is valid boolean. returns current flash flag in any case
bool        (*confined)(int on); // sets confined flag if arg is valid boolean. returns current confined flag in any case
bool        (*decorated)(int on); // sets decorated flag if arg is valid boolean. returns current decorated flag in any case
bool        (*resizable)(int on); // sets resizable flag if arg is valid boolean. returns current resizable flag in any case
bool        (*fullscreen)(int on); // sets fullscreen flag if arg is valid boolean. returns current fullscreen flag in any case. @todo: [2] for hard-fullscreen (win32)
float       (*alpha)(float percent); // sets opacity alpha if arg is valid percent [0..1]. returns current opacity alpha in any case
float       (*progress)(float percent); //< sets progress bar if arg is valid percent: [0] shows infinite bar, [>0] shows normal bar, [1] hides bar. returns current percentage in any case
float       (*aspect)(float min_ratio, float max_ration); //< applies aspect ratios if both args are >= 0. returns current aspect ratio in any case
float2_t    (*position)(int x, int y); // sets position if both args >= 0. returns current position in any case
float2_t    (*size)(int w, int h); // sets size if both args >0. returns current size in any case
float       (*maximize)(float sz); // // sets state if arg is valid [0(minimized)..>0(windowed)..1(maximized)]. returns current state in any case
SDL_Window   *handle; // @todo. void *handles[3]: native<sdl3<gl/vk handles
} window;

// ----------------------------------------------------------------------------

#if NDEBUG
#define SDL_CHECK(x) (void)(x)
#else
#define SDL_CHECK(x) ((x) || dialog.prompt("Fatal", va("%s\n%s\n\n%s", #x, SDL_GetError(), trace(16)), 1, "Ok"))
#endif

typedef struct { const char *path, *name; int id; void(*fn)(event); } main_t;

#undef  main
#define main(...)       ma1n( JOIN(app,__COUNTER__), __VA_ARGS__ )
#define ma1n(func, ...) func(event); extern ifdef(KIT_CPP,"C") array_(main_t) mains; AUTORUN { main_t e = { __FILE__, strrchr("/"__FILE__,'/')+1, __COUNTER__, func }; array_push(mains, e); } void func(__VA_ARGS__)

#endif // KIT_HEADER

#ifdef __cplusplus
}
#endif
