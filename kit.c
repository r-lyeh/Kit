#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#ifndef _CRT_NONSTDC_NO_DEPRECATE 
#define _CRT_NONSTDC_NO_DEPRECATE 
#endif

#include "kit.h"
#define KIT_CODE 1
#include "kit.h"

void        ansi_off_(void) { printf("\033[0m"); } // default color
#include    ifdef(KIT_WINDOWS, <winsock2.h>, <stdlib.h>)
AUTORUN{    atexit(ansi_off_); ifdef(KIT_WINDOWS,{ DWORD mode = 0; SetConsoleMode(GetStdHandle(-11), (GetConsoleMode(GetStdHandle(-11), &mode), mode|4)); WinExec("chcp 65001 >nul", SW_HIDE); }); } // enable ansi colors + unicode codepage. safe to call from dll (WinExec)

            // @todo: save_on_reload();
            // @todo: SDL_AddEventWatch + SDL_EVENT_WINDOW_EXPOSED to know when to redraw our window from within the callback so it doesn't stretch or look overly unnatural.
            void (*os_app)(event); array_(main_t) mains; typedef SDL_AppResult SDLres;
            int app_cmp( const void *a, const void *b ) {
                int rc = strcmp( ((main_t*)a)->name, ((main_t*)b)->name );
                if(!rc) rc = ((main_t*)a)->id - ((main_t*)b)->id;
                return rc;
            }
SDLres      SDL_AppIterate(void *appstate) {
                kit.loop(0);   static unsigned counter = 0; counter = (counter + 1) % 10;
                               if(os_app) os_app(app.event = (event){.frame = app.event.frame+1, .tick = render.delta()});
                if(counter==0) if(os_app) os_app(app.event = (event){.frame = app.event.frame+0, .step = 1.0/SDL_fmod(display.hz(0),10)});
                input_next_frame(); // kit.post();
                return os_app ? SDL_APP_CONTINUE : SDL_APP_SUCCESS; }
SDLres      SDL_AppEvent(void *appstate, SDL_Event *event_) { 
                SDL_ConvertEventToRenderCoordinates(render.handle, event_);
                input_pump_event(event_); // kit.event(event_);
                if(os_app) os_app(app.event = (event){.frame = app.event.frame, .emit = event_, .type = event_->type });
                return os_app ? SDL_APP_CONTINUE : SDL_APP_SUCCESS; }
void        SDL_AppQuit(void *appstate, SDL_AppResult result) {
                if( 0 == SDL_strcmp(strvalid(SDL_getenv("USERDOMAIN")), "DESKTOP-GPTNFKB") ) //< dev-only
                if( render.handle ) {
                    char url[64] = {0};
                    for( int i = 0; i < 999; ++i ) {
                        SDL_snprintf(url, 64, ".screenshot%03d.png", i);
                        if( file.exists(url) ) continue;
                        render.save(url);
                        break;
                    }
                }
            }
SDLres      SDL_AppInit(void **appstate, int argc, char *argv[]) {
                __argc = argc;
                __argv = argv;

                qsort(mains, array_count(mains), sizeof(mains[0]), app_cmp);

                int cnt = array_count(mains); cnt += !cnt;
                int num = atoi(os.arg("--main=-1")); 
                int idx = ((num % cnt) + cnt) % cnt;
                if( idx >= 0 && idx < array_count(mains) ) {
                    os_app = mains[idx].fn;
                    if( os_app ) os_app( app.event = (event){.frame = 0, .init = 1} );
                }

                return os_app ? SDL_APP_CONTINUE : SDL_APP_SUCCESS;
            }
void        app_quit(int rc) { if(rc) os.log("%s", SDL_GetError()); fflush(0); if(window.handle) window.show(0); if(app.event.init || app.event.quit) exit(rc); if(os_app) os_app(app.event = (event){.quit = 1}); os_app = NULL; }
void        app_reload(void) { /* chdir(app_path()); */ fflush(0); execv(__argv[0], (const char *const *)__argv); exit(-1); }
struct      app app = { .quit = app_quit, .reload = app_reload };

array_(char*) archive_dir(const char *filter, int *count) {
                static SDL_Storage *storage = NULL;
                if( storage == NULL ) {
                    storage = SDL_OpenTitleStorage(NULL, 0/*|SDL_STORAGE_READ*/);
                    if( storage == NULL ) {
                        SDL_Log("Unable to open storage %s", SDL_GetError());
                        return NULL;
                    }
                }
                static THREAD array_(char*) iter = 0;
                for each_array(iter, i) if(iter[i]) SDL_free(iter[i]);
                array_resize(iter, 0);
                char **results = SDL_GlobStorageDirectory(storage, "", filter && strstr(filter,"**") ? NULL : "*", 0, NULL);
                for (int i = 0; results[i]; i++) {
                    const char *entry = results[i];
                    if( entry[0] == '.' ) continue; // hide special files and folders (".git",".svn",etc.)
                    if( filter && !strmatchi(entry, filter) ) continue;

                    SDL_PathInfo info;
                    if (!SDL_GetStoragePathInfo(storage, entry, &info)) {
                        SDL_Log("GetStoragePathInfo failed (%s): %s", entry, SDL_GetError());
                    } else {
                        if (info.type != SDL_PATHTYPE_DIRECTORY) {
                            array_push(iter, SDL_strdup(entry));
                        }
                    }
                }
                SDL_free(results);

                array_sortstri(iter);
                array_push(iter, NULL);
                array_pop(iter);
                if(count) *count = array_count(iter);
                return iter;
            }
struct      archive archive = { .dir = archive_dir };

int         assert_debug(bool expr) { SDL_assert(expr); return expr; }
int         assert_release(bool expr) { SDL_assert_release(expr); return expr; }
void        assert_breakpoint(void) { abort(); } // SDL_assert_release(!"breakpoint() call"); }
int         assert_enabled(void) { int enabled = 0; SDL_assert( enabled |= 1 ); return enabled; }
struct      assert assert = { .debug = assert_debug, .release = assert_release, .breakpoint = assert_breakpoint, .enabled = assert_enabled };

struct      audio audio = { .open = al_audio_new, .duration = al_audio_duration, .close = al_audio_free };
AUTORUN{    if( !al_open() ) os_die("cannot init audio"); }

int         battery_time(void) { int secs, pct, state = SDL_GetPowerInfo(&secs, &pct); SDL_CHECK(state != SDL_POWERSTATE_ERROR); return secs <= 0 ? 0 : secs; }
int         battery_power(void) { int secs, pct, state = SDL_GetPowerInfo(&secs, &pct); SDL_CHECK(state != SDL_POWERSTATE_ERROR); return pct <= 0 ? 0 : pct; }
bool        battery_inuse(void) { int secs, pct, state = SDL_GetPowerInfo(&secs, &pct); SDL_CHECK(state != SDL_POWERSTATE_ERROR); return state == SDL_POWERSTATE_ON_BATTERY; }
struct      battery battery = { .time = battery_time, .power = battery_power, .inuse = battery_inuse };

void        clipboard_set(text contents) { bool ok = SDL_SetClipboardText(contents); SDL_CHECK(ok); }
text        clipboard_get(void) { static char *last = 0; if(last) SDL_free(last); /*< @leak */ return last = SDL_GetClipboardText(), strvalid(last); }
bool        clipboard_empty(void) { return SDL_HasClipboardText(); }
struct      clipboard clipboard = { .set = clipboard_set, .get = clipboard_get, .empty = clipboard_empty };

#define     H(c)((s[c]&15)+(s[c]>>6)*9)
union       rgba_ { struct { uint8_t r,g,b,a;}; unsigned rgba; };
unsigned    rgba_(uint8_t r,uint8_t g,uint8_t b,uint8_t a) { union rgba_ x = {r,g,b,a}; return x.rgba; }
unsigned    color_hex(text s) { if (s[0] == '#' && (s[4] == 0 || s[5] == 0)) return((union rgba_){ H(1)*0x11,H(2)*0x11,H(3)*0x11,s[4] ? H(4)*0x11 : 255 }).rgba; return!(s[0] == '#' && (s[7] == 0 || s[9] == 0)) ? 0 : ((union rgba_){ H(1)*16+H(2),H(3)*16+H(4),H(5)*16+H(6),s[7] ? H(7)*16+H(8) : 255 }).rgba; } // "#rgb" "#rgba" "#rrggbb" "#rrggbbaa"
unsigned    color_hsva(float h255, float s255, float v255, float a255) { uint8_t h = (uint8_t)h255, s = (uint8_t)s255, v = (uint8_t)v255; uint8_t i = h/43, f = (h%43)*6, p = (v*(255-s))>>8, q = (v*(255-((s*f)>>8)))>>8, t = (v*(255-((s*(255-f))>>8)))>>8; return ((union rgba_){ (i==0||i==5)?v:(i==1)?q:(i==2||i==3)?p:t, (i==0)?t:(i==1||i==2)?v:(i==3)?q:p, (i==0)?p:(i==1)?p:(i==2)?t:(i==3||i==4)?v:q, (uint8_t)a255 }).rgba; }
unsigned    color_hsv(float h255, float s255, float v255) { return color_hsva(h255,s255,v255,255); }
unsigned    color_rgba(float r255, float g255, float b255, float a255) { return rgba_(r255,g255,b255,a255); }
unsigned    color_rgb(float r255, float g255, float b255) { return rgba_(r255,g255,b255,255); }
uint8_t     color_r(unsigned color) { return (color >>  0) & 255; }
uint8_t     color_g(unsigned color) { return (color >>  8) & 255; }
uint8_t     color_b(unsigned color) { return (color >> 16) & 255; }
uint8_t     color_a(unsigned color) { return (color >> 24) & 255; }
struct      color color = { .r = color_r, .g = color_g, .b = color_b, .a = color_a, .rgb = color_rgb, .rgba = color_rgba, .hex = color_hex, .hsv = color_hsv, .hsva = color_hsva, };
#undef      H

int         dialog_prompt(const char* head, const char* body, int buttons, ...) {
                int severe = buttons < 0; buttons = SDL_abs(buttons);
                if (buttons < 1 || buttons > 5) { // too many buttons?
                    return 0;
                }

                SDL_MessageBoxButtonData* btn_data = SDL_malloc(buttons * sizeof(SDL_MessageBoxButtonData));
                if (!btn_data) return 0;

                bool callstack = 0;
                head = strvalid(head); callstack |= head[0] == '!'; head += strspn(head, "!");
                body = strvalid(body); callstack |= body[0] == '!'; body += strspn(body, "!");
                if( 0 && SDL_strlen(head) >= SDL_strlen(body) ) {
                    const char *swap = head; head = body; body = swap;
                }
                if( callstack ) body = va("%s\n\n%s", body, trace(16));

                va_list args;
                va_start(args, buttons);

                for (int i = 0; i < buttons; ++i) {
                    const char* text = va_arg(args, const char*);
                    btn_data[i].flags = (i == 0) ? SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT : 0;
                    btn_data[i].buttonID = i + 1; // 1-based index
                    btn_data[i].text = text ? text : "Button";
                }

                va_end(args);

                SDL_MessageBoxFlags flags = severe ||
                    strstri(head, "fail") || strstri(head, "err")   || strstri(head, "fatal")  ? SDL_MESSAGEBOX_ERROR :
                    strstri(head, "warn") || strstri(head, "alert") || strstri(head, "cann") || strstri(head, "couldn") ? SDL_MESSAGEBOX_WARNING :
                    SDL_MESSAGEBOX_INFORMATION;

                SDL_MessageBoxData messageboxdata = {
                    .flags = flags,
                    .window = NULL,                         // no parent window (or pass your window)
                    .title = head ? head : "Message",
                    .message = body ? body : "",
                    .numbuttons = buttons,
                    .buttons = btn_data,
                    .colorScheme = NULL
                };

                int button_id = 0;
                bool ok = SDL_ShowMessageBox(&messageboxdata, &button_id);
                SDL_assert_release(ok); //< do not use SDL_CHECK() here, since that uses us (os_dialog)
                SDL_free(btn_data);

                return ok && button_id > 0 ? button_id : 0; // returns 1, 2, 3... or 0 on error
            }
void        dialog_alert(text msg) { dialog_prompt("Alert", msg, 1, "Ok"); }
void        dialog_fatal(text msg) { dialog_prompt("Fatal", va("%s\n\n%s", msg, trace(16)), -1, "Ok"); }
            // @todo: SDL_DialogFileFilter filters[] = { { .name = "Office document", .pattern = "doc;docx" } }; // also "*"
void        dialog_callback(void *userdata, const char * const *filelist, int filter) { if(filelist) while(*filelist) { static char *s = 0; if(s) SDL_free(s); *((text*)userdata) = s = strdup(*filelist); filelist++; return; } *((text*)userdata) = ""; }
void        dialog_folder(text *path)   { if(path) SDL_ShowOpenFolderDialog(dialog_callback, path/*userdata*/, NULL/*window*/, NULL/*default_location*/, false/*allow_many*/); }
void        dialog_loadfile(text *name) { if(name)   SDL_ShowOpenFileDialog(dialog_callback, name/*userdata*/, NULL/*window*/, NULL/*filters*/, 0/*num_filters*/, NULL/*default_location*/, false/*allow_many*/); }
void        dialog_savefile(text *name) { if(name)   SDL_ShowSaveFileDialog(dialog_callback, name/*userdata*/, NULL/*window*/, NULL/*filters*/, 0/*num_filters*/, NULL/*default_location*/); }
struct      dialog dialog = { .loadfile = dialog_loadfile, .savefile = dialog_savefile, .folder = dialog_folder, .prompt = dialog_prompt, .alert = dialog_alert, .fatal = dialog_fatal, };

            // SDL_GetCurrentDisplayOrientation(did);
            static SDL_DisplayID did(int index) {
                static int count = 0;
                static SDL_DisplayID *ids = 0; if(!ids) ids = SDL_GetDisplays(&count); /*< @leak */
                while( index < 0 ) index += (count + !count);
                return index ? ids[ index % (count + !count) ] : SDL_GetPrimaryDisplay();
            }
unsigned    display_count(void) { int count; SDL_DisplayID *ids = SDL_GetDisplays(&count); SDL_free(ids); return count; }
text        display_name(unsigned monitor) { return strvalid(SDL_GetDisplayName(did(monitor))); }
int         display_width(unsigned monitor) { return SDL_GetDesktopDisplayMode(did(monitor))->w; }
int         display_height(unsigned monitor) { return SDL_GetDesktopDisplayMode(did(monitor))->h; }
int         display_bpp(unsigned monitor) { return SDL_BITSPERPIXEL(SDL_GetDesktopDisplayMode(did(monitor))->format); }
float       display_hz(unsigned monitor) { return SDL_GetDesktopDisplayMode(did(monitor))->refresh_rate; }
float       display_scale(unsigned monitor) { return SDL_GetDisplayContentScale(did(monitor)); }
float       display_dpi(unsigned monitor) { return display_scale(monitor) * ifdef(KIT_MOBILE, 96.f, 160.f); }
float4      display_workarea(unsigned monitor) { float4 workarea = {0}; SDL_Rect rect; if (SDL_GetDisplayUsableBounds(did(monitor), &rect)) workarea = (float4){ (float)rect.x, (float)rect.y, (float)rect.w, (float)rect.h }; return workarea; }
struct      display display = { .count = display_count, .name = display_name, .width = display_width, .height = display_height, .bpp = display_bpp, .hz = display_hz, .scale = display_scale, .dpi = display_dpi, .workarea = display_workarea, };

static      map_(char*,SDL_SharedObject*) dlls = {less,hash};
void*       dll_load(const char *file, const char *func) { file = stable(file); SDL_SharedObject** found = map_find(dlls, (char*)file), *lib = NULL; lib = found ? *found : SDL_LoadObject(file); if(!found) map_add(dlls, (char*)file, lib); return lib ? SDL_LoadFunction(lib, func) : NULL; }
void        dll_unload(const char *file) { file = stable(file); SDL_SharedObject** found = map_find(dlls, (char*)file); if( found ) SDL_UnloadObject(*found), map_add(dlls, (char*)file, NULL); }
struct      dll dll = { .load = dll_load, .unload = dll_unload };

uint64_t    elapsed_dt(void) { static SDL_Time t[2] = {0}; return t[0] ? (SDL_GetCurrentTime(t+1), t[1] - t[0]) : (SDL_CHECK(SDL_GetCurrentTime(t+0)), t[0]); }
uint64_t    elapsed_ns(void) { return elapsed_dt(); }
uint64_t    elapsed_us(void) { return elapsed_ns() / 1000; }
double      elapsed_ms(void) { return elapsed_ns() / 1e6; }
double      elapsed_ss(void) { return elapsed_ns() / 1e9; }
double      elapsed_mm(void) { return (int)(elapsed_ss() /   60) % 60; }
double      elapsed_hh(void) { return (int)(elapsed_ss() / 3600) % 24; }
struct      elapsed elapsed = { .hh = elapsed_hh, .mm = elapsed_mm, .ss = elapsed_ss, .ms = elapsed_ms, .us = elapsed_us, .ns = elapsed_ns, };
AUTORUN{    elapsed_ns(); }

int         endian_big(void) { return SDL_BYTEORDER == SDL_BIG_ENDIAN; }
int         endian_little(void) { return SDL_BYTEORDER == SDL_LIL_ENDIAN; }
float       endian_swapf(float x) { return SDL_SwapFloat(x); }
uint16_t    endian_swap16(uint16_t x) { return SDL_Swap16(x); }
uint32_t    endian_swap32(uint32_t x) { return SDL_Swap32(x); }
uint64_t    endian_swap64(uint64_t x) { return SDL_Swap64(x); }
struct      endian endian = { .big = endian_big, .little = endian_little, .swapf = endian_swapf, .swap16 = endian_swap16, .swap32 = endian_swap32, .swap64 = endian_swap64, };

data        file_read(text url, int *len) { size_t len_; char *data = SDL_LoadFile(url, &len_); if(len) *len = data ? (int)len_ : 0; return data; }
bool        file_write(text url, const void *data, int len) { return SDL_SaveFile(url, data, (size_t)len); }
bool        file_exists(text url) { SDL_PathInfo info; return SDL_GetPathInfo(url, &info) ? info.type == SDL_PATHTYPE_FILE : 0; }
int         file_size(text url) { SDL_PathInfo info; if(SDL_GetPathInfo(url, &info)) if(info.type == SDL_PATHTYPE_FILE) return info.size; return 0; }
struct      file file = { .read = file_read, .write = file_write, .exists = file_exists, .size = file_size };

text        folder_home(void) { return strvalid(SDL_GetUserFolder(SDL_FOLDER_HOME)); }
text        folder_desktop(void) { return strvalid(SDL_GetUserFolder(SDL_FOLDER_DESKTOP)); }
text        folder_documents(void) { return strvalid(SDL_GetUserFolder(SDL_FOLDER_DOCUMENTS)); }
text        folder_downloads(void) { return strvalid(SDL_GetUserFolder(SDL_FOLDER_DOWNLOADS)); }
text        folder_music(void) { return strvalid(SDL_GetUserFolder(SDL_FOLDER_MUSIC)); }
text        folder_pictures(void) { return strvalid(SDL_GetUserFolder(SDL_FOLDER_PICTURES)); }
text        folder_shared(void) { return strvalid(SDL_GetUserFolder(SDL_FOLDER_PUBLICSHARE)); }
text        folder_savedgames(void) { return strvalid(SDL_GetUserFolder(SDL_FOLDER_SAVEDGAMES)); }
text        folder_screenshots(void) { return strvalid(SDL_GetUserFolder(SDL_FOLDER_SCREENSHOTS)); }
text        folder_templates(void) { return strvalid(SDL_GetUserFolder(SDL_FOLDER_TEMPLATES)); }
text        folder_videos(void) { return strvalid(SDL_GetUserFolder(SDL_FOLDER_VIDEOS)); }
text        folder_install(void) { return strvalid(SDL_GetBasePath()); }
text        folder_storage(text company, text appname) { return strvalid(SDL_GetPrefPath(company,appname)); }
text        folder_working(void) { return strvalid(SDL_GetCurrentDirectory()); }
struct      folder folder = { .home = folder_home, .desktop = folder_desktop, .documents = folder_documents, .downloads = folder_downloads, .music = folder_music, .pictures = folder_pictures, .shared = folder_shared, .savedgames = folder_savedgames, .screenshots = folder_screenshots, .templates = folder_templates, .videos = folder_videos, .install = folder_install, .storage = folder_storage, .working = folder_working };

            array_(array_(TTF_Font*)) fonts;
unsigned    font_open(float ptsize, const char *paths) {
                if( !TTF_WasInit() ) if( !TTF_Init() ) os.die("cannot init ttf library");
                // collection of handles: [0] main font, [1,2,3...] fallback fonts
                array_(TTF_Font*) handles = {0};
                for each_string(file, paths, ",;|\t\n") {
                    TTF_Font *f = TTF_OpenFont(file, ptsize);
                    if(!f) os.die("cannot load truetype file");
                    array_push(handles, f);
                    if( array_count(handles) > 1 )
                    if( !TTF_AddFallbackFont(handles[0], f) ) os.die("");
                }
                //    font = TTF_OpenFontIO(SDL_IOFromConstMem(tiny_ttf, tiny_ttf_len), true, 18.0f * 4);
                TTF_Font *font = handles[0];
                TTF_SetFontDirection(font, TTF_DIRECTION_LTR);
                TTF_SetFontKerning(font, true);
                TTF_SetFontSDF(font, atoi(os.arg("--sdf")));  // not working. needs sdlgpu?
                TTF_SetFontHinting(font, TTF_HINTING_NORMAL);
                ONCE array_push(fonts, NULL);
                array_push(fonts, handles);
                return array_count(fonts) - 1;
            }
void        font_close(unsigned *x) {
                unsigned idx = *x;
                for(int i = 0, end = array_count(fonts[idx]); i < end; ++i)
                    if(fonts[idx][i]) TTF_CloseFont(fonts[idx][i]);
                array_free(fonts[idx]);
                *x = 0;
            }
TTF_Font*   font_handle(unsigned h) { return fonts[h][0]; }
SDL_Surface*font_bake(unsigned h, unsigned quality03, unsigned fg, unsigned bg, int width, const char *message) { // width: <0 unlimited, 0 break on linefeeds, >0 specific width in pixels
                TTF_Font *font = font_handle(h);
                SDL_Color black = { color.r(fg), color.g(fg), color.b(fg), color.a(fg) };
                SDL_Color white = { color.r(bg), color.g(bg), color.b(bg), color.a(bg) };
                // [0..3] = fast,good,high/sdf,alt qualities
                SDL_Surface *text = NULL;
                if(quality03 == 0) text = TTF_RenderText_Solid_Wrapped(font, message, 0, black, width < 0 ? INT_MAX : width);
                if(quality03 == 1) text = TTF_RenderText_Shaded_Wrapped(font, message, 0, black, white, width < 0 ? INT_MAX : width);
                if(quality03 == 2) text = TTF_RenderText_Blended_Wrapped(font, message, 0, black, width < 0 ? INT_MAX : width);
                if(quality03 == 3) text = TTF_RenderText_LCD_Wrapped(font, message, 0, black, white, width < 0 ? INT_MAX : width);
                return text;
            }
float       font_size(unsigned handle, float size) { TTF_Font *font = font_handle(handle); if( size > 0 ) TTF_SetFontSize(font, size); return TTF_GetFontSize(font); }
bool        font_bold(unsigned handle, int bold) { TTF_Font *font = font_handle(handle); if( bold >= 0 && bold <= 1 ) TTF_SetFontStyle(font, bold ? TTF_GetFontStyle(font) | TTF_STYLE_BOLD : TTF_GetFontStyle(font) & ~TTF_STYLE_BOLD); return TTF_GetFontStyle(font) & TTF_STYLE_BOLD; }
bool        font_underline(unsigned handle, int underline) { TTF_Font *font = font_handle(handle); if( underline >= 0 && underline <= 1 ) TTF_SetFontStyle(font, underline ? TTF_GetFontStyle(font) | TTF_STYLE_UNDERLINE : TTF_GetFontStyle(font) & ~TTF_STYLE_UNDERLINE); return TTF_GetFontStyle(font) & TTF_STYLE_UNDERLINE; }
bool        font_italic(unsigned handle, int italic) { TTF_Font *font = font_handle(handle); if( italic >= 0 && italic <= 1 ) TTF_SetFontStyle(font, italic ? TTF_GetFontStyle(font) | TTF_STYLE_ITALIC : TTF_GetFontStyle(font) & ~TTF_STYLE_ITALIC); return TTF_GetFontStyle(font) & TTF_STYLE_ITALIC; }
bool        font_strike(unsigned handle, int strike) { TTF_Font *font = font_handle(handle); if( strike >= 0 && strike <= 1 ) TTF_SetFontStyle(font, strike ? TTF_GetFontStyle(font) | TTF_STYLE_STRIKETHROUGH : TTF_GetFontStyle(font) & ~TTF_STYLE_STRIKETHROUGH); return TTF_GetFontStyle(font) & TTF_STYLE_STRIKETHROUGH; }
int         font_outline(unsigned handle, int outline) { TTF_Font *font = font_handle(handle); if( outline >= 0 ) TTF_SetFontOutline(font, outline); return TTF_GetFontOutline(font); }
char        font_align(unsigned handle, const char *mode) { TTF_Font *font = font_handle(handle); if( mode ) if( mode[0] == 'l' || mode[0] == 'c' || mode[0] == 'r' ) TTF_SetFontWrapAlignment(font, mode[0] == 'l' ? TTF_HORIZONTAL_ALIGN_LEFT : mode[0] == 'c' ? TTF_HORIZONTAL_ALIGN_CENTER : TTF_HORIZONTAL_ALIGN_RIGHT); int align = TTF_GetFontWrapAlignment(font); return align == TTF_HORIZONTAL_ALIGN_LEFT ? 'l' : align == TTF_HORIZONTAL_ALIGN_CENTER ? 'c' : 'r'; }
char        font_direction(unsigned handle, const char *mode) { TTF_Font *font = font_handle(handle); int modes[] = { ['l'] = TTF_DIRECTION_LTR, ['r'] = TTF_DIRECTION_RTL, ['t'] = TTF_DIRECTION_TTB, ['b'] = TTF_DIRECTION_BTT, [TTF_DIRECTION_LTR] = 'l', [TTF_DIRECTION_RTL] = 'r', [TTF_DIRECTION_TTB] = 't', [TTF_DIRECTION_BTT] = 'b' }; if( mode ) if( SDL_strchr("lrbt", mode[0] ) ) TTF_SetFontDirection(font, modes[mode[0]]); return modes[ TTF_GetFontDirection(font) ]; }
int         font_spacing(unsigned handle, float spacing) { TTF_Font *font = font_handle(handle); if(spacing==(int)spacing) TTF_SetFontCharSpacing(font, spacing); return TTF_GetFontCharSpacing(font); }
char        font_hinting(unsigned handle, const char *mode) { TTF_Font *font = font_handle(handle); int modes[] = { ['0'] = TTF_HINTING_NONE, [TTF_HINTING_NONE] = '0', ['l'] = TTF_HINTING_LIGHT, [TTF_HINTING_LIGHT] = 'l', ['m'] = TTF_HINTING_MONO, [TTF_HINTING_MONO] = 'm', ['n'] = TTF_HINTING_NORMAL, [TTF_HINTING_NORMAL] = 'n', ['s'] = TTF_HINTING_LIGHT_SUBPIXEL, [TTF_HINTING_LIGHT_SUBPIXEL] = 's' }; if( mode ) if( SDL_strchr("0lmns", mode[0] ) ) TTF_SetFontHinting(font, modes[mode[0]]); return modes[ TTF_GetFontHinting(font) ]; } // (s) not working
struct      font font = { .open = font_open, .size = font_size, .bold = font_bold, .italic = font_italic, .strike = font_strike, .underline = font_underline, .outline = font_outline, .spacing = font_spacing, .align = font_align, .direction = font_direction, .hinting = font_hinting, .bake = font_bake, .handle = font_handle, .close = font_close, };

            #define FX_C
            #include "kits/kit.fx/kit.h"
bool        fx_open(void) { bool rc = fx_init(); if(rc) fx.handle = FX.dev; return rc; }
void        fx_close(void) { fx_destroy(); fx.handle = NULL; }
struct      fx fx = { .open = fx_open, .close = fx_close, .begin = fx_begin, .end = fx_end, .add = fx_add, .enable = fx_enable, .uniform = fx_uniform, };

            #if __has_include("3rd/3rd_stb_image.h") // JPG, PNG, BMP, TGA, PSD, GIF
            #include "3rd/3rd_stb_image.h"
            #endif
            #if __has_include("3rd/3rd_koi_image.h") // QOI
            #include "3rd/3rd_koi_image.h"
            #endif
            #if __has_include("3rd/3rd_tiny_webp.h") // WEBP
            #include "3rd/3rd_tiny_webp.h"
            #endif
            #if __has_include("3rd/3rd_stb_image_write.h")
            #include "3rd/3rd_stb_image_write.h"
            #endif
void*       image_open(url path, int *w, int *h, int *n) {
                path = va("%s%s", SDL_GetBasePath(), path);
                void *pixels = 0;
                #if __has_include("3rd/3rd_stb_image.h")
                if( !pixels ) {
                    int req = n ? *n : 0;
                    pixels = stbi_load(path, w, h, n, req);
                    if( pixels ) if(n) *n = req ? req : *n;
                }
                #endif
                #if __has_include("3rd/3rd_koi_image.h")
                if( !pixels ) {
                    int req = n ? *n : 0;
                    pixels = koi_load(path, w, h, n, req);
                    if( pixels ) if(n) *n = req ? req : *n;
                }
                #endif
                #if __has_include("3rd/3rd_tiny_webp.h")
                if( !pixels ) {
                    int req = n && *n == 3 ? twp_FORMAT_RGB : twp_FORMAT_RGBA;
                    pixels = twp_read(path, w, h, req, 0); // no flags
                    if( pixels ) if(n) *n = req == twp_FORMAT_RGB ? 3 : 4;
                }
                #endif
                /*
                if( i.flip ) for( int y = 0, e = h - 1, w = w * n[0]; y <= e; ++y )
                SDL_memcpy(&pixel8[0+(e-y)*w], &data[0+y*w], w);
                else
                SDL_memcpy(data, data, w * h * n[0]);
                */
                return pixels ? pixels : (*w = *h = *n = 0, NULL);
            }
void        image_close(void **pixels) { if( pixels ) SDL_free(*pixels), *pixels = NULL; }
struct      image image = { .open = image_open, .close = image_close };


            //#include "3rd/hey_imgui.h"
bool        imgui_open(void) {
                if (!SDL_WasInit(SDL_INIT_VIDEO|SDL_INIT_GAMEPAD)) if (!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMEPAD)) {
                    printf("Error: SDL_Init(): %s\n", SDL_GetError());
                    app.quit(-1);
                }

                // Create window with SDL_Renderer graphics context
                float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

                // Setup Dear ImGui context
                //IMGUI_CHECKVERSION();
                igCreateContext(0);
                ImGuiIO* io = igGetIO_Nil();
                io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
                io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

                // Setup Dear ImGui style
                igStyleColorsDark(0);
                //igStyleColorsLight();

                // Setup scaling
                ImGuiStyle *style = igGetStyle();
                ImGuiStyle_ScaleAllSizes(style, main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
                style->FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io->ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)

                // Setup Platform/Renderer backends
                ImGui_ImplSDL3_InitForSDLRenderer(window.handle, render.handle);
                ImGui_ImplSDLRenderer3_Init(render.handle);

                // Load Fonts
                // - If fonts are not explicitly loaded, Dear ImGui will select an embedded font: either AddFontDefaultVector() or AddFontDefaultBitmap().
                //   This selection is based on (style->FontSizeBase * style->FontScaleMain * style->FontScaleDpi) reaching a small threshold.
                // - You can load multiple fonts and use igPushFont()/PopFont() to select them.
                // - If a file cannot be loaded, AddFont functions will return a nullptr. Please handle those errors in your code (e.g. use an assertion, display an error and quit).
                // - Read 'docs/FONTS.md' for more instructions and details.
                // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use FreeType for higher quality font rendering.
                // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
                //style->FontSizeBase = 20.0f;
                //io->Fonts->AddFontDefaultVector();
                //io->Fonts->AddFontDefaultBitmap();
                //io->Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
                //io->Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
                //io->Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
                //io->Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
                //ImFont* font = io->Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
                //IM_ASSERT(font != nullptr);

                #ifdef __EMSCRIPTEN__
                // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
                // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
                io->IniFilename = nullptr;
                #endif

                // kit styling
        #if 1 // def IMGUI_HAS_DOCK
                io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;       // Enable Docking
                //io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;     // Enable Multi-Viewport / Platform Windows
        //#define IMGUI_SETTINGS     io.IniFilename = option("--ui.ini", ".settings.ini");
        //#define IMGUI_DEPS_A       igFileDialogInit(), ImPlot_CreateContext(), ImPlot_SetImGuiContext(ctx);
        #endif
                igThemeKit(6,4,0,0,0,1,1,0);
                igLoadFonts();

                return true;
            }
bool        imgui_close(void) {
                ImGui_ImplSDLRenderer3_Shutdown();
                ImGui_ImplSDL3_Shutdown();
                igDestroyContext(0);
                return true;    
            }
void        imgui_begin(void) {
                ImGui_ImplSDLRenderer3_NewFrame();
                ImGui_ImplSDL3_NewFrame();
                igNewFrame();
                //void igNotifyDemo(void); igNotifyDemo();
                void igNotifyUpdate(void); igNotifyUpdate();
            }
void        imgui_end(void) {
                igRender(); //< commands
                ImGuiIO* io = igGetIO_Nil();
                SDL_SetRenderScale(render.handle, io->DisplayFramebufferScale.x, io->DisplayFramebufferScale.y);
                ImGui_ImplSDLRenderer3_RenderDrawData(igGetDrawData(), render.handle); //< actual rendering
                #if 0 // def IMGUI_HAS_DOCK
                if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable) 
                {
                    GLFWwindow *backup_current_window = glfwGetCurrentContext();
                    igUpdatePlatformWindows();
                    igRenderPlatformWindowsDefault(NULL, NULL);
                    glfwMakeContextCurrent(backup_current_window);
                }
                #endif
            }
SDL_Event*  imgui_event(SDL_Event *event) {
                // Poll and handle events (inputs, window resize, etc.)
                // You can read the io->WantCaptureMouse, io->WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
                // - When io->WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
                // - When io->WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
                // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
                return ImGui_ImplSDL3_ProcessEvent(event) ? NULL : event;
            }
struct      imgui imgui = { .open = imgui_open, .close = imgui_close, .event = imgui_event, .begin = imgui_begin, .end = imgui_end };

  int       kit_fps;
float       kit_dt, kit_frequency;
bool        kit_loop(bool pump) {
                double now = elapsed.ss(), mul = 2;

                // frame limiter/locker: targetfps [-1=adaptive] [0=off] [1=vsync] [>1=Nth capped]
                if( kit_frequency > 1 ) {

                    static double then = 0; double dt = now - then; then = now;
                    if( dt > 1 ) dt = 1; kit_dt = dt; dt *= 1000;
                    #if 0
                    float target = 1000./(kit_frequency+0.5); // @fixme: +2 fixes some occasional tearing, but still not perfect. why?
                    if( dt < target ) SDL_DelayPrecise(mul*(target - dt)*1e6); // ms-to-ns //@fixme: why 2* ?
                    #else
                    static double accumulator = 0.0; accumulator += kit_dt;
                    const double target = 1.0 / kit_frequency;

                    if( accumulator < target ) {
                        double sleep_sec = target - accumulator;
                        if( sleep_sec > 0.001 )
                        SDL_DelayPrecise((uint64_t)(sleep_sec * 1e9));
                    }

                    accumulator -= target;
                    if( accumulator > 0.1 ) accumulator = 0.1; // anti-spiral
                    #endif
                }

                // frame counter + title
                static unsigned fps0 = 0, fps1 = 0, fps_count = 0;
                static double old, horizon; ONCE horizon = now, old = now;
                kit_dt = now - old; old = now;
                while (horizon + 0.5 < now) {
                    horizon += 0.5;
                    fps0 = fps1;
                    fps1 = fps_count;
                    fps_count = 0;
                }
                if( !fps_count ) {
                    kit_fps = fps0 + fps1;
                    if(window.handle) {
                    const char *title = window.title(NULL);
                    title = title[0] ? title : strvalid(SDL_GetAppMetadataProperty(SDL_PROP_APP_METADATA_NAME_STRING));
                    const char *sep = strstr(title, " |");
                    SDL_SetWindowTitle(window.handle, va("%.*s%s%02u fps", (int)(sep ? sep-title : SDL_strlen(title)), title, title[0] ? " | ":"", kit_fps));
                    }
                }
                ++fps_count;

                if( pump ) SDL_PumpEvents(); // { SDL_Event event; while(SDL_PollEvent(&event)); }
                int count = SDL_PeepEvents(NULL, 0, SDL_PEEKEVENT, SDL_EVENT_QUIT, SDL_EVENT_QUIT);
                return count > 0 ? false : true;
            }
void        kit_post(void) { }
text        kit_version(void) { return KIT_HEADER; }
text        kit_backends(void) { return va("SDL3-%d.%d.%d NET-%d.%d.%d TTF-%d.%d.%d AL-%s",SDL_VERSIONNUM_MAJOR(SDL_GetVersion()),SDL_VERSIONNUM_MINOR(SDL_GetVersion()),SDL_VERSIONNUM_MICRO(SDL_GetVersion()), NET_Version()/1000000, (NET_Version()/1000)%1000, NET_Version()%1000, TTF_Version()/1000000, (TTF_Version()/1000)%1000, TTF_Version()%1000, alGetString(AL_VERSION)); }
struct      kit kit = { .version = kit_version, .backends = kit_backends, .loop = kit_loop };

            map_(char*,char*) lockit = {less,hash};
text        language_translate(text key) { char **found = map_find(lockit, (char*)stable(key)); return found ? *found : va("[[%s]]", key); }
bool        language_import(const char* pathfile_mo) { // .mo reader
                for each_map(lockit, i) if(lockit.values[i]) SDL_free(lockit.values[i]);
                map_clear(lockit);
                union mo_header { char data[]; struct { unsigned magic, revision, num_strings, keys_offset, values_offset; }; };
                union mo_header *hdr = (union mo_header*)file.read(pathfile_mo, NULL);
                if( hdr && hdr->magic == 0x950412de && hdr->revision == 0 ) { // @todo: revision > 0 support; handle big-endian too?
                    unsigned* keys = (unsigned*)(hdr->data + hdr->keys_offset); // list of [size+offset] tuples
                    unsigned* values = (unsigned*)(hdr->data + hdr->values_offset); // list of [size+offset] tuples
                    for( unsigned i = 0; i < hdr->num_strings; ++i ) {
                        char* key = hdr->data + keys[i*2+1], *key_end = key + keys[i*2+0];
                        char* value = hdr->data + values[i*2+1], *value_end = value + values[i*2+0];
                        map_add(lockit, (char*)stable(key), SDL_strdup(value)); // @todo: non null-terminated strings (rare?)
                    }
                }
                // for each_map(lockit, i) printf("[%s]=[%s]\n", lockit.keys[i], lockit.values[i]);
                if(hdr) SDL_free(hdr);
                return map_count(lockit) > 0;
            }
struct      language language = { .import = language_import, .translate = language_translate };

struct      listener listener = { .volume = al_listener_volume, .position = al_listener_position, .velocity = al_listener_velocity, .orientation = al_listener_orientation, .doppler = al_listener_doppler, .model = al_listener_model, };

            // @todo: handle SDL_EVENT_LOCALE_CHANGED case
char*       locale_datetime(void) {
                static SDL_DateFormat dateFormat; // SDL_DATE_FORMAT_[YYYYMMDD|DDMMYYYY|MMDDYYYY]
                static SDL_TimeFormat timeFormat; // SDL_TIME_FORMAT_[12|24]HR
                ONCE { bool ok = SDL_GetDateTimeLocalePreferences(&dateFormat, &timeFormat); SDL_CHECK(ok); }
                SDL_Time ticks; bool ok = SDL_GetCurrentTime(&ticks); SDL_CHECK(ok);
                time_t mtime = ticks / 1e9;
                struct tm *ti = localtime(&mtime);
                int YYYY = ti->tm_year+1900, MM = ti->tm_mon+1, DD = ti->tm_mday, hh = ti->tm_hour, mm = ti->tm_min, ss = ti->tm_sec;
                char *hour = timeFormat == SDL_TIME_FORMAT_24HR ? va("%02d:%02d:%02d", hh, mm, ss) : va("%02d:%02d:%02d %s", hh % 12, mm, ss, hh >= 12 ? "pm":"am");
                /**/ if( dateFormat == SDL_DATE_FORMAT_DDMMYYYY ) return va("%02d/%02d/%02d %s",DD,MM,YYYY,hour);
                else if( dateFormat == SDL_DATE_FORMAT_MMDDYYYY ) return va("%02d/%02d/%02d %s",MM,DD,YYYY,hour);
                else /*( dateFormat == SDL_DATE_FORMAT_YYYYMMDD*/ return va("%02d/%02d/%02d %s",YYYY,MM,DD,hour);
            }
text        locale_time(void) { return locale_datetime() + 11; }
text        locale_date(void) { char *buf = locale_datetime(); return buf[10] = '\0', buf; }
struct      locale locale = { .date = locale_date, .time = locale_time };


            //#define LUA_IMPL
            //#pragma push_macro("time")
            //#undef time
            #include "3rd/3rd_minilua.h"
            //#pragma pop_macro("time")
            struct lua_t { lua_State *L; char *file; };
            array_(struct lua_t) Ls; AUTORUN { array_push(Ls, (struct lua_t){0}); }
bool        lua_read(unsigned id, const char *file_lua) {
                lua_State *L = Ls[id].L;
                if( !L ) return false;
                if( luaL_dofile(L, file_lua) != LUA_OK ) {
                    SDL_Log("\2lua_read(): error loading %s: %s", file_lua, lua_tostring(L, -1));
                    lua_pop(L, 1);
                    return false;
                }
                return true;
            }
bool        lua_reload(unsigned id) { return lua_read(id, Ls[id].file); }
unsigned    lua_open(const char *file_lua) {
                lua_State *L = luaL_newstate();
                struct lua_t lt = { L, file_lua ? string.dup(file_lua) : NULL };
                array_push(Ls, lt);
                unsigned id = array_count(Ls) - 1;
                if( L ) { luaL_openlibs(L); if( file_lua ) lua_read(id, file_lua); }
                return id;
            }
void        lua_close_(unsigned *id) {
                if( Ls[*id].L ) lua_close(Ls[*id].L), Ls[*id].L = NULL;
                if( Ls[*id].file ) string.close(&Ls[*id].file);
                *id = 0;
            }
bool        lua_call_(unsigned id, const char *func) {
                lua_State *L = Ls[id].L;
                lua_getglobal(L, func);
                if (lua_isfunction(L, -1)) {
                    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
                        SDL_Log("\2lua.call() `%s` Lua Runtime Error: %s", func, lua_tostring(L, -1));
                        lua_pop(L, 1); // Pop the error message
                        return false;
                    }
                    return true;
                } else {
                    SDL_Log("\2lua.call() `%s` not a function", func);
                    lua_pop(L, 1); // Pop the non-function value (e.g., nil)
                    return false;
                }
            }
struct      lua lua = { .open = lua_open, .read = lua_read, .reload = lua_reload, .call = lua_call_, .close = lua_close_, };

            array_(unsigned) palette_;
void        palette_set(unsigned index, unsigned color) { if( index >= array_count(palette_) ) array_resize(palette_, index); palette_[index] = color; }
unsigned    palette_get(unsigned index) { return palette_[index]; }
struct      palette palette = { .set = palette_set, .get = palette_get };

bool        os_async( const char *cmd ) {
                if( !cmd[0] ) return false;
                /* cmd = file_normalize(cmd); */
                ifdef(KIT_WINDOWS, { return WinExec(va("cmd /c \"%s\"", cmd), SW_HIDE) > 31; }, { return system(va("%s &", cmd)) == 0 } );
            }
void        os_beep(void) {
                ifdef(KIT_WINDOWS, if( os_async("rundll32 user32.dll,MessageBeep") ) return; );
                ifdef(KIT_LINUX,   if( os_async("paplay /usr/share/sounds/freedesktop/stereo/message.oga") ) return; );
                ifdef(KIT_MACOS,   if( os_async("tput bel") ) return; );
                fputc('\x7', stdout);
            }

#include    ifdef(KIT_WINDOWS, <windows.h>, <sys/statvfs.h>)
int         os_storage(void) { // MiB
                uint64_t bytes = 0;
                char *path = SDL_GetPrefPath("org", "app");
                if( path ) {
                    #if _WIN32
                        ULARGE_INTEGER avail_bytes;
                        if (GetDiskFreeSpaceExA(path, &avail_bytes, NULL, NULL)) bytes = (uint64_t)avail_bytes.QuadPart;
                    #else
                        struct statvfs s;
                        if (statvfs(path, &s) == 0) bytes = (uint64_t)s.f_bavail * s.f_frsize;
                    #endif
                    SDL_free(path);
                }
                return bytes / 1024 / 1024;
            }
            FILE *stdlog;
            #if 0 
            AUTORUN {
                // to console
                // stdlog = stdout;

                // to logfile
                // stdlog = fopen(arg("--logfile=.log"),"a+");

                // redirect to file
                // freopen(".log", "a+t", stdout);

                #if 0 // << enable if you're worried about logging perf
                // Flush automatically every 16 KiB from now
                setvbuf(stdlog ? stdlog : stdout, NULL, _IOFBF, 16 * 1024);
                #endif
            }
            #endif
static void os_logger(void* userdata, int category, SDL_LogPriority priority, const char* msg) {
                double clk = elapsed.ss();
                int ansi[] = { 0,34,31,35,32,36,33,0 }; // default,blue,red,pink,green,cyan,yellow,default colors
                if(!stdlog) stdlog = stdout;
                flockfile_(stdlog);

                    int def = 7; //category % COUNTOF(ansi); // default color based on category
                    int span = strspn(msg, "\1\2\3\4\5\6\7"); // unless overriden
                    if( span ) def = msg[span-1], msg += span; // get last color

                    fprintf(stdlog,"\033[%dm", ansi[def]);

                    if(msg[0] == '!') tracef(stdlog, +16), fprintf(stdlog,"\033[6m"), ++msg; // trace and blink after

                    // thread id, timestamp, text incl color escapes
                    fprintf(stdlog,"%04x %06.2f ", ((unsigned)thread.self()) & 0xFFFF, clk);
                    while( *msg ) *msg > 0 && *msg <= 7 ? fprintf(stdlog,"\033[%dm", ansi[*msg++]) : fputc(*msg++, stdlog);
                    fprintf(stdlog, "\033[%dm\n", ansi[7]);

                    // OutputDebugMessageA();

                funlockfile_(stdlog);
            }
void        os_log(text fmt, ...) {
                ONCE SDL_SetLogOutputFunction(os_logger, NULL);
                SDL_LogPriority priority[] = {
                [0] = SDL_LOG_PRIORITY_INVALID,
                [1] = SDL_LOG_PRIORITY_VERBOSE,
                [2] = SDL_LOG_PRIORITY_CRITICAL,
                [3] = SDL_LOG_PRIORITY_ERROR, 
                [4] = SDL_LOG_PRIORITY_TRACE,
                [5] = SDL_LOG_PRIORITY_DEBUG,
                [6] = SDL_LOG_PRIORITY_WARN,
                [7] = SDL_LOG_PRIORITY_INFO,
                }, col = fmt[fmt[0] == '!'], pr = priority[7]; // [col <= 7 ? col : 7];
                va_list li; va_start(li, fmt); SDL_LogMessageV(0/*cat*/,pr/*prio*/,fmt,li); va_end(li); puts(SDL_GetError()); 
            }
int         os_memory(void) { return SDL_GetSystemRAM(); } // MiB
text        os_name(void) { return SDL_GetPlatform(); }
void        os_browse(text url) { bool ok = SDL_OpenURL(url); SDL_CHECK(ok); }
void        os_die(text msg) { fprintf(stderr, "%s\n", msg); dialog.fatal(msg); exit(-1); }
text        os_arg(text key_value) { return arg(key_value); }
unsigned    os_argc(void) { return argc(); }
text        os_argv(unsigned num) { return argv(num); }
struct      os os = { .memory = os_memory, .name = os_name, .browse = os_browse, .die = os_die, .log = os_log, .storage = os_storage, .beep = os_beep, .argc = os_argc, .argv = os_argv, .arg = os_arg, };

void        random_seed(int n) { SDL_srand(n ? n : SDL_GetPerformanceCounter()); SDL_rand(1); }
bool        random_boolean(void) { return SDL_rand_bits() & 1; }
int         random_integer(int max) { return SDL_rand(max + 1); }
double      random_floating(double max) { return SDL_clamp(SDL_randf() * (max + 0.1), 0, max); }
double      random_range(double min, double max) { return max < min ? random_range(max, min) : min + random_floating(max - min); }
struct      random random = { .seed = random_seed, .boolean = random_boolean, .integer = random_integer, .floating = random_floating, .range = random_range };
AUTORUN{    random.seed(0); }
AUTOTEST{   test( random.boolean() | random.boolean() | random.boolean() | random.boolean() ); while(random.floating(5) != 5); } // should not wait forever

            // SDL_LOGICAL_PRESENTATION_STRETCH,   /**< The rendered content is stretched to the output resolution */
            // SDL_LOGICAL_PRESENTATION_LETTERBOX, /**< The rendered content is fit to the largest dimension and the other dimension is letterboxed with the clear color */
            // SDL_LOGICAL_PRESENTATION_OVERSCAN,  /**< The rendered content is fit to the smallest dimension and the other dimension extends beyond the output bounds */
            // SDL_LOGICAL_PRESENTATION_INTEGER_SCALE   /**< The rendered content is scaled up by integer multiples to fit the output resolution */
bool        render_open(int logical_w, int logical_h, float physical_area, unsigned flags) {
                ONCE {
                    if (!SDL_WasInit(SDL_INIT_VIDEO))
                    if (!SDL_Init(SDL_INIT_VIDEO)) {
                        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
                        return false;
                    }
                }
                float area = physical_area;
                if( area <= 0 ) area = 0.85;
                if( area  > 5 ) area /= 100; // [0, 0.5, 1.0, 1.1, 1.25, 2.00 ...]
                int phyw = display.width(0) * area;
                int phyh = display.height(0) * area;
                if (!SDL_CreateWindowAndRenderer("", phyw, phyh, SDL_WINDOW_RESIZABLE|SDL_WINDOW_TRANSPARENT|flags, &window.handle, &render.handle)) {
                    SDL_Log("Couldn't create window+renderer: %s", SDL_GetError());
                    return false;
                }
                SDL_SetRenderVSync(render.handle, kit_frequency = 1); // -1=adaptive, 0=unlimited, 1=vsync, 2=half vsync, etc.
                SDL_SetRenderLogicalPresentation(render.handle, logical_w ? logical_w : phyw, logical_h ? logical_h : phyh, SDL_LOGICAL_PRESENTATION_LETTERBOX);
                int vsync; SDL_CHECK(SDL_GetRenderVSync(render.handle, &vsync));
                os.log(" \b\6Kit%.*s\7 (renderer=%s, vsync=%d, backends=%s)", 1, kit.version(), SDL_GetRendererName(render.handle), vsync, kit.backends());
                return true;
            }
int         render_vsync(int mode) { if( mode >= -1 ) { kit_frequency = mode; SDL_SetRenderVSync(render.handle, kit_frequency < 0 ? SDL_RENDERER_VSYNC_ADAPTIVE : kit_frequency == 1 ? 1 : SDL_RENDERER_VSYNC_DISABLED); } SDL_CHECK(SDL_GetRenderVSync(render.handle, &mode)); return mode; }
int         render_fps(void) { return kit_fps; }  // see: kit_loop()
float       render_delta(void) { return kit_dt > 1 ? 1.f : kit_dt; } // see: kit_loop()
void        render_clear(unsigned rgba) { SDL_SetRenderScale(render.handle, 1,1); SDL_SetRenderDrawColor(render.handle, color.r(rgba), color.g(rgba), color.b(rgba), color.a(rgba)); SDL_RenderClear(render.handle); }
void        render_present(void) { SDL_SetRenderScale(render.handle, 1,1);SDL_RenderPresent(render.handle); }
float2      render_size(void) { int w, h; 
                float scale = SDL_GetWindowDisplayScale(window.handle); if(!SDL_GetWindowSize(window.handle, &w, &h)) w = h = 0;
                //  if (!SDL_GetRenderOutputSize(render.handle, &w, &h)) w = h = 0; 
                return float2(w,h); } // { // SDL_GetWindowSize(hwnd, &w, &h)
bool        render_save(const char *path_png) {
                #if __has_include("3rd/3rd_stb_image_write.h")
                if (render.handle)
                {
                    int w = 0, h = 0;
                    if (!SDL_GetRenderOutputSize(render.handle, &w, &h)) return false;

                    /* Allocate a buffer: 4 bytes per pixel (RGBA) */
                    unsigned char *pixels = (unsigned char *)SDL_malloc((size_t)w * h * 4);
                    if (!pixels) return false;

                    /* Read pixels from the renderer in RGBA8888 format */
                    SDL_Surface *surface = SDL_RenderReadPixels(render.handle, NULL);
                    if (!surface) { SDL_free(pixels); return false; }

                    /* Convert to RGBA if necessary */
                    SDL_Surface *rgba = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
                    SDL_DestroySurface(surface);
                    if (!rgba) { SDL_free(pixels); return false; }

                    stbi_write_png(path_png,
                                   rgba->w, rgba->h,
                                   4,
                                   rgba->pixels,
                                   rgba->pitch);

                    SDL_DestroySurface(rgba);
                    SDL_free(pixels);

                    return true;
                }
                #endif
                return false;
            }
struct      render render = { .save = render_save, .open = render_open, .clear = render_clear, .present = render_present, .vsync = render_vsync, .size = render_size, .fps = render_fps, .delta = render_delta };

void        sleep_ns(double ns) { if(ns > 0) SDL_DelayNS(ns); else thread.yield(); }
void        sleep_us(double us) { sleep_ns(us * 1000); }
void        sleep_ms(double ms) { sleep_us(ms * 1000); }
void        sleep_ss(double ss) { sleep_ms(ss * 1000); }
struct      sleep sleep = { .ns = sleep_ns, .us = sleep_us, .ms = sleep_ms, .ss = sleep_ss };

struct      speaker speaker = { .open = al_speaker_new, .range = al_speaker_range, .relative = al_speaker_relative, .position = al_speaker_position, .velocity = al_speaker_velocity, .direction = al_speaker_direction, .loop = al_speaker_loop, .volume = al_speaker_volume, .pitch = al_speaker_pitch, .play = al_speaker_play, .pause = al_speaker_pause, .resume = al_speaker_resume, .stop = al_speaker_stop, .playing = al_speaker_playing, .stopped = al_speaker_stopped, .paused = al_speaker_paused, .close = al_speaker_free, };

data        string_va(text fmt, ...) {
                va_list vl, copy;
                va_start(vl, fmt);
                    va_copy(copy, vl);
                    int sz = SDL_vsnprintf( 0, 0, fmt, copy ) + 1;
                    va_end(copy);
                enum { STACK_ALLOC = 64*1024 }; SDL_assert("<- no stack enough, increase STACK_ALLOC value" && sz < STACK_ALLOC);
                static THREAD char buf[STACK_ALLOC] = {0};
                static THREAD int cur = 0, len = STACK_ALLOC - 1;
                char* ptr = buf + (cur *= (cur+sz) < len, (cur += sz) - sz);
                SDL_vsnprintf( ptr, sz, fmt, vl );
                va_end(vl);
                return ptr;
            }
data        string_open(text fmt, ...) { va_list li; va_start(li,fmt); char *out = 0; int x = SDL_vasprintf(&out, fmt, li); va_end(li); return out; }
data        string_dup(text src) { return string_open("%s", src); }
void        string_close(text *str) { if(*str) SDL_free((char*)*str), *str = 0; }
text        string_valid(text s) { return strvalid(s); }
text        string_stable(text s) { return stable(s); }
uint64_t    string_hash(text s) { return hash(s); }
int         string_count(text s, int ch) { return strcnt(s, ch); }
int         string_match(text s, text sub) { return strmatch(s, sub); }
int         string_matchi(text s, text sub) { return strmatchi(s, sub); }
struct      string string = { .dup = string_dup, .open = string_open, .close = string_close, .va = string_va, .valid = string_valid, .match = string_match, .matchi = string_matchi, .stable = string_stable, .hash = string_hash, .count = string_count };

            array_(SDL_Texture*) textures;
            AUTORUN { static SDL_Texture empty = {0}; array_push(textures, &empty ); }
unsigned    texture_copy(const void *pixels, int w, int h, int n) {
                SDL_assert(n >= 3);
                SDL_PixelFormat fmt = n == 3 ? SDL_PIXELFORMAT_RGB24 : SDL_PIXELFORMAT_RGBA32;
                SDL_Texture *texture = 0;
                if( pixels ) {
                    SDL_Surface *surface = pixels ? SDL_CreateSurfaceFrom(w, h, fmt, (void*)pixels, w*n) : SDL_CreateSurface(w, h, fmt);
                    if (!surface) SDL_Log("Couldn't copy image: %s", SDL_GetError());
                    if (!surface) return 0;
                    texture = SDL_CreateTextureFromSurface(render.handle, surface);
                    if (!texture) SDL_Log("Couldn't create static texture: %s", SDL_GetError());
                    SDL_DestroySurface(surface);
                } else {
                    texture = SDL_CreateTexture(render.handle, fmt, SDL_TEXTUREACCESS_STREAMING, w, h);
                    if (!texture) SDL_Log("Couldn't create dynamic texture: %s", SDL_GetError());
                }
                if (!texture) return 0;
                array_push(textures, texture);
                return array_count(textures) - 1;
            }
unsigned    texture_open(const char *pathfile) {
                SDL_Texture *texture = 0;
                #if __has_include("3rd/3rd_stb_image.h")
                int w,h,n = 0; void *pixels = image.open(pathfile, &w, &h, &n);
                if( pixels ) {
                    unsigned tx = texture_copy(pixels, w, h, n);
                    image.close(&pixels);
                    return tx;
                }
                #else
                SDL_Surface *surface = 0;
                if( !surface ) surface = SDL_LoadPNG(pathfile);
                if( !surface ) surface = SDL_LoadBMP(pathfile);
                if(  surface ) texture = SDL_CreateTextureFromSurface(render.handle, surface);
                if(  texture ) SDL_DestroySurface(surface);
                #endif
                if( texture ) {
                    array_push(textures, texture);
                    return array_count(textures) - 1;
                }
                SDL_Log("Couldn't create static texture `%s`: %s", pathfile, SDL_GetError());
                return 0;
            }
unsigned    texture_width (unsigned texture) { return textures[texture]->w; }
unsigned    texture_height(unsigned texture) { return textures[texture]->h; }
unsigned    texture_pitch(unsigned texture) { return textures[texture]->w * SDL_BYTESPERPIXEL(textures[texture]->format); }
void*       texture_handle(unsigned texture) { return texture ? textures[texture] : NULL; }
void        texture_close(unsigned *texture) { if( *texture ) { SDL_DestroyTexture(texture_handle(*texture)); *texture = 0; } }
struct      texture texture = { .open = texture_open, .close = texture_close, .width = texture_width, .height = texture_height, .handle = texture_handle, .copy = texture_copy };

int         thread_cores(void) { return SDL_GetNumLogicalCPUCores(); }
void        thread_yield(void) { SDL_CPUPauseInstruction(); }
bool        thread_primary(void) { return SDL_IsMainThread(); }
//  void    thread_defer( void (*fn)(void*), void *userdata, bool on_main_thread ) { bool ok = SDL_RunOnMainThread(fn, userdata, 0/*wait_complete*/); SDL_CHECK(ok); } 
uint64_t    thread_self(void) { return SDL_GetCurrentThreadID(); } // 0 if threads are not supported
uint64_t    thread_create(text name, int (*fn)(void*), void *userdata) { SDL_Thread *th = SDL_CreateThread(fn, name, userdata); SDL_CHECK(th); return (uintptr_t)th; }
int         thread_join(uint64_t tid) { int rc = 0; SDL_WaitThread((SDL_Thread*)(uintptr_t)tid, &rc); return rc; }
text        thread_name(uint64_t tid) { return SDL_GetThreadName((SDL_Thread*)(uintptr_t)tid); }
struct      thread thread = { .cores = thread_cores, .yield = thread_yield, .primary = thread_primary, .self = thread_self, .create = thread_create, .join = thread_join, .name = thread_name };

uint64_t    timer_callback1(void *userdata, SDL_TimerID timerID, uint64_t interval) { return ((void (*)(void))userdata)(), 0; }
uint64_t    timer_callbackN(void *userdata, SDL_TimerID timerID, uint64_t interval) { return ((void (*)(void))userdata)(), interval; }
void        timer_once(double interval_ss, void (*fn)(void)) { uint64_t ns = interval_ss * 1e9; SDL_TimerID id = SDL_AddTimerNS(ns, timer_callback1, fn); SDL_CHECK(id); }
void        timer_every(double interval_ss, void (*fn)(void)) { uint64_t ns = interval_ss * 1e9; SDL_TimerID id = SDL_AddTimerNS(ns, timer_callbackN, fn); SDL_CHECK(id); }
struct      timer timer = { .once = timer_once, .every = timer_every };

            #define KIT_CODE  1
            #include "kits/kit.ui/kit.h"
//int        (*ui_tree)(const char *label) = ui2_tree;
//int         (*ui_label)(const char *txt) = ui2_label;
//int         (*ui_section)(const char *txt, int open) = ui2_section;
//int         (*ui_int)(const char *txt, int *f, int lo, int hi) = ui2_int;
//int         (*ui_float)(const char *txt, float *f, float lo, float hi) = ui2_float;
//int         (*ui_rgba8)(const char *txt, void *f) = ui2_rgba8;
//int        (*ui_tree_end)(void) = ui2_tree_end;
//int       (*ui_popup)(const char *label) = ui2_popup;
//int       (*ui_popup_end)(void) = ui2_popup_end;
//int       (*ui_hovered)(void) = ui2_hovered;
struct      ui ui = { 
                .window = ui2_window, 
                 .button = ui2_button, 
                 .buttons = ui2_buttons,
                 .boolean = ui2_bool,
                .window_end = ui2_window_end,
            };

text        user_name(void) { return strvalid(SDL_getenv(ifdef(KIT_WINDOWS, "USERNAME", "USER"))); }
text        user_language(void) { SDL_Locale **list = SDL_GetPreferredLocales(NULL); SDL_CHECK(list); char *preferred = va("%s", list[0]->language); return SDL_free(list), preferred; }
text        user_country(void) { SDL_Locale **list = SDL_GetPreferredLocales(NULL); SDL_CHECK(list); char *preferred = va("%s", strvalid(list[0]->country)); return SDL_free(list), preferred; }
struct      user user = { .language = user_language, .country = user_country };

            // SDL_GetCameraDriver
            // SDL_GetCameraFormat
            // SDL_GetCameraID
            // SDL_GetCameraName
            // SDL_GetCameraPermissionState
            // SDL_GetCameraPosition
            // SDL_GetCameraProperties
            // SDL_GetCurrentCameraDriver
            // SDL_GetNumCameraDrivers
            // @fixme: color on Y/UV planar modes
            // @todo: resize texture if webcam resolution changes
            static array_(SDL_Camera*)  camdev;
            static array_(unsigned)     camtex;
            AUTORUN { array_push(camdev, NULL); }
            AUTORUN { array_push(camtex, 0); }
unsigned    webcam_count(void) {
                int count = 0;
                SDL_CameraID *devices = SDL_GetCameras(&count);
                if( devices ) SDL_free(devices);
                return count;
            }
void        webcam_close(unsigned *id_) {
                unsigned id = *id_;
                if( id < array_count(camdev) ) if( camdev[id] ) SDL_CloseCamera(camdev[id]), camdev[id] = 0;
                if( id < array_count(camtex) ) if( camtex[id] ) texture.close(&camtex[id]);
                *id_ = 0;
            }
unsigned    webcam_capture(unsigned id) {
                ONCE if(!SDL_WasInit(SDL_INIT_CAMERA)) if(!SDL_Init(SDL_INIT_CAMERA)) os.die("Cannot init webcam");

                if( id >= array_count(camdev) ) array_resize(camdev,id+1);
                if( id >= array_count(camtex) ) array_resize(camtex,id+1);

                if( !camdev[id] ) {
                    int devcount = 0;
                    SDL_CameraID *devices = SDL_GetCameras(&devcount);
                    SDL_CHECK(devices);
                    if( devices && devcount ) {
                        int numspecs = 0;
                        SDL_CameraSpec **specs = SDL_GetCameraSupportedFormats(devices[0], &numspecs), *chosen = 0;
                        if( specs && numspecs ) { // iterate the camera modes
                            for( int i = 0; i < numspecs; ++i ) {
                                // @todo: pick up spec mode that matches best our app resolution (~similar densities). good idea?
                                if( specs[i]->format == SDL_PIXELFORMAT_YUY2 ) continue;                     // not ok, probably interleaved
                                if( specs[i]->format == SDL_PIXELFORMAT_NV12 ) { chosen = specs[i]; break; } // ok Y + U/V (2 planes)
                            }
                            if( chosen ) {
                                camdev[id] = SDL_OpenCamera(devices[0], chosen);
                            }
                        }
                        SDL_free(devices);
                    }
                }

                unsigned *texid = &camtex[id];

                uint64_t timestamp_ns = 0;
                SDL_Surface *frame = SDL_AcquireCameraFrame(camdev[id], &timestamp_ns);
                for( ; frame; SDL_ReleaseCameraFrame(camdev[id], frame), frame = 0) {

                    if( *texid) {
                        SDL_Texture *tex = texture.handle(*texid);

                        void* pixels; int pitch;
                        if (SDL_LockTexture(tex, NULL, &pixels, &pitch)) {

                            // expand Y plane as RGB plane. note: input is x2 planes [Y + U/V], output is RGB
                            for (int y = 0, w=frame->w; y < frame->h; y++) {
                                uint8_t* row = ((uint8_t*)pixels + y * w*3);

                                for (int x = 0; x < w; x++) {
                                    uint8_t v = ((uint8_t*)frame->pixels)[y * w + (w-1-x)];

                                    *row++ = v;
                                    *row++ = v;
                                    *row++ = v;
                                }
                            }

                            SDL_UnlockTexture(tex);
                        }
                    }
                    else {
                        *texid = texture.copy(NULL, frame->w, frame->h, 3); //frame->pitch);
                    }
                    // int bpp = frame->pitch / frame->w;
                    // texture_update(id,frame->w,frame->h,bpp,frame->pixels,TEXTURE_R);
                }

                return *texid;
            }
struct      webcam webcam = { .count = webcam_count, .capture = webcam_capture, .close = webcam_close };


//@todo:    window.exclusive()                SDL_SetWindowFullscreenMode(win, SDL_DisplayMode *)
//@todo:    window.child()                    SDL_SetWindowParent(childwin, parentwin) + SDL_SetWindowModal(childwin)
//@todo:    window.sizes(minsize,maxsize)     SDL_SetWindowMinimumSize(win,minw,minh) + SDL_SetWindowMaximumSize(win,maxw,maxh)
//@todo:    window.menu()                     SDL_ShowWindowSystemMenu(win, x, y); useful? user actually handles this
bool        window_fullscreen(int on) { if( on >= 0 && on <= 1 ) SDL_SetWindowFullscreen(window.handle, on), SDL_SyncWindow(window.handle); return !!(SDL_GetWindowFlags(window.handle) & SDL_WINDOW_FULLSCREEN); }
bool        window_show(int on) { if( on >= 0 && on <= 1 ) (on ? SDL_ShowWindow : SDL_HideWindow)(window.handle), SDL_SyncWindow(window.handle); return !(SDL_GetWindowFlags(window.handle) & SDL_WINDOW_HIDDEN); }
bool        window_ontop(int on) { if( on >= 0 && on <= 1 ) SDL_SetWindowAlwaysOnTop(window.handle, on), SDL_SyncWindow(window.handle); return !!(SDL_GetWindowFlags(window.handle) & SDL_WINDOW_ALWAYS_ON_TOP); }
bool        window_decorated(int on) { if( on >= 0 && on <= 1 ) SDL_SetWindowBordered(window.handle, on), SDL_SyncWindow(window.handle); return !(SDL_GetWindowFlags(window.handle) & SDL_WINDOW_BORDERLESS); }
bool        window_resizable(int on) { if( on >= 0 && on <= 1 ) SDL_SetWindowResizable(window.handle, on), SDL_SyncWindow(window.handle); return !!(SDL_GetWindowFlags(window.handle) & SDL_WINDOW_RESIZABLE); }
bool        window_confined(int on) { if( on >= 0 && on <= 1 ) SDL_SetWindowMouseGrab(window.handle, on), SDL_SyncWindow(window.handle); return !!(SDL_GetWindowFlags(window.handle) & SDL_WINDOW_MOUSE_GRABBED); }
float       window_alpha(float on) { if( on >= 0 && on <= 1 ) SDL_SetWindowOpacity(window.handle, on), SDL_SyncWindow(window.handle); return SDL_GetWindowOpacity(window.handle); }
text        window_title(text string) { if( string ) SDL_SetWindowTitle(window.handle, string), SDL_SyncWindow(window.handle); return SDL_GetWindowTitle(window.handle); }
text        window_icon(text pngfile) { SDL_Surface *icon = SDL_LoadPNG(pngfile); if( icon ) SDL_SetWindowIcon(window.handle, icon), SDL_SyncWindow(window.handle), SDL_free(icon); return pngfile; }
bool        window_flash(int on) { static int last = 0; if( on >= 0 && on <= 1 ) { on = last = SDL_GetWindowFlags(window.handle) & SDL_WINDOW_MINIMIZED ? SDL_FLASH_UNTIL_FOCUSED : SDL_FLASH_BRIEFLY; SDL_FlashWindow(window.handle, on ? on : SDL_FLASH_CANCEL); SDL_SyncWindow(window.handle); } return last; }
float       window_progress(float on) { if( on >= 0 && on <= 1 ) SDL_SetWindowProgressState(window.handle, on <= 0 ? SDL_PROGRESS_STATE_INDETERMINATE : on < 1 ? SDL_PROGRESS_STATE_NORMAL : SDL_PROGRESS_STATE_NONE), SDL_SetWindowProgressValue(window.handle, on), SDL_SyncWindow(window.handle); return SDL_GetWindowProgressValue(window.handle); }
float       window_aspect(float minratio, float maxratio) { if( minratio >= 0 && maxratio >= 0 ) SDL_SetWindowAspectRatio(window.handle,minratio,maxratio), SDL_SyncWindow(window.handle); int x, y; SDL_GetWindowSize(window.handle, &x, &y); return x / (float)(y + !y); }
float2      window_position(int x, int y) { if( x >= 0 && y >= 0 ) SDL_SetWindowPosition(window.handle, x, y), SDL_SyncWindow(window.handle); return SDL_GetWindowPosition(window.handle, &x, &y), float2(x, y); }
float2      window_size(int w, int h) { if( w > 0 && h > 0 ) SDL_SetWindowSize(window.handle, w, h), SDL_SyncWindow(window.handle); return SDL_GetWindowSize(window.handle, &w, &h), float2(w, h); }
float       window_maximize(float percent01) {
                if( percent01 >= 0 && percent01 <= 1 )
                    (percent01 >= 1 ? SDL_MaximizeWindow : percent01 > 0 ? SDL_RestoreWindow : SDL_MinimizeWindow)(window.handle), 
                    SDL_RaiseWindow(window.handle); 
                unsigned flags = (SDL_SyncWindow(window.handle), SDL_GetWindowFlags(window.handle));
                return flags & SDL_WINDOW_MINIMIZED ? 0.0f : flags & SDL_WINDOW_MAXIMIZED ? 1.0f : 0.5f;
            }
struct      window window = { .fullscreen = window_fullscreen, .show = window_show, .ontop = window_ontop, .decorated = window_decorated, .resizable = window_resizable, .confined = window_confined, .alpha = window_alpha, .title = window_title, .icon = window_icon, .flash = window_flash, .progress = window_progress, .aspect = window_aspect, .position = window_position, .size = window_size, .maximize = window_maximize, };

#undef main
#define SDL_MAIN_USE_CALLBACKS 1 // use the callbacks instead of main(). wont freeze while dragging windows + will also work on emscripten
#include <SDL3/SDL_main.h>

AUTORUN {    
    // SDL hints + metas + init
    // read: https://github.com/libsdl-org/SDL/blob/main/include/SDL3/SDL_hints.h

    SDL_SetAppMetadata("Kit", KIT_HEADER, "com.kit.example");

    // @todo: SDL_PROP_GPU_DEVICE_CREATE_PREFERLOWPOWER_BOOLEAN
#if !NDEBUG
    // debug defaults first
    ifdef(KIT_LINUX, SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "x11")); // allow linux+renderdoc via xwayland
    SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1"); // allow CTRL^C to break
#endif

    // user defaults now
    // list of "name=value\n"[...] pairs
    extern const char *hints;
    if( hints && hints[0] )
    for each_string(s,hints,"\r\n") {
        char *name = s;
        char *value = SDL_strchr(s, '='); if(!value) continue; *value++ = 0;

        if( 0 == SDL_strncmp(name, "SDL_HINT_", 9) ) { // example "SDL_HINT_NO_SIGNAL_HANDLERS=1"
            SDL_memcpy(name += 5, "SDL_", 4);
            SDL_CHECK(SDL_SetHint(name, value));
        }
        else
        if( 0 == SDL_strncmp(name, "SDL.app.metadata.", 17) ) { // example "sdl.app.metadata.name=hello"
            SDL_CHECK(SDL_SetAppMetadataProperty(name, value));
        }
        else {
            dialog.alert(va("Unknown hint/prop provided: `%s`=`%s`", name, value));
        }
    }

    if( !SDL_WasInit(SDL_INIT_VIDEO) )
        if( !SDL_Init(SDL_INIT_VIDEO) )
            os.die( va("Couldn't initialize SDL: %s", SDL_GetError()) );
}
