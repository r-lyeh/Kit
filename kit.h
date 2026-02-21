#ifndef KIT_H
#define KIT_H "1.0.0"

//#include <stdbool.h>: we use int
//#include <stdint.h>: we use int for int32_t, unsigned for uint32_t, then float, double and char

#define KIT_STATIC
#define KIT_IMPORT ifdef(KIT_CL, __declspec(dllimport), __attribute__ ((dllimport)))
#define KIT_EXPORT ifdef(KIT_CL, __declspec(dllexport), __attribute__ ((dllexport)))

#ifndef KIT_API
#define KIT_API    KIT_STATIC
#endif

#define KIT KIT_API
#define KIT_STRUCT(t, ...) (ifdef(KIT_C, (struct t), t){__VA_ARGS__})
#define countof(x) ((int)(sizeof(x) / sizeof(0[x])))

// ----------------------------------------------------------------------------

#line 1 "kit_ifdef.h"

#ifdef __cplusplus
#define KIT_C 0
#define KIT_CPP 1
#else
#define KIT_C 1
#define KIT_CPP 0
#endif

#  if defined __clang__
#define KIT_CL    0
#define KIT_CLANG 1
#define KIT_GCC   0
#elif defined _MSC_VER
#define KIT_CL    1
#define KIT_CLANG 0
#define KIT_GCC   0
#elif defined __GNUC__
#define KIT_CL    0
#define KIT_CLANG 0
#define KIT_GCC   1
#endif

#  if defined _WIN32
#define KIT_LINUX   0
#define KIT_MACOS   0
#define KIT_WINDOWS 1
#elif defined __APPLE__
#define KIT_LINUX   0
#define KIT_MACOS   1
#define KIT_WINDOWS 0
#elif defined linux
#define KIT_LINUX   1
#define KIT_MACOS   0
#define KIT_WINDOWS 0
#endif

#define ifdef( x,yes,/*no*/...) ifdefA(x, yes, __VA_ARGS__)
#define ifdefA(x,yes,/*no*/...) ifdefB(x,yes, __VA_ARGS__)
#define ifdefB(x,yes,/*no*/...) ifdef##x(yes, __VA_ARGS__)
#define ifdef1(yes,/*no*/...)   yes
#define ifdef0(yes,/*no*/...)   __VA_ARGS__/*no*/

//#define ifdef_1941 ifdef_1 // msvc 19.41.xxxx

// ----------------------------------------------------------------------------

#line 1 "kit_error.h"
#line 1 "kit_modal.h"
#line 1 "kit_time.h"
#line 1 "kit_date.h"
#line 1 "kit_icon.h"
#line 1 "kit_notify.h"
#line 1 "kit_drag.h"
#line 1 "kit_window.h"
#line 1 "kit_argcv.h"
#line 1 "kit_env.h"
#line 1 "kit_clipboard.h"
#line 1 "kit_monitor.h"
#line 1 "kit_app.h"

KIT int app_create();
KIT int app_ready();
KIT int app_swap();
KIT int app_destroy();
KIT int app_count();
KIT int app_restart();

KIT void* app_handle();

KIT void* window_handle(int id);
KIT int window_create(int id);
KIT int window_ready(int id);
KIT int window_swap(int id);
KIT int window_destroy(int id);
KIT int window_count();
KIT void window_clear(int id, unsigned color);
KIT int key(int id, int ch);

// ----------------------------------------------------------------------------

#line 1 "kit_zip.h"
#line 1 "kit_dir.h"
#line 1 "kit_io.h"

KIT int io_create();
KIT int io_destroy();

KIT void* io_handle();

// ----------------------------------------------------------------------------

#line 1 "kit_keyboard.h"

//KIT int key(); // int);
KIT char* keyboard_string();
KIT int keyboard_show(int);

KIT int ui_keyboard(int);

// ----------------------------------------------------------------------------

#line 1 "kit_gamepad.h"

KIT int gamepad(int);

KIT int ui_gamepad(int);

// ----------------------------------------------------------------------------

#line 1 "kit_mouse.h"

KIT int mouse_x();
KIT int mouse_y();
KIT int mouse_w();

KIT int ui_mouse(int);

// ----------------------------------------------------------------------------

#line 1 "kit_cursor.h"

KIT int cursor(int);

KIT int ui_cursor(int);

#line 0
#endif
