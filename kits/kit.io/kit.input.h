// kit_input.h — single-header input library for kit/SDL3
// - rlyeh, public domain
//
// USAGE
//   In ONE .c file before including:
//     #define KIT_CODE
//     #include "kit_input.h"
//   In all other files just:
//     #include "kit_input.h"
//
// INTEGRATION (per-frame, inside your event loop)
//   ev.emit -> kit_input_pump_event(ev.emit)
//   ev.tick -> ... queries ... kit_input_next_frame()  // next_frame LAST
//
// EXAMPLE
//   if (keyboard.down("f1"))                       os.alert("F1!");
//   if (gamepad.held(0, "south"))                  player_jump();
//   float lx = gamepad.read(0, "lx");
//   if (mouse.down("left"))                        fire();
//   if (touch.tapped(0))                           tap_action();
//   if (gesture.swipe_left())                      go_back();
//   input.bind("jump", "keyboard(space) || gamepad(0,south)");
//   input.bind("back", "swipe_left() || keyboard(escape)");
//   if (input.action("jump"))                      player_jump();

#ifndef KIT_INPUT_H
#define KIT_INPUT_H

#include <stdbool.h>

// ---------------------------------------------------------------------------
// public structs

extern struct keyboard_api {
float       (*get)   (const char *vk);
float       (*up)    (const char *vk);
float       (*down)  (const char *vk);
float       (*idle)  (const char *vk);
float       (*held)  (const char *vk);
float       (*tapped)(const char *vk);
float       (*repeat)(const char *vk, unsigned delay_frames);
float       (*any)   (void);
} keyboard;

extern struct mouse_api {
// vk: "left"/"lmb","right"/"rmb","middle"/"mmb","1","2","x","y","wheel"
float       (*get)   (const char *vk);
float       (*delta) (const char *vk);
float       (*up)    (const char *vk);
float       (*down)  (const char *vk);
float       (*idle)  (const char *vk);
float       (*held)  (const char *vk);
float       (*tapped)(const char *vk);
float       (*repeat)(const char *vk, unsigned delay_frames);
float       (*any)   (void);
int         (*cursor)(int shape); // sets shape if valid arg is supplied: 0:hide,1:arrow,2:cross,3:hand,4:text,5:move,6:spin,7:wait,8:deny. returns current shape in any case
} mouse;

extern struct gamepad_api { // @todo: battery -> .live(gid) -> returns 0 if disconnected, >0 if connected (battery level (0..100])
// vk buttons: "A","B","X","Y", aka "South","East","West","North"
//             "L1","R1","L2","R2","L3","R3","Start","Back","Guide" @todo:"Share"
//             "<",">","^","v" (dpad)
// vk axes:    "LX","LY","RX","RY"
float       (*connected)(int gid);
float       (*get)      (int gid, const char *vk);
float       (*delta)    (int gid, const char *vk);
float       (*up)       (int gid, const char *vk);
float       (*down)     (int gid, const char *vk);
float       (*idle)     (int gid, const char *vk);
float       (*held)     (int gid, const char *vk);
float       (*tapped)   (int gid, const char *vk);
float       (*repeat)   (int gid, const char *vk, unsigned delay_frames);
float       (*any)      (int gid);
} gamepad;

extern struct rumble_api {
bool        (*device)  (float amount01, unsigned ms); // amount[0..1]; ms=0 to stop. rumbles primary haptic device found (mobile, wheel, haptic mouse). always device[0], player-agnostic. returns 0 if error or not supported
bool        (*motors)  (int gid, float lo01, float hi01, unsigned ms); // gamepad body motors [0..1]; ms=0 to stop. returns 0 if error
bool        (*triggers)(int gid, float left01, float right01, unsigned ms); // gamepad trigger motors [0..1] (Xbox/DualSense); ms=0 to stop. returns 0 if error
} rumble;

extern struct touch_api {
// id = finger birth-order [0..count()-1]
int         (*count)   (void);
float       (*x)       (int id);
float       (*y)       (int id);
float       (*pressure)(int id);
float       (*delta_x) (int id);
float       (*delta_y) (int id);
float       (*down)    (int id);
float       (*up)      (int id);
float       (*held)    (int id);
float       (*tapped)  (int id);
float       (*any)     (void);
// frame-delta gestures
float       (*swipe_x) (int id);  // per-finger x swipe delta this frame
float       (*swipe_y) (int id);  // per-finger y swipe delta this frame
float       (*pinch)   (void);    // scale delta this frame (0=no change)
float       (*rotate)  (void);    // rotation delta in degrees this frame (0=no change)
} touch;

extern struct gesture_api {
// dollar-gesture recognition (via SDL3_gesture)
bool        (*record)     (const char *name);  // begin recording template
bool        (*save)       (const char *file);  // serialize templates to file
bool        (*load)       (const char *file);  // load templates from file
float       (*recognized) (const char *name);  // match score 0..1, 0 if not fired this frame
// directional swipes — fire once per gesture, consume on read
float       (*swipe_up)   (void);
float       (*swipe_down) (void);
float       (*swipe_left) (void);
float       (*swipe_right)(void);
// tap counts
float       (*tap)        (int fingers);       // N-finger tap completed this frame
float       (*double_tap) (int fingers);       // N-finger double-tap within ~300ms
} gesture;

extern struct input_api {
// expr may reference: keyboard(vk), gamepad(gid,vk), mouse(vk),
//   touch_held(id), touch_down(id), touch_any(), touch_pinch(),
//   swipe_left(), swipe_right(), swipe_up(), swipe_down(),
//   tap(n), double_tap(n), recognized(name)
bool        (*bind)  (const char *name, const char *expr); // false if expr invalid
float       (*action)(const char *name);   // evaluate bound action; NaN if not found
bool        (*unbind)(const char *name);   // exact name, "*" all, "ns.*" prefix wildcard
float       (*eval)  (const char *expr);   // ad-hoc expression eval; NaN on error
} input;

// ---------------------------------------------------------------------------
// lifecycle

void input_pump_event(const SDL_Event *e);  // call for each ev.emit
void input_next_frame(void);                // call LAST in ev.tick

#endif // KIT_INPUT_H

// ===========================================================================
#if KIT_CODE
#pragma once

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <SDL3/SDL.h>
#define SDL_GESTURE_IMPLEMENTATION
#include <3rd_sdl_gesture.h>
#include <3rd_minilua.h>
//#include <lua.h>
//#include <lauxlib.h>
//#include <lualib.h>

// ---------------------------------------------------------------------------
// constants

#define INPUT_MAX_KEYS       512
#define INPUT_MAX_GP         128   // [0..31] buttons, [32+] axes
#define INPUT_MAX_MS          16   // [0..7] buttons, [8]x [9]y [10]wheel
#define INPUT_MAX_FINGERS     10
#define GAMEPAD_AXIS_OFS      32
#define INPUT_MAX_GAMEPADS     4
#define INPUT_MAX_BINDINGS   256
#define INPUT_MAX_DOLLAR      32
#define INPUT_SWIPE_VEL        0.15f // normalised units/frame to count as swipe
#define INPUT_DTAP_MS        300 // double tap time

// ---------------------------------------------------------------------------
// raw state types

typedef struct { float now, prev; int down_frame, up_frame; } inputstate_t;

typedef struct {
    float x, y, px, py, pressure;
    int   down_frame, up_frame;
    bool  active, was_active;
    SDL_FingerID sdl_id;
} finger_t;

typedef struct {
    float last_tap_ms;
    bool  swipe_up, swipe_down, swipe_left, swipe_right;
    bool  tap1, tap2, dtap1, dtap2;
} gesture_t;

// ---------------------------------------------------------------------------
// state storage

static inputstate_t keyboards[INPUT_MAX_KEYS];
static inputstate_t gamepads[INPUT_MAX_GAMEPADS][INPUT_MAX_GP];
static inputstate_t mice[INPUT_MAX_MS];
static finger_t     fingers[INPUT_MAX_FINGERS];
static int          fingers_count = 0;
static float        input_pinch_delta  = 0.f;
static float        input_rotate_delta = 0.f;
static gesture_t    input_gest        = {0};
static int          input_frame        = 0;
static double       input_time_ms      = 0.0;
static bool         gamepad_conn[INPUT_MAX_GAMEPADS];
static SDL_Gamepad *gamepad_handlers[INPUT_MAX_GAMEPADS];

static struct { char name[64]; Gesture_ID id; float score; } dollars[INPUT_MAX_DOLLAR];
static int  dollar_count          = 0;
static bool dollar_recording      = 0;
static char dollar_rec_name[64]   = {0};

typedef struct { char name[64]; } binding_t;
static binding_t  bindings[INPUT_MAX_BINDINGS];
static int        bindings_count = 0;

// ---------------------------------------------------------------------------
// index helpers

static int input_key_index(const char *n) {
    if (!n||!n[0]) return -1;
    SDL_Scancode sc = SDL_GetScancodeFromName(n);
    if (sc != SDL_SCANCODE_UNKNOWN) return (int)sc;
    if (!n[1]) {
        SDL_Keycode kc = SDL_GetKeyFromName(n);
        if (kc != SDLK_UNKNOWN) {
            sc = SDL_GetScancodeFromKey(kc, NULL);
            if (sc != SDL_SCANCODE_UNKNOWN) return (int)sc;
        }
    }
    return -1;
}

static int gamepad_index(const char *n) {
    if (!SDL_strcasecmp(n,"^"))     return SDL_GAMEPAD_BUTTON_DPAD_UP;
    if (!SDL_strcasecmp(n,"v"))     return SDL_GAMEPAD_BUTTON_DPAD_DOWN;
    if (!SDL_strcasecmp(n,"<"))     return SDL_GAMEPAD_BUTTON_DPAD_LEFT;
    if (!SDL_strcasecmp(n,">"))     return SDL_GAMEPAD_BUTTON_DPAD_RIGHT;
    if (!SDL_strcasecmp(n,"A")||!SDL_strcasecmp(n,"south")||!SDL_strcasecmp(n,"cross"))    return SDL_GAMEPAD_BUTTON_SOUTH;
    if (!SDL_strcasecmp(n,"B")||!SDL_strcasecmp(n,"east") ||!SDL_strcasecmp(n,"circle"))   return SDL_GAMEPAD_BUTTON_EAST;
    if (!SDL_strcasecmp(n,"X")||!SDL_strcasecmp(n,"west") ||!SDL_strcasecmp(n,"square"))   return SDL_GAMEPAD_BUTTON_WEST;
    if (!SDL_strcasecmp(n,"Y")||!SDL_strcasecmp(n,"north")||!SDL_strcasecmp(n,"triangle")) return SDL_GAMEPAD_BUTTON_NORTH;
    if (!SDL_strcasecmp(n,"lx"))    return GAMEPAD_AXIS_OFS + SDL_GAMEPAD_AXIS_LEFTX;
    if (!SDL_strcasecmp(n,"ly"))    return GAMEPAD_AXIS_OFS + SDL_GAMEPAD_AXIS_LEFTY;
    if (!SDL_strcasecmp(n,"rx"))    return GAMEPAD_AXIS_OFS + SDL_GAMEPAD_AXIS_RIGHTX;
    if (!SDL_strcasecmp(n,"ry"))    return GAMEPAD_AXIS_OFS + SDL_GAMEPAD_AXIS_RIGHTY;
    if (!SDL_strcasecmp(n,"L2"))    return GAMEPAD_AXIS_OFS + SDL_GAMEPAD_AXIS_LEFT_TRIGGER;
    if (!SDL_strcasecmp(n,"R2"))    return GAMEPAD_AXIS_OFS + SDL_GAMEPAD_AXIS_RIGHT_TRIGGER;
    if (!SDL_strcasecmp(n,"L1"))    return SDL_GAMEPAD_BUTTON_LEFT_SHOULDER;
    if (!SDL_strcasecmp(n,"R1"))    return SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER;
    if (!SDL_strcasecmp(n,"L3"))    return SDL_GAMEPAD_BUTTON_LEFT_STICK;
    if (!SDL_strcasecmp(n,"R3"))    return SDL_GAMEPAD_BUTTON_RIGHT_STICK;
    if (!SDL_strcasecmp(n,"start")) return SDL_GAMEPAD_BUTTON_START;
    if (!SDL_strcasecmp(n,"back"))  return SDL_GAMEPAD_BUTTON_BACK;
    if (!SDL_strcasecmp(n,"guide")) return SDL_GAMEPAD_BUTTON_GUIDE;
    return -1;
}

static int mouse_index(const char *n) {
    if (!SDL_strcasecmp(n,"x")) return 8;
    if (!SDL_strcasecmp(n,"y")) return 9;
    if (!SDL_strcasecmp(n,"l")||!SDL_strcasecmp(n,"lmb")||!SDL_strcasecmp(n,"left")  ) return 0;
    if (!SDL_strcasecmp(n,"m")||!SDL_strcasecmp(n,"mmb")||!SDL_strcasecmp(n,"middle")) return 1;
    if (!SDL_strcasecmp(n,"r")||!SDL_strcasecmp(n,"rmb")||!SDL_strcasecmp(n,"right") ) return 2;
    if (!SDL_strcasecmp(n,"w")||!SDL_strcasecmp(n,"wheel")) return 10;
    if (!SDL_strcasecmp(n,"1")||!SDL_strcasecmp(n,"x1"))return 3;
    if (!SDL_strcasecmp(n,"2")||!SDL_strcasecmp(n,"x2"))return 4;
    return -1;
}

// ---------------------------------------------------------------------------
// generic state accessors

static float input_state_read  (inputstate_t *s) { return s->now; }
static float input_state_delta (inputstate_t *s) { return s->now - s->prev; }
static float input_state_up    (inputstate_t *s) { return (s->now==0&&s->prev!=0)?1.f:0.f; }
static float input_state_down  (inputstate_t *s) { return (s->now!=0&&s->prev==0)?1.f:0.f; }
static float input_state_idle  (inputstate_t *s) { return (s->now==0&&s->prev==0)?1.f:0.f; }
//static float input_state_held  (inputstate_t *s) { return (s->now!=0&&s->prev!=0)?1.f:0.f; }
static float input_state_held  (inputstate_t *s) { return s->now!=0 ? 1.f : 0.f; }
static float input_state_tapped(inputstate_t *s) {
    return (s->now==0&&s->prev!=0&&s->down_frame>=0&&s->up_frame>=s->down_frame)?1.f:0.f;
}
static float input_state_repeat(inputstate_t *s, unsigned delay) {
    if (!s->now) return 0.f;
    int hf = input_frame - s->down_frame;
    return (hf>=(int)delay&&(hf%4)==0)?1.f:0.f;
}

// ---------------------------------------------------------------------------
// keyboard

static float keyboard_read  (const char *vk) { int i=input_key_index(vk); return i>=0?input_state_read  (&keyboards[i]):0; }
static float keyboard_up    (const char *vk) { int i=input_key_index(vk); return i>=0?input_state_up    (&keyboards[i]):0; }
static float keyboard_down  (const char *vk) { int i=input_key_index(vk); return i>=0?input_state_down  (&keyboards[i]):0; }
static float keyboard_idle  (const char *vk) { int i=input_key_index(vk); return i>=0?input_state_idle  (&keyboards[i]):1; }
static float keyboard_held  (const char *vk) { int i=input_key_index(vk); return i>=0?input_state_held  (&keyboards[i]):0; }
static float keyboard_tapped(const char *vk) { int i=input_key_index(vk); return i>=0?input_state_tapped(&keyboards[i]):0; }
static float keyboard_repeat(const char *vk, unsigned d) { int i=input_key_index(vk); return i>=0?input_state_repeat(&keyboards[i],d):0; }
static float keyboard_any(void) { for(int i=0;i<INPUT_MAX_KEYS;i++) if(keyboards[i].now) return 1.f; return 0.f; }

struct keyboard_api keyboard = { keyboard_read,keyboard_up,keyboard_down,keyboard_idle,keyboard_held,keyboard_tapped,keyboard_repeat,keyboard_any };

// ---------------------------------------------------------------------------
// mouse

static float mouse_read  (const char *vk) { int i=mouse_index(vk); return i>=0?input_state_read  (&mice[i]):0; }
static float mouse_delta (const char *vk) { int i=mouse_index(vk); return i>=0?input_state_delta (&mice[i]):0; }
static float mouse_up    (const char *vk) { int i=mouse_index(vk); return i>=0?input_state_up    (&mice[i]):0; }
static float mouse_down  (const char *vk) { int i=mouse_index(vk); return i>=0?input_state_down  (&mice[i]):0; }
static float mouse_idle  (const char *vk) { int i=mouse_index(vk); return i>=0?input_state_idle  (&mice[i]):1; }
static float mouse_held  (const char *vk) { int i=mouse_index(vk); return i>=0?input_state_held  (&mice[i]):0; }
static float mouse_tapped(const char *vk) { int i=mouse_index(vk); return i>=0?input_state_tapped(&mice[i]):0; }
static float mouse_repeat(const char *vk, unsigned d) { int i=mouse_index(vk); return i>=0?input_state_repeat(&mice[i],d):0; }
static float mouse_any(void) { for(int i=0;i<5;i++) if(mice[i].now) return 1.f; return 0.f; }

// @todo: SDL_Cursor * SDL_CreateColorCursor(SDL_Surface *surface, int hot_x, int hot_y);
int mouse_cursor(int shape) {
    static int last = 1;
    static array_(SDL_Cursor*) cursors; // { blank,default,crosshair,pointer,text,move,progress,wait,forbidden }
    if( shape < 0 ) return last;
    int tbl[] = {
        SDL_SYSTEM_CURSOR_DEFAULT,
        SDL_SYSTEM_CURSOR_DEFAULT,
        SDL_SYSTEM_CURSOR_CROSSHAIR,
        SDL_SYSTEM_CURSOR_POINTER,
        SDL_SYSTEM_CURSOR_TEXT,
        SDL_SYSTEM_CURSOR_MOVE,         /**< Four pointed arrow pointing north, south, east, and west. */
        SDL_SYSTEM_CURSOR_PROGRESS,     /**< Program is busy but still interactive. Usually it's WAIT with an arrow. */
        SDL_SYSTEM_CURSOR_WAIT,         /**< Wait. Usually an hourglass or watch or spinning ball. */
        SDL_SYSTEM_CURSOR_NOT_ALLOWED,  /**< Not permitted. Usually a slashed circle or crossbones. */
    }, cnt = COUNTOF(tbl);
    if( (shape %= cnt) != last ) {
        if( !cursors ) {
            for( int i = 0; i < SDL_SYSTEM_CURSOR_COUNT; ++i ) {
                SDL_Cursor *cursor = SDL_CreateSystemCursor((SDL_SystemCursor)i);
                array_push(cursors, cursor);
            }
        }
        if(!shape) SDL_SetWindowRelativeMouseMode(window.handle, 1), SDL_HideCursor();
        else SDL_SetCursor( cursors[ tbl[shape%cnt] ]), SDL_SetWindowRelativeMouseMode(window.handle, 0), SDL_ShowCursor();
    }
    return last = shape;
}

struct mouse_api mouse = { mouse_read,mouse_delta,mouse_up,mouse_down,mouse_idle,mouse_held,mouse_tapped,mouse_repeat,mouse_any,mouse_cursor };

// ---------------------------------------------------------------------------
// gamepad

static inputstate_t *gamepad_st(int gid, const char *vk) {
    if (gid<0||gid>=INPUT_MAX_GAMEPADS) return NULL;
    int i=gamepad_index(vk); if(i<0||i>=INPUT_MAX_GP) return NULL;
    return &gamepads[gid][i];
}
static float gamepad_connected(int gid) { return (gid>=0&&gid<INPUT_MAX_GAMEPADS&&gamepad_conn[gid])?1.f:0.f; }
static float gamepad_read  (int g, const char *vk) { inputstate_t*s=gamepad_st(g,vk); return s?input_state_read  (s):0; }
static float gamepad_delta (int g, const char *vk) { inputstate_t*s=gamepad_st(g,vk); return s?input_state_delta (s):0; }
static float gamepad_up    (int g, const char *vk) { inputstate_t*s=gamepad_st(g,vk); return s?input_state_up    (s):0; }
static float gamepad_down  (int g, const char *vk) { inputstate_t*s=gamepad_st(g,vk); return s?input_state_down  (s):0; }
static float gamepad_idle  (int g, const char *vk) { inputstate_t*s=gamepad_st(g,vk); return s?input_state_idle  (s):1; }
static float gamepad_held  (int g, const char *vk) { inputstate_t*s=gamepad_st(g,vk); return s?input_state_held  (s):0; }
static float gamepad_tapped(int g, const char *vk) { inputstate_t*s=gamepad_st(g,vk); return s?input_state_tapped(s):0; }
static float gamepad_repeat(int g, const char *vk, unsigned d) { inputstate_t*s=gamepad_st(g,vk); return s?input_state_repeat(s,d):0; }
static float gamepad_any(int gid) {
    if(gid<0||gid>=INPUT_MAX_GAMEPADS) return 0.f;
    for(int i=0;i<INPUT_MAX_GP;i++) if(gamepads[gid][i].now!=0.f) return 1.f; return 0.f;
}

struct gamepad_api gamepad = {
    gamepad_connected, gamepad_read,gamepad_delta,
    gamepad_up,gamepad_down,gamepad_idle,gamepad_held,gamepad_tapped,gamepad_repeat,gamepad_any
};

// ----------------------------------------------------------------------------
// rumble/haptic

// device haptic: mobile / misc devices
static SDL_Haptic *haptic = NULL;

static void haptic_init(void) { // @todo: multi-haptic: any other device id > 0
    if (haptic) return;
    int count = 0;
    SDL_HapticID *ids = SDL_GetHaptics(&count);
    if (ids && count > 0) {
        haptic = SDL_OpenHaptic(ids[0]); // first primary device
        if (haptic) SDL_InitHapticRumble(haptic);
        SDL_free(ids);
    }
}

static bool rumble_device(float amount, unsigned ms) {
    haptic_init();
    if (!haptic) return false;
    if (amount <= 0.f || ms == 0) return SDL_StopHapticRumble(haptic);
    return SDL_PlayHapticRumble(haptic, amount, ms);
}
static bool rumble_motors(int gid, float lo, float hi, unsigned ms) {
    if (gid < 0 || gid >= INPUT_MAX_GAMEPADS || !gamepad_handlers[gid]) return false;
    return SDL_RumbleGamepad(gamepad_handlers[gid], (Uint16)(lo * 65535.f), (Uint16)(hi * 65535.f), ms);
}
static bool rumble_triggers(int gid, float left, float right, unsigned ms) {
    if (gid < 0 || gid >= INPUT_MAX_GAMEPADS || !gamepad_handlers[gid]) return false;
    return SDL_RumbleGamepadTriggers(gamepad_handlers[gid], (Uint16)(left * 65535.f), (Uint16)(right * 65535.f), ms);
}

struct rumble_api rumble = { .device = rumble_device, .motors = rumble_motors, .triggers = rumble_triggers };

// ---------------------------------------------------------------------------
// touch

static finger_t *input_get_finger(int id) { return (id>=0&&id<INPUT_MAX_FINGERS)?&fingers[id]:NULL; }

static int   touch_count  (void)     { return fingers_count; }
static float touch_x      (int id)   { finger_t*f=input_get_finger(id); return f?f->x:0; }
static float touch_y      (int id)   { finger_t*f=input_get_finger(id); return f?f->y:0; }
static float touch_press  (int id)   { finger_t*f=input_get_finger(id); return f?f->pressure:0; }
static float touch_dx     (int id)   { finger_t*f=input_get_finger(id); return f?(f->x-f->px):0; }
static float touch_dy     (int id)   { finger_t*f=input_get_finger(id); return f?(f->y-f->py):0; }
static float touch_down   (int id)   { finger_t*f=input_get_finger(id); return (f&&f->active&&!f->was_active)?1.f:0.f; }
static float touch_up     (int id)   { finger_t*f=input_get_finger(id); return (f&&!f->active&&f->was_active)?1.f:0.f; }
static float touch_held   (int id)   { finger_t*f=input_get_finger(id); return (f&&f->active&&f->was_active)?1.f:0.f; }
static float touch_tapped (int id)   {
    finger_t*f=input_get_finger(id);
    if(!f||f->active||!f->was_active) return 0.f;
    return (f->up_frame==input_frame&&f->down_frame>=0)?1.f:0.f;
}
static float touch_any    (void)     { return fingers_count>0?1.f:0.f; }
static float touch_swipe_x(int id)   { return touch_dx(id); }
static float touch_swipe_y(int id)   { return touch_dy(id); }
static float touch_pinch  (void)     { return input_pinch_delta; }
static float touch_rotate (void)     { return input_rotate_delta; }

struct touch_api touch = {
    touch_count, touch_x,touch_y,touch_press,
    touch_dx,touch_dy,
    touch_down,touch_up,touch_held,touch_tapped,touch_any,
    touch_swipe_x,touch_swipe_y,touch_pinch,touch_rotate
};

// ---------------------------------------------------------------------------
// gesture

static bool  gesture_record    (const char *name) {
    Gesture_RecordGesture(-1);
    SDL_snprintf(dollar_rec_name,sizeof dollar_rec_name,"%s",name);
    dollar_recording=1; return 1;
}
static bool  gesture_save      (const char *file) {
    SDL_IOStream *rw=SDL_IOFromFile(file,"wb"); if(!rw) return 0;
    bool ok=Gesture_SaveAllDollarTemplates(rw); SDL_CloseIO(rw); return ok;
}
static bool  gesture_load      (const char *file) {
    SDL_IOStream *rw=SDL_IOFromFile(file,"rb"); if(!rw) return 0;
    bool ok=Gesture_LoadDollarTemplates(-1,rw); SDL_CloseIO(rw); return ok;
}
static float gesture_recognized(const char *name) {
    for(int i=0;i<dollar_count;i++)
        if(!SDL_strcasecmp(dollars[i].name,name)){ float s=dollars[i].score; dollars[i].score=0; return s; }
    return 0.f;
}
static float gesture_swipe_up   (void) { bool v=input_gest.swipe_up;    input_gest.swipe_up=0;    return v?1.f:0.f; }
static float gesture_swipe_down (void) { bool v=input_gest.swipe_down;  input_gest.swipe_down=0;  return v?1.f:0.f; }
static float gesture_swipe_left (void) { bool v=input_gest.swipe_left;  input_gest.swipe_left=0;  return v?1.f:0.f; }
static float gesture_swipe_right(void) { bool v=input_gest.swipe_right; input_gest.swipe_right=0; return v?1.f:0.f; }
static float gesture_tap        (int n) { return ((n==1&&input_gest.tap1)||(n==2&&input_gest.tap2))?1.f:0.f; }
static float gesture_double_tap (int n) { return ((n==1&&input_gest.dtap1)||(n==2&&input_gest.dtap2))?1.f:0.f; }

struct gesture_api gesture = {
    gesture_record,gesture_save,gesture_load,gesture_recognized,
    gesture_swipe_up,gesture_swipe_down,gesture_swipe_left,gesture_swipe_right,
    gesture_tap,gesture_double_tap
};

// ---------------------------------------------------------------------------
// Lua VM

static lua_State *input_L = NULL;

static const char *input_LUA =

// unit helpers
"function ms(x)     return {unit='ms',    v=x} end\n"
"function frames(x) return {unit='frames',v=x} end\n"
"local function to_frames(t)\n"
"    if type(t)=='number' then return t end\n"
"    if t.unit=='frames'  then return t.v end\n"
"    return math.ceil(t.v*60/1000)\n"
"end\n"

// low-level state accessors (use gp[0] as default device for bare expressions)
"function now(name)\n"
"    if name:find('.',1,true) then\n"
"        local i=__gp_idx(0,name); if i>=0 then return __gp_now(0,i) end; return 0\n"
"    end\n"
"    local i=__kb_idx(name); if i>=0 then return __kb_now(i) end\n"
"    i=__gp_idx(0,name);     if i>=0 then return __gp_now(0,i) end\n"
"    i=__ms_idx(name);       if i>=0 then return __ms_now(i) end\n"
"    return 0\n"
"end\n"
"function prev(name)\n"
"    if name:find('.',1,true) then\n"
"        local i=__gp_idx(0,name); if i>=0 then return __gp_prev(0,i) end; return 0\n"
"    end\n"
"    local i=__kb_idx(name); if i>=0 then return __kb_prev(i) end\n"
"    i=__gp_idx(0,name);     if i>=0 then return __gp_prev(0,i) end\n"
"    i=__ms_idx(name);       if i>=0 then return __ms_prev(i) end\n"
"    return 0\n"
"end\n"
"function idle(name)    return now(name)==0 end\n"
"function held(name)    return now(name)~=0 end\n"
"function down(name)    return now(name)~=0 and prev(name)==0 end\n"
"function up(name)      return now(name)==0 and prev(name)~=0 end\n"
"function delta(name)   return now(name)-prev(name) end\n"
"function pressed(name,dz) dz=dz or 0.5; return math.abs(now(name))>dz and 1 or 0 end\n"
"function clicked(name,window)\n"
"    if not down(name) then return 0 end\n"
"    local i=__kb_idx(name); if i<0 then return 0 end\n"
"    local win=window and to_frames(window) or 10\n"
"    local uf=__kb_up_frame(i); local df=__kb_down_frame(i)\n"
"    return (uf>=df-win and uf<__frame()) and 1 or 0\n"
"end\n"

// device helpers: usable directly in input.bind expressions
"function keyboard(vk)    local i=__kb_idx(vk); return i>=0 and __kb_now(i)~=0 end\n"
"function mouse(vk)       local i=__ms_idx(vk); return i>=0 and __ms_now(i)~=0 end\n"
"function gamepad(gid,vk) local i=__gp_idx(gid,vk); return i>=0 and __gp_now(gid,i)~=0 end\n"

// touch helpers — prefixed to avoid colliding with touch C struct name
"function touch_count()     return __touch_count() end\n"
"function touch_any()       return __touch_count()>0 end\n"
"function touch_down(id)    return __touch_down(id or 0)~=0 end\n"
"function touch_up(id)      return __touch_up(id or 0)~=0 end\n"
"function touch_held(id)    return __touch_held(id or 0)~=0 end\n"
"function touch_tapped(id)  return __touch_tapped(id or 0)~=0 end\n"
"function touch_x(id)       return __touch_x(id or 0) end\n"
"function touch_y(id)       return __touch_y(id or 0) end\n"
"function touch_dx(id)      return __touch_dx(id or 0) end\n"
"function touch_dy(id)      return __touch_dy(id or 0) end\n"
"function touch_pressure(id)return __touch_pressure(id or 0) end\n"
"function touch_pinch()     return __touch_pinch() end\n"
"function touch_rotate()    return __touch_rotate() end\n"

// gesture helpers
"function swipe_up()        return __gest_swipe_u()~=0 end\n"
"function swipe_down()      return __gest_swipe_d()~=0 end\n"
"function swipe_left()      return __gest_swipe_l()~=0 end\n"
"function swipe_right()     return __gest_swipe_r()~=0 end\n"
"function tap(n)            return __gest_tap(n or 1)~=0 end\n"
"function double_tap(n)     return __gest_dtap(n or 1)~=0 end\n"
"function recognized(name)  return __gest_recog(name) end\n"

// internal raw helpers for rewriter
"function __gp_raw(gid,name) return __gp_now(gid,__gp_idx(gid,name)) end\n"
"function __ms_raw(name)     return __ms_now(__ms_idx(name)) end\n"
"function __any_key()   for i=0,511 do if __kb_now(i)~=0 then return 1 end end return 0 end\n"
"function __any_gp(gid) for i=0,127 do if __gp_now(gid,i)~=0 then return 1 end end return 0 end\n"
"function __any_ms()    for i=0,7   do if __ms_now(i)~=0 then return 1 end end return 0 end\n"

// sandbox — built AFTER all helpers are defined
"local _senv={\n"
"    math={abs=math.abs,ceil=math.ceil,floor=math.floor,min=math.min,max=math.max,sqrt=math.sqrt},\n"
"    now=now,prev=prev,idle=idle,held=held,down=down,up=up,\n"
"    delta=delta,pressed=pressed,clicked=clicked,\n"
"    ms=ms,frames=frames,\n"
"    keyboard=keyboard,mouse=mouse,gamepad=gamepad,\n"
"    touch_count=touch_count,touch_any=touch_any,\n"
"    touch_down=touch_down,touch_up=touch_up,touch_held=touch_held,touch_tapped=touch_tapped,\n"
"    touch_x=touch_x,touch_y=touch_y,touch_dx=touch_dx,touch_dy=touch_dy,\n"
"    touch_pressure=touch_pressure,touch_pinch=touch_pinch,touch_rotate=touch_rotate,\n"
"    swipe_up=swipe_up,swipe_down=swipe_down,swipe_left=swipe_left,swipe_right=swipe_right,\n"
"    tap=tap,double_tap=double_tap,recognized=recognized,\n"
"    __any_key=__any_key,__any_gp=__any_gp,__any_ms=__any_ms,\n"
"    __gp_raw=__gp_raw,__ms_raw=__ms_raw,\n"
"}\n"
"_senv._ENV=_senv\n"

// token-level rewriter
"local FUNCS={}\n"
"for _,f in ipairs({\n"
"    'idle','down','held','up','delta','pressed','clicked','now','prev','ms','frames',\n"
"    'keyboard','mouse','gamepad',\n"
"    'touch_count','touch_any','touch_down','touch_up','touch_held','touch_tapped',\n"
"    'touch_x','touch_y','touch_dx','touch_dy','touch_pressure','touch_pinch','touch_rotate',\n"
"    'swipe_up','swipe_down','swipe_left','swipe_right','tap','double_tap','recognized',\n"
"}) do FUNCS[f]=true end\n"
"local KW={['and']=true,['or']=true,['not']=true,['true']=true,['false']=true,['nil']=true}\n"

"local function rewrite(expr,is_gp)\n"
"    local s=expr:gsub('&&',' and '):gsub('||',' or '):gsub('!([%w_])','__NOT__%1')\n"
"    local out,i,len,fdepth={},1,#s,0\n"
"    while i<=len do\n"
"        local c=s:sub(i,i)\n"
"        if c:match('%s') then out[#out+1]=c;i=i+1\n"
"        elseif c==')' then\n"
"            out[#out+1]=c;i=i+1\n"
"            if fdepth>0 then fdepth=fdepth-1 end\n"
"        elseif c=='(' then out[#out+1]=c;i=i+1;fdepth=fdepth+1\n"
"        elseif c=='*' then out[#out+1]=is_gp and '__any_gp(0)' or '__any_key()';i=i+1\n"
"        elseif c:match('%d') or (c=='.' and s:sub(i+1,i+1):match('%d')) then\n"
"            local j=i; while j<=len and s:sub(j,j):match('[%d%.]') do j=j+1 end\n"
"            out[#out+1]=s:sub(i,j-1);i=j\n"
"        elseif c:match('[%a_]') then\n"
"            local j=i; while j<=len and s:sub(j,j):match('[%w_.]') do j=j+1 end\n"
"            local tok=s:sub(i,j-1);i=j\n"
"            local k=i; while k<=len and s:sub(k,k)==' ' do k=k+1 end\n"
"            local call=s:sub(k,k)=='('\n"
"            if tok:sub(1,7)=='__NOT__' then\n"
"                out[#out+1]='(not held(\"'..tok:sub(8)..'\"))'\n"
"            elseif call and FUNCS[tok] then\n"
"                out[#out+1]=tok;out[#out+1]=s:sub(i,k);i=k+1\n"
"                fdepth=fdepth+1\n"  // inside FUNCS call arg list
"                while i<=len and s:sub(i,i)==' ' do i=i+1 end\n"
"                if s:sub(i,i):match('[%a_]') then\n"
"                    local m=i\n"
"                    while m<=len and s:sub(m,m)~=',' and s:sub(m,m)~=')' do m=m+1 end\n"
"                    local arg=s:sub(i,m-1):match('^(.-)%s*$')\n"
"                    out[#out+1]='\"'..arg..'\"';i=m\n"
"                end\n"
"            elseif call then out[#out+1]=tok\n"
"            elseif KW[tok] then out[#out+1]=tok\n"
"            elseif fdepth>0 then\n"  // inside a FUNCS call: quote as string
"                out[#out+1]='\"'..tok..'\"'\n"
"            else out[#out+1]=is_gp and '__gp_raw(0,\"'..tok..'\")'  or 'held(\"'..tok..'\")'  \n"
"            end\n"
"        else out[#out+1]=c;i=i+1 end\n"
"    end\n"
"    return table.concat(out)\n"
"end\n"
 
"local function eval(expr,is_gp)\n"
"    local rw=rewrite(expr,is_gp)\n"
"    local fn,err=load('return '..rw,'input','t',_senv)\n"
"    if not fn then io.stderr:write('expr ['..expr..']: '..tostring(err)..'\\n'); return 0/0 end\n"
"    local ok,r=pcall(fn)\n"
"    if not ok then io.stderr:write('eval ['..rw..']: '..tostring(r)..'\\n'); return 0/0 end\n"
"    if r==true then return 1 end; if r==false then return 0 end\n"
"    return tonumber(r) or 0/0\n"
"end\n"
"function __eval_kb(expr) return eval(expr,false) end\n"
"function __eval_gp(expr) return eval(expr,true)  end\n"
"_action_refs={}\n"
;
 
// ---------------------------------------------------------------------------
// Lua C callbacks — keyboard / mouse / gamepad

static int lc_kb_idx    (lua_State*l) { lua_pushinteger(l,input_key_index(luaL_checkstring(l,1))); return 1; }
static int lc_gp_idx    (lua_State*l) { lua_pushinteger(l,gamepad_index(luaL_checkstring(l,2))); return 1; }
static int lc_ms_idx    (lua_State*l) { lua_pushinteger(l,mouse_index(luaL_checkstring(l,1))); return 1; }
static int lc_kb_now    (lua_State*l) { int i=(int)luaL_checkinteger(l,1); lua_pushnumber(l,i>=0&&i<INPUT_MAX_KEYS?keyboards[i].now :0); return 1; }
static int lc_kb_prev   (lua_State*l) { int i=(int)luaL_checkinteger(l,1); lua_pushnumber(l,i>=0&&i<INPUT_MAX_KEYS?keyboards[i].prev:0); return 1; }
static int lc_kb_down_f (lua_State*l) { int i=(int)luaL_checkinteger(l,1); lua_pushinteger(l,i>=0&&i<INPUT_MAX_KEYS?keyboards[i].down_frame:-1); return 1; }
static int lc_kb_up_f   (lua_State*l) { int i=(int)luaL_checkinteger(l,1); lua_pushinteger(l,i>=0&&i<INPUT_MAX_KEYS?keyboards[i].up_frame  :-1); return 1; }
static int lc_gp_now    (lua_State*l) {
    int g=(int)luaL_checkinteger(l,1),i=(int)luaL_checkinteger(l,2);
    lua_pushnumber(l,(g>=0&&g<INPUT_MAX_GAMEPADS&&i>=0&&i<INPUT_MAX_GP)?gamepads[g][i].now:0); return 1;
}
static int lc_gp_prev   (lua_State*l) {
    int g=(int)luaL_checkinteger(l,1),i=(int)luaL_checkinteger(l,2);
    lua_pushnumber(l,(g>=0&&g<INPUT_MAX_GAMEPADS&&i>=0&&i<INPUT_MAX_GP)?gamepads[g][i].prev:0); return 1;
}
static int lc_ms_now    (lua_State*l) { int i=(int)luaL_checkinteger(l,1); lua_pushnumber(l,i>=0&&i<INPUT_MAX_MS?mice[i].now :0); return 1; }
static int lc_ms_prev   (lua_State*l) { int i=(int)luaL_checkinteger(l,1); lua_pushnumber(l,i>=0&&i<INPUT_MAX_MS?mice[i].prev:0); return 1; }
static int lc_frame     (lua_State*l) { lua_pushinteger(l,input_frame);   return 1; }
static int lc_time_ms   (lua_State*l) { lua_pushnumber (l,input_time_ms); return 1; }

// ---------------------------------------------------------------------------
// Lua C callbacks — touch

static int lc_touch_count   (lua_State*l) { lua_pushinteger(l,touch_count());                return 1; }
static int lc_touch_x       (lua_State*l) { lua_pushnumber (l,touch_x      (luaL_checkinteger(l,1))); return 1; }
static int lc_touch_y       (lua_State*l) { lua_pushnumber (l,touch_y      (luaL_checkinteger(l,1))); return 1; }
static int lc_touch_pressure(lua_State*l) { lua_pushnumber (l,touch_press  (luaL_checkinteger(l,1))); return 1; }
static int lc_touch_dx      (lua_State*l) { lua_pushnumber (l,touch_dx     (luaL_checkinteger(l,1))); return 1; }
static int lc_touch_dy      (lua_State*l) { lua_pushnumber (l,touch_dy     (luaL_checkinteger(l,1))); return 1; }
static int lc_touch_down    (lua_State*l) { lua_pushnumber (l,touch_down   (luaL_checkinteger(l,1))); return 1; }
static int lc_touch_up      (lua_State*l) { lua_pushnumber (l,touch_up     (luaL_checkinteger(l,1))); return 1; }
static int lc_touch_held    (lua_State*l) { lua_pushnumber (l,touch_held   (luaL_checkinteger(l,1))); return 1; }
static int lc_touch_tapped  (lua_State*l) { lua_pushnumber (l,touch_tapped (luaL_checkinteger(l,1))); return 1; }
static int lc_touch_pinch   (lua_State*l) { lua_pushnumber (l,touch_pinch  ()); return 1; }
static int lc_touch_rotate  (lua_State*l) { lua_pushnumber (l,touch_rotate ()); return 1; }

// ---------------------------------------------------------------------------
// Lua C callbacks — gesture

static int lc_gest_swipe_u (lua_State*l) { lua_pushnumber(l,gesture_swipe_up   ()); return 1; }
static int lc_gest_swipe_d (lua_State*l) { lua_pushnumber(l,gesture_swipe_down ()); return 1; }
static int lc_gest_swipe_l (lua_State*l) { lua_pushnumber(l,gesture_swipe_left ()); return 1; }
static int lc_gest_swipe_r (lua_State*l) { lua_pushnumber(l,gesture_swipe_right()); return 1; }
static int lc_gest_tap     (lua_State*l) { lua_pushnumber(l,gesture_tap       (luaL_checkinteger(l,1))); return 1; }
static int lc_gest_dtap    (lua_State*l) { lua_pushnumber(l,gesture_double_tap(luaL_checkinteger(l,1))); return 1; }
static int lc_gest_recog   (lua_State*l) { lua_pushnumber(l,gesture_recognized(luaL_checkstring (l,1))); return 1; }

// ---------------------------------------------------------------------------
// named action bindings

static void input_init(void);

static bool input_bind(const char *name, const char *expr) {
    input_init();
    // update existing
    for (int i=0;i<bindings_count;i++) {
        if (!SDL_strcasecmp(bindings[i].name,name)) {
            lua_getglobal(input_L,"_action_refs");
            lua_pushinteger(input_L,i); lua_pushstring(input_L,expr);
            lua_settable(input_L,-3); lua_pop(input_L,1);
            return 1;
        }
    }
    if (bindings_count>=INPUT_MAX_BINDINGS) return 0;
    int idx=bindings_count++;
    SDL_snprintf(bindings[idx].name,sizeof bindings[0].name,"%s",name);
    lua_getglobal(input_L,"_action_refs");
    lua_pushinteger(input_L,idx); lua_pushstring(input_L,expr);
    lua_settable(input_L,-3); lua_pop(input_L,1);
    return 1;
}

static float input_action(const char *name) {
    for (int i=0;i<bindings_count;i++) {
        if (!SDL_strcasecmp(bindings[i].name,name)) {
            lua_getglobal(input_L,"__eval_kb");
            lua_getglobal(input_L,"_action_refs");
            lua_pushinteger(input_L,i); lua_gettable(input_L,-2); lua_remove(input_L,-2);
            if (lua_pcall(input_L,1,1,0)!=LUA_OK) { lua_pop(input_L,1); return (float)NAN; }
            float r=(float)lua_tonumber(input_L,-1); lua_pop(input_L,1); return r;
        }
    }
    return (float)NAN;
}

static bool input_unbind(const char *name) {
    bool any=0;
    int nlen=(int)SDL_strlen(name);
    bool wc=nlen>0&&name[nlen-1]=='*';
    int plen=wc?nlen-1:nlen;
    for (int i=0;i<bindings_count;) {
        bool match = !SDL_strcasecmp(name,"*") || (wc ? SDL_strncasecmp(bindings[i].name,name,plen)==0
                                              : SDL_strcasecmp (bindings[i].name,name)==0);
        if (match) {
            lua_getglobal(input_L,"_action_refs");
            lua_pushinteger(input_L,i); lua_pushnil(input_L);
            lua_settable(input_L,-3); lua_pop(input_L,1);
            bindings[i]=bindings[--bindings_count];
            any=1;
        } else i++;
    }
    return any;
}

static float input_eval(const char *expr) {
    input_init();
    lua_getglobal(input_L,"__eval_kb");
    lua_pushstring(input_L,expr);
    if (lua_pcall(input_L,1,1,0)!=LUA_OK) { lua_pop(input_L,1); return (float)NAN; }
    float r=(float)lua_tonumber(input_L,-1); lua_pop(input_L,1); return r;
}

struct input_api input = { input_bind,input_action,input_unbind,input_eval };

// ---------------------------------------------------------------------------
// gamepad management

static void input_open_gamepads(void) {
    for (int i=0;i<INPUT_MAX_GAMEPADS;i++) {
        if (gamepad_handlers[i]) { SDL_CloseGamepad(gamepad_handlers[i]); gamepad_handlers[i]=NULL; }
        gamepad_conn[i]=0;
    }
    int count=0; SDL_JoystickID *ids=SDL_GetGamepads(&count);
    if (ids) {
        for (int i=0;i<count&&i<INPUT_MAX_GAMEPADS;i++) {
            gamepad_handlers[i]=SDL_OpenGamepad(ids[i]);
            gamepad_conn[i]=(gamepad_handlers[i]!=NULL);
        }
        SDL_free(ids);
    }
}

// ---------------------------------------------------------------------------
// touch finger slot management

static int input_find_finger(SDL_FingerID fid) {
    for (int i=0;i<INPUT_MAX_FINGERS;i++)
        if (fingers[i].active&&fingers[i].sdl_id==fid) return i;
    return -1;
}
static int input_alloc_finger(SDL_FingerID fid) {
    for (int i=0;i<INPUT_MAX_FINGERS;i++)
        if (!fingers[i].active) { fingers[i].sdl_id=fid; return i; }
    return -1;
}

// ---------------------------------------------------------------------------
// init

static void input_init(void) {
    if (input_L) return;
    input_L=luaL_newstate(); luaL_openlibs(input_L);

    // keyboard / mouse / gamepad
    lua_register(input_L,"__kb_idx",        lc_kb_idx);
    lua_register(input_L,"__gp_idx",        lc_gp_idx);
    lua_register(input_L,"__ms_idx",        lc_ms_idx);
    lua_register(input_L,"__kb_now",        lc_kb_now);
    lua_register(input_L,"__kb_prev",       lc_kb_prev);
    lua_register(input_L,"__kb_down_frame", lc_kb_down_f);
    lua_register(input_L,"__kb_up_frame",   lc_kb_up_f);
    lua_register(input_L,"__gp_now",        lc_gp_now);
    lua_register(input_L,"__gp_prev",       lc_gp_prev);
    lua_register(input_L,"__ms_now",        lc_ms_now);
    lua_register(input_L,"__ms_prev",       lc_ms_prev);
    lua_register(input_L,"__frame",         lc_frame);
    lua_register(input_L,"__time_ms",       lc_time_ms);
    // touch
    lua_register(input_L,"__touch_count",   lc_touch_count);
    lua_register(input_L,"__touch_x",       lc_touch_x);
    lua_register(input_L,"__touch_y",       lc_touch_y);
    lua_register(input_L,"__touch_pressure",lc_touch_pressure);
    lua_register(input_L,"__touch_dx",      lc_touch_dx);
    lua_register(input_L,"__touch_dy",      lc_touch_dy);
    lua_register(input_L,"__touch_down",    lc_touch_down);
    lua_register(input_L,"__touch_up",      lc_touch_up);
    lua_register(input_L,"__touch_held",    lc_touch_held);
    lua_register(input_L,"__touch_tapped",  lc_touch_tapped);
    lua_register(input_L,"__touch_pinch",   lc_touch_pinch);
    lua_register(input_L,"__touch_rotate",  lc_touch_rotate);
    // gesture
    lua_register(input_L,"__gest_swipe_u",  lc_gest_swipe_u);
    lua_register(input_L,"__gest_swipe_d",  lc_gest_swipe_d);
    lua_register(input_L,"__gest_swipe_l",  lc_gest_swipe_l);
    lua_register(input_L,"__gest_swipe_r",  lc_gest_swipe_r);
    lua_register(input_L,"__gest_tap",      lc_gest_tap);
    lua_register(input_L,"__gest_dtap",     lc_gest_dtap);
    lua_register(input_L,"__gest_recog",    lc_gest_recog);

    if (luaL_dostring(input_L,input_LUA)!=LUA_OK)
        fprintf(stderr,"kit_input bootstrap: %s\n",lua_tostring(input_L,-1));

    SDL_memset(keyboards,0,sizeof keyboards);
    SDL_memset(gamepads,0,sizeof gamepads);
    SDL_memset(mice,0,sizeof mice);
    SDL_memset(fingers,0,sizeof fingers);
    Gesture_Init();
    input_open_gamepads();
}

// ---------------------------------------------------------------------------
// event pump

void input_pump_event(const SDL_Event *e) {
    input_init();

    switch (e->type) {

    case SDL_EVENT_KEY_DOWN:
        if (!e->key.repeat) {
            int sc=(int)e->key.scancode;
            if (sc>=0&&sc<INPUT_MAX_KEYS) { keyboards[sc].now=1; keyboards[sc].down_frame=input_frame; }
        }
        break;
    case SDL_EVENT_KEY_UP: {
        int sc=(int)e->key.scancode;
        if (sc>=0&&sc<INPUT_MAX_KEYS) { keyboards[sc].now=0; keyboards[sc].up_frame=input_frame; }
        break;
    }

    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
        int idx=e->button.button-1;
        if (idx>=0&&idx<8) { mice[idx].now=1; mice[idx].down_frame=input_frame; }
        break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
        int idx=e->button.button-1;
        if (idx>=0&&idx<8) { mice[idx].now=0; mice[idx].up_frame=input_frame; }
        break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
        int w=1,h=1; SDL_GetWindowSize(SDL_GetWindowFromEvent(e),&w,&h);
        mice[8].now=(float)e->motion.x; // /(float)(w?w:1);
        mice[9].now=(float)e->motion.y; // /(float)(h?h:1);
        break;
    }
    case SDL_EVENT_MOUSE_WHEEL:
        mice[10].now+=e->wheel.y;
        break;

    case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
        int btn=e->gbutton.button;
        for (int g=0;g<INPUT_MAX_GAMEPADS;g++)
            if (gamepad_handlers[g]&&SDL_GetGamepadID(gamepad_handlers[g])==e->gbutton.which) {
                if (btn>=0&&btn<GAMEPAD_AXIS_OFS) { gamepads[g][btn].now=1; gamepads[g][btn].down_frame=input_frame; }
                break;
            }
        break;
    }
    case SDL_EVENT_GAMEPAD_BUTTON_UP: {
        int btn=e->gbutton.button;
        for (int g=0;g<INPUT_MAX_GAMEPADS;g++)
            if (gamepad_handlers[g]&&SDL_GetGamepadID(gamepad_handlers[g])==e->gbutton.which) {
                if (btn>=0&&btn<GAMEPAD_AXIS_OFS) { gamepads[g][btn].now=0; gamepads[g][btn].up_frame=input_frame; }
                break;
            }
        break;
    }
    case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
        int idx=GAMEPAD_AXIS_OFS+e->gaxis.axis;
        for (int g=0;g<INPUT_MAX_GAMEPADS;g++)
            if (gamepad_handlers[g]&&SDL_GetGamepadID(gamepad_handlers[g])==e->gaxis.which) {
                if (idx>=GAMEPAD_AXIS_OFS&&idx<INPUT_MAX_GP) gamepads[g][idx].now=e->gaxis.value/32767.f;
                break;
            }
        break;
    }
    case SDL_EVENT_GAMEPAD_ADDED:
    case SDL_EVENT_GAMEPAD_REMOVED:
        input_open_gamepads();
        break;

    case SDL_EVENT_FINGER_DOWN: {
        int slot=input_alloc_finger(e->tfinger.fingerID);
        if (slot>=0) {
            finger_t *f=&fingers[slot];
            f->active=1; f->was_active=0;
            f->x=f->px=e->tfinger.x; f->y=f->py=e->tfinger.y;
            f->pressure=e->tfinger.pressure;
            f->down_frame=input_frame; f->up_frame=-1;
        }
        break;
    }
    case SDL_EVENT_FINGER_UP: {
        int slot=input_find_finger(e->tfinger.fingerID);
        if (slot>=0) {
            finger_t *f=&fingers[slot];
            f->x=e->tfinger.x; f->y=e->tfinger.y; f->pressure=0; f->up_frame=input_frame;
            // swipe detection on lift
            float dx=f->x-f->px, dy=f->y-f->py;
            if (fabsf(dx)>INPUT_SWIPE_VEL&&fabsf(dx)>fabsf(dy))
                { if(dx>0) input_gest.swipe_right=1; else input_gest.swipe_left=1; }
            if (fabsf(dy)>INPUT_SWIPE_VEL&&fabsf(dy)>fabsf(dx))
                { if(dy>0) input_gest.swipe_down=1; else input_gest.swipe_up=1; }
            // tap detection
            bool is_tap=(input_frame-f->down_frame)<20;
            if (is_tap) {
                if(fingers_count<=1) input_gest.tap1 = 1; else input_gest.tap2 = 1;
                float now_ms=(float)SDL_GetTicks();
                if (now_ms-input_gest.last_tap_ms<INPUT_DTAP_MS)
                    if(fingers_count<=1) input_gest.dtap1 = 1; else input_gest.dtap2 = 1;
                input_gest.last_tap_ms=now_ms;
            }
            f->active=0;
        }
        break;
    }
    case SDL_EVENT_FINGER_MOTION: {
        int slot=input_find_finger(e->tfinger.fingerID);
        if (slot>=0) {
            finger_t *f=&fingers[slot];
            f->px=f->x; f->py=f->y;
            f->x=e->tfinger.x; f->y=e->tfinger.y; f->pressure=e->tfinger.pressure;
        }
        break;
    }

    case GESTURE_DOLLARRECORD: {
        Gesture_DollarGestureEvent *ev = (Gesture_DollarGestureEvent*)e;
        if (dollar_recording&&dollar_count<INPUT_MAX_DOLLAR) {
            dollars[dollar_count].id=ev->gestureId;
            dollars[dollar_count].score=0;
            SDL_snprintf(dollars[dollar_count].name,64,"%s",dollar_rec_name);
            dollar_count++; dollar_recording=0;
        }
        break;
    }
    case GESTURE_DOLLARGESTURE: {
        Gesture_DollarGestureEvent *ev = (Gesture_DollarGestureEvent *)e;
        for (int i=0;i<dollar_count;i++)
            if (dollars[i].id==ev->gestureId)
                dollars[i].score=ev->error>0?1.f/ev->error:1.f;
        break;
    }
    case GESTURE_MULTIGESTURE: {
        Gesture_MultiGestureEvent *ev = (Gesture_MultiGestureEvent *)e;
        input_pinch_delta  +=ev->dDist;
        input_rotate_delta +=ev->dTheta*(180.f/3.14159265f);
        break;
    }

    default:
        GestureProcessEvent(e);
        break;
    }
}

// ---------------------------------------------------------------------------
// frame advance — call LAST in ev.tick

void input_next_frame(void) {
    for (int i=0;i<INPUT_MAX_KEYS;i++) keyboards[i].prev=keyboards[i].now;
    for (int g=0;g<INPUT_MAX_GAMEPADS;g++) for(int i=0;i<INPUT_MAX_GP;i++) gamepads[g][i].prev=gamepads[g][i].now;
    for (int i=0;i<INPUT_MAX_MS;i++) mice[i].prev=mice[i].now;
    mice[10].now=0;

    fingers_count=0;
    for (int i=0;i<INPUT_MAX_FINGERS;i++) {
        fingers[i].was_active=fingers[i].active;
        if (fingers[i].active) fingers_count++;
    }

    input_pinch_delta=0; input_rotate_delta=0;
    input_gest.tap1=input_gest.tap2=input_gest.dtap1=input_gest.dtap2=0;
    for (int i=0;i<dollar_count;i++) dollars[i].score=0;

    input_frame++;
    input_time_ms=(double)SDL_GetTicks();
}

// ---------------------------------------------------------------------------
// self-tests

#if TEST
AUTOTEST {
    input_init();
    #define INPUT_CHECK(expr,expected) do { \
        float got=(float)(expr),exp=(float)(expected); \
        int ok=test(got>=exp-0.001f&&got<=exp+0.001f); \
        if(!ok) fprintf(stderr,"[FAIL] %-44s => %.3f (expected %.3f)\n",#expr,got,exp); \
    } while(0)

    // keyboard
    keyboards[SDL_SCANCODE_A].now=1; keyboards[SDL_SCANCODE_A].down_frame=input_frame;
    INPUT_CHECK(keyboard.down("a"),1); INPUT_CHECK(keyboard.held("a"),1);
    INPUT_CHECK(keyboard.up("a"),  0); INPUT_CHECK(keyboard.idle("a"),0);
    INPUT_CHECK(keyboard.any(),    1);

    input_next_frame();
    INPUT_CHECK(keyboard.down("a"),0); INPUT_CHECK(keyboard.held("a"),1);

    input_next_frame();
    keyboards[SDL_SCANCODE_A].now=0; keyboards[SDL_SCANCODE_A].up_frame=input_frame;
    keyboards[SDL_SCANCODE_B].now=1; keyboards[SDL_SCANCODE_B].down_frame=input_frame;
    INPUT_CHECK(keyboard.up("a"),   1); INPUT_CHECK(keyboard.down("b"),1);
    INPUT_CHECK(keyboard.tapped("a"),1);

    input_next_frame();
    keyboards[SDL_SCANCODE_A].now=1; keyboards[SDL_SCANCODE_A].down_frame=input_frame;

    // gamepad
    gamepads[0][GAMEPAD_AXIS_OFS+SDL_GAMEPAD_AXIS_LEFTX].now =  0.8f;
    gamepads[0][GAMEPAD_AXIS_OFS+SDL_GAMEPAD_AXIS_LEFTX].prev=  0.3f;
    gamepads[0][GAMEPAD_AXIS_OFS+SDL_GAMEPAD_AXIS_LEFTY].now = -0.5f;
    gamepads[0][GAMEPAD_AXIS_OFS+SDL_GAMEPAD_AXIS_LEFTY].prev= -0.2f;
    INPUT_CHECK(gamepad.get(0,"lx"),   0.8f);
    INPUT_CHECK(gamepad.any(0),        1.0f);
    INPUT_CHECK(gamepad.delta(0,"lx"), 0.5f);

    // rumble
    rumble.device(0.8f, 300);            // mobile / haptic mouse buzz for 300ms
    rumble.device(0.f,  0);              // stop device haptic

    rumble.motors(0, 1.0f, 1.0f, 500);   // full gamepad rumble for 500ms
    rumble.motors(0, 0.3f, 0.7f, 200);   // lo=30% hi=70% asymmetric rumble
    rumble.motors(0, 0.f,  0.f, 0);      // stop gamepad motors

    rumble.triggers(0, 0.5f, 0.5f, 200); // Xbox/DualSense trigger motors for 200ms
    rumble.triggers(0, 0.f,  0.f, 0);    // stop trigger motors

    // input.bind / action / unbind
    input.bind("jump","keyboard(space) || gamepad(0,south)");
    INPUT_CHECK(input.action("jump"),0.0f);
    keyboards[SDL_SCANCODE_SPACE].now=1; keyboards[SDL_SCANCODE_SPACE].down_frame=input_frame;
    INPUT_CHECK(input.action("jump"),1.0f);
    input.unbind("jump");
    INPUT_CHECK(isnan(input.action("jump")),1);

    // input.bind with touch/gesture expressions
    input.bind("tap_fire", "touch_tapped(0)");
    input.bind("go_back",  "swipe_left() || keyboard(escape)");
    input.bind("pinch_zoom","touch_pinch() > 0.0");

    // restore
    SDL_memset(keyboards,0,sizeof keyboards);
    SDL_memset(gamepads,0,sizeof gamepads);
    SDL_memset(mice,0,sizeof mice);
    SDL_memset(fingers,0,sizeof fingers);
    input_frame=0;
    #undef INPUT_CHECK
}
#endif // TESTS

#endif // KIT_CODE
