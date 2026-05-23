// Kit framework
// - rlyeh, 0bsd licensed

#ifndef KIT_H
#define KIT_H "0.0.0"
#endif

// ----------------------------------------------------------------------------
// visibility
// win32 users may need to use -DKIT_API=KIT_EXPORT/KIT_IMPORT/KIT_STATIC as needed when using DLLs.

#define KIT_IMPORT ifdef(KIT_WINDOWS, ifdef(KIT_GCC, __attribute__ ((dllimport)), __declspec(dllimport)))
#define KIT_EXPORT ifdef(KIT_WINDOWS, ifdef(KIT_GCC, __attribute__ ((dllexport)), __declspec(dllexport)))
#define KIT_STATIC

#ifndef KIT_API
#define KIT_API    KIT_EXPORT // public visibility by default
#endif

// -----------------------------------------------------------------------------
// available backends. pick your poisons:

/*
#define v3_app_backend        ifdef(true,       "app(rgfw).h",       "app(null).h")
#define v3_asset_backend      ifdef(true,       "asset().h")
#define v3_audio_backend      ifdef(true,       "audio().h",         "audio(null).h")
#define v3_battery_backend    ifdef(true,       "battery(rabia).h")
#define v3_callback_backend   ifdef(true,       "callback().h")
#define v3_cook_backend       ifdef(KIT_RETAIL,  "cook(null).h",      "cook().h")
#define v3_dd_backend         ifdef(true,       "dd(gl3).h")
#define v3_dll_backend        ifdef(true,       "dll().h")
#define v3_eval_backend       ifdef(true,       "eval(stoop).h")
#define v3_file_backend       ifdef(true,       "file(stdio).h",     "file(null).h")
#define v3_flag_backend       ifdef(true,       "flag().h")
#define v3_fpslimit_backend   ifdef(true,       "fps(auto).h",       "fps(max).h", "fps(60).h", "fps(30).h")
#define v3_gamepad_backend    ifdef(true,       "gamepad(glfw3).h",  "gamepad(mg).h",     "gamepad(null).h")
#define v3_gldebug_backend    ifdef(KIT_RETAIL,  "gldebug(null).h",   "gldebug().h")
#define v3_gui_backend        ifdef(KIT_RETAIL,  "gui(null).h",       "gui(microui).h")
#define v3_image_backend      ifdef(true,       "image(stb).h")
#define v3_ini_backend        ifdef(true,       "ini().h")
#define v3_json_backend       ifdef(true,       "json().h")
#define v3_keyboard_backend   ifdef(true,       "keyboard(glfw3).h", "keyboard(null).h")
#define v3_lookup_backend     ifdef(true,       "lookup().h")
#define v3_ls_backend         ifdef(true,       "ls().h")
#define v3_math_backend       ifdef(true,       "vecmath().h")
#define v3_memleaks_backend   ifdef(KIT_RETAIL,  "memleaks(null).h",  "memleaks(file).h")
#define v3_monitor_backend    ifdef(true,       "monitor(glfw3).h",  "monitor(rgfw).h")
#define v3_mouse_backend      ifdef(true,       "mouse(glfw3).h",    "mouse(null).h")
#define v3_object_backend     ifdef(true,       "object().h")
#define v3_path_backend       ifdef(true,       "path(stat).h")
#define v3_render_backend     ifdef(true,       "render(gl3).h")
#define v3_resize_backend     ifdef(true,       "resize(realloc).h")
#define v3_script_backend     ifdef(true,       "script(lua548).h",  "script(null).h")
#define v3_stream_backend     ifdef(true,       "stream().h")
#define v3_test_backend       ifdef(KIT_RETAIL,  "test(null).h",      "test(stdio).h")
#define v3_text_backend       ifdef(true,       "text(widen).h")
#define v3_thread_backend     ifdef(true,       "thread(mt).h")
#define v3_time_backend       ifdef(true,       "time().h",          "time(null).h")
#define v3_trace_backend      ifdef(true,       "trace().h")
#define v3_url_backend        ifdef(true,       "url().h")
#define v3_va_backend         ifdef(true,       "va(stb).h")
#define v3_vfs_backend        ifdef(true,       "vfs(zip).h",        "vfs(null).h")
#define v3_xml_backend        ifdef(true,       "xml().h")
*/

// -----------------------------------------------------------------------------
// glue code
// ----------------------------------------------------------------------------

//#define alloca_      ifdef(KIT_CL, _alloca, alloca)
//#define atoi64_      ifdef(KIT_CL, _atoi64, atoll/*atoi64*/)
//#define chdir_       ifdef(KIT_CL, _chdir, chdir)
#define flockfile_     ifdef(KIT_CL, _lock_file, ifdef(KIT_MINGW,_lock_file,flockfile))
//#define ftruncate_   ifdef(KIT_CL, _chsize_s, ftruncate)
#define funlockfile_   ifdef(KIT_CL, _unlock_file, ifdef(KIT_MINGW,_unlock_file,funlockfile))
//#define mkdir_(p)    ifdef(KIT_WINDOWS, mkdir(p), mkdir(p,0777)) // @todo: win32 function is recursive, probably counterparts should be as well
//#define pclose_      ifdef(KIT_CL, _pclose, pclose)
//#define popen_       ifdef(KIT_CL, _popen, popen)
//#define restrict_    ifdef(KIT_WINDOWS, __restrict, __restrict__)
#define strcmpi_       ifdef(KIT_WINDOWS, _strcmpi, strcasecmp)
#define strncmpi_      ifdef(KIT_WINDOWS, strnicmp, strncasecmp)
//#define thread_      ifdef(KIT_CL, __declspec(thread), __thread)
//#define unlink_      ifdef(KIT_WINDOWS, _unlink, unlink)

// -----------------------------------------------------------------------------
// our headers
// -----------------------------------------------------------------------------

#include "kit.common.h"
#include "kit.ifndef.h"
#include "kit.macros.h"
#include "kit.test.h"

int  os_dialog(const char *head, const char *body, int buttons, ...);
void os_fatal(const char *msg);
void os_die(const char *msg);
void os_log(const char *fmt, ...);
char *file_read(const char *url, int *len);

#include "kit.zrealloc.h" //< string||alloc
#include "kit.alloc.h"
#include "kit.string.h" //< string
#include "kit.string.strstri.h" //< string
//< kit.string.h : fmt, mem, eval, log,

#include "kit.array.h" //< ds
#include "kit.bitpool.h" //< ds
#include "kit.hash.h" //< ds
#include "kit.map.h" //< ds
#include "kit.stable.h" //< ds

#include "kit.enum.h" //< obj
#include "kit.make.h"
#include "kit.module.h"
#include "kit.reflect.h"
#include "kit.serial.h"

#include "kit.ini.h"
#include "kit.obj.h"
#include "kit.objptr.h"

#include "kit.argcv.h"

#include "kit.hexdump.h" //< debug
#include "kit.trace.h" //< debug
