// WIP! WIP! WIP! WIP! 

/*
 * This code is public domain. Feel free to use it for any purpose!
 */

#include "kit.h"
#include "_dd.h"
const char *hints;

// [ref] https://wiki.libsdl.org/SDL3/BestKeyboardPractices


#if 0

mouse(L) .any M R 1 2 3 X Y W
keyboard .any 
gamepad .any
clipboard, ime
touchpad
sensor
combo()
event < digitalinput()
time imgui
-INF[up] <0 idle.. ==0 [down] ... >0 [pressed] 
repeat(), click(), click2()


typedef struct PlayerInput PlayerInput;
struct PlayerInput
{
  unsigned up : 1;
  unsigned down : 1;
  unsigned left : 1;
  unsigned right : 1;
  unsigned select : 1;
  unsigned start : 1;
  unsigned a : 1;
  unsigned b : 1;
  unsigned x : 1;
  unsigned y : 1;
  unsigned l1 : 1;
  unsigned r1 : 1;
  unsigned l2 : 1;
  unsigned r2 : 1;
  unsigned l3 : 1;
  unsigned r3 : 1;
  float2 joystick;
};

[eKey_up] = SDL_SCANCODE_UP;
[eKey_down] = SDL_SCANCODE_DOWN;
[eKey_left] = SDL_SCANCODE_LEFT;
[eKey_right] = SDL_SCANCODE_RIGHT;
[eKey_select] = SDL_SCANCODE_SPACE;
[eKey_start] = SDL_SCANCODE_RETURN;
[eKey_a] = SDL_SCANCODE_A;
[eKey_b] = SDL_SCANCODE_B;
[eKey_x] = SDL_SCANCODE_X;
[eKey_y] = SDL_SCANCODE_Y;
[eKey_l1] = SDL_SCANCODE_1;
[eKey_r1] = SDL_SCANCODE_6;
[eKey_l2] = SDL_SCANCODE_2;
[eKey_r2] = SDL_SCANCODE_5;
[eKey_l3] = SDL_SCANCODE_3;
[eKey_r3] = SDL_SCANCODE_4;

ei->input[i].up = !!(SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_UP));
ei->input[i].down = !!(SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_DOWN));
ei->input[i].left = !!(SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_LEFT));
ei->input[i].right = !!(SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_RIGHT));
ei->input[i].select = !!(SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_BACK));
ei->input[i].start = !!(SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_START));
ei->input[i].a = !!(SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_A));
ei->input[i].b = !!(SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_B));
ei->input[i].x = !!(SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_X));
ei->input[i].y = !!(SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_Y));
ei->input[i].l1 = !!(SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_LEFTSHOULDER));
ei->input[i].r1 = !!(SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER));
ei->input[i].l2 = !!(SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 16384);
ei->input[i].r2 = !!(SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 16384);
ei->input[i].l3 = !!(SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_LEFTSTICK));
ei->input[i].r3 = !!(SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_RIGHTSTICK));

ei->input[i].joystick.x = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTX);
ei->input[i].joystick.y = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTY);

#endif



/* Joysticks are low-level interfaces: there's something with a bunch of
   buttons, axes and hats, in no understood order or position. This is
   a flexible interface, but you'll need to build some sort of configuration
   UI to let people tell you what button, etc, does what. On top of this
   interface, SDL offers the "gamepad" API, which works with lots of devices,
   and knows how to map arbitrary buttons and such to look like an
   Xbox/PlayStation/etc gamepad. This is easier, and better, for many games,
   but isn't necessarily a good fit for complex apps and hardware. A flight
   simulator, a realistic racing game, etc, might want the joystick interface
   instead of gamepads. */

SDL_Window *hwnd;




struct Player {
    SDL_MouseID    mouse;
    SDL_KeyboardID keyboard;
    SDL_JoystickID gamepad;
};

array_(struct Player) players;
AUTORUN { array_resize(players, 1+4); }

int num_players() {
    int c = 0;
    for( int i = 0; i < array_count(players); ++i) c += players[i].mouse || players[i].keyboard;
    return c;
}

const char *debug_player(int i) {
    if( i >= 1 && i < array_count(players) ) {
        return va("[%d] mouse(%u) keyboard(%u) gamepad(%u)", i, players[i].mouse, players[i].keyboard, players[i].gamepad);
    }
    return "";
}

static int player_findoradd_mouse(SDL_MouseID mouse) {
    for (int i = 1; i < array_count(players); i++) {
        if (players[i].mouse == mouse) return i;
    }
    for (int i = 1; i < array_count(players); i++) {
        if (players[i].mouse == 0) return players[i].mouse = mouse, i;
    }
    return 0;
}
static int player_del_mouse(SDL_MouseID mouse) {
    for (int i = 1; i < array_count(players); i++) {
        if (players[i].mouse == mouse) return players[i].mouse = 0, i;
    }
    return 0;
}

static int player_findoradd_keyboard(SDL_KeyboardID keyboard) {
    for (int i = 1; i < array_count(players); i++) {
        if (players[i].keyboard == keyboard) return i;
    }
    for (int i = 1; i < array_count(players); i++) {
        if (players[i].keyboard == 0) return players[i].keyboard = keyboard, i;
    }
    return 0;
}
static int player_del_keyboard(SDL_KeyboardID keyboard) {
    for (int i = 1; i < array_count(players); i++) {
        if (players[i].keyboard == keyboard) return players[i].keyboard = 0, i;
    }
    return 0;
}

static int player_findoradd_gamepad(SDL_JoystickID gamepad) {
    for (int i = 1; i < array_count(players); i++) {
        if (players[i].gamepad == gamepad) return i;
    }
    for (int i = 1; i < array_count(players); i++) {
        if (players[i].gamepad == 0) return players[i].gamepad = gamepad, i;
    }
    return 0;
}
static int player_del_gamepad(SDL_JoystickID gamepad) {
    for (int i = 1; i < array_count(players); i++) {
        if (players[i].gamepad == gamepad) return players[i].gamepad = 0, i;
    }
    return 0;
}

static const char *battery_state_string(SDL_PowerState state) {
    switch (state) {
        default:
        case SDL_POWERSTATE_UNKNOWN: return "UNKNOWN";
        case SDL_POWERSTATE_ERROR: return "ERROR";
        case SDL_POWERSTATE_ON_BATTERY: return "ON BATTERY";
        case SDL_POWERSTATE_NO_BATTERY: return "NO BATTERY";
        case SDL_POWERSTATE_CHARGING: return "CHARGING";
        case SDL_POWERSTATE_CHARGED: return "CHARGED";
    }
}

SDL_Event *kit_event(SDL_Event *event) {

    ONCE
    if (!SDL_WasInit(SDL_INIT_GAMEPAD))
    if (!SDL_Init(SDL_INIT_GAMEPAD)) {
        SDL_Log("Couldn't initialize SDL gamepads: %s", SDL_GetError());
    }

    switch(event->type) {
        default:

        break; case SDL_EVENT_KEYBOARD_ADDED: {
            SDL_KeyboardID which = event->key.which;
            int index = player_findoradd_keyboard(which);
            if( index )
                dd_log("\4Keyboard [%d] added", index);
        }
        break; case SDL_EVENT_KEYBOARD_REMOVED: {
            SDL_KeyboardID which = event->key.which;
            int index = player_del_keyboard(which);
            if( index )
                dd_log("\2Keyboard [%d] removed", index);
        }
        break; case SDL_EVENT_KEY_DOWN: {
            SDL_KeyboardID which = event->key.which;
            int index = player_findoradd_keyboard(which);
            if( index ) {
                SDL_Keycode sym = event->key.key;
                dd_log("\6Keyboard [%d] key down [%u]", index, sym);
                if (sym == SDLK_W) ;
                if (sym == SDLK_A) ;
                if (sym == SDLK_S) ;
                if (sym == SDLK_D) ;
                if (sym == SDLK_SPACE) ;
            }
        }
        break; case SDL_EVENT_KEY_UP: {
            SDL_KeyboardID which = event->key.which;
            int index = player_findoradd_keyboard(which);
            if( index ) {
                SDL_Keycode sym = event->key.key;
                dd_log("\6Keyboard [%d] key up [%u]", index, sym);
                if (sym == SDLK_W) ;
                if (sym == SDLK_A) ;
                if (sym == SDLK_S) ;
                if (sym == SDLK_D) ;
                if (sym == SDLK_SPACE) ;
            }
        }

        break; case SDL_EVENT_MOUSE_ADDED: {
            SDL_MouseID which = event->button.which;
            int index = player_findoradd_mouse(which);
            if( index )
                dd_log("\4Mouse [%d] added", index);
        }
        break; case SDL_EVENT_MOUSE_REMOVED: {
            SDL_MouseID which = event->button.which;
            int index = player_del_mouse(which);
            if( index )
                dd_log("\2Mouse [%d] removed", index);
        }
        break; case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            SDL_MouseID which = event->button.which;
            int index = player_findoradd_mouse(which);
            if( index ) {
                //shoot(index);
            }
        }
        break; case SDL_EVENT_MOUSE_MOTION: {
            SDL_MouseID which = event->motion.which;
            int index = player_findoradd_mouse(which);
            if( index ) {
                //((int)event->motion.xrel) * 0x00080000;
                //((int)event->motion.yrel) * 0x00080000;
            }
        }
        break; case SDL_EVENT_MOUSE_WHEEL: {
        }

        break; case SDL_EVENT_GAMEPAD_ADDED: {
            /* this event is sent for each hotplugged stick, but also each already-connected gamepad during SDL_Init(). */
            SDL_JoystickID which = event->gdevice.which;
            SDL_Gamepad *gamepad = SDL_OpenGamepad(which);
            if( gamepad ) {
                int index = player_findoradd_gamepad(which);
                if( index ) {
                    char *mapping = SDL_GetGamepadMapping(gamepad);
                    dd_log("\4Gamepad [%d] added (%s) (mapping: %s)", index, SDL_GetGamepadName(gamepad), strvalid(mapping));
                    if( mapping ) SDL_free(mapping);
                } else {
                    SDL_CloseGamepad(gamepad);
                }
            }
        }
        break; case SDL_EVENT_GAMEPAD_REMOVED: {
            SDL_JoystickID which = event->gdevice.which;
            SDL_Gamepad *gamepad = SDL_GetGamepadFromID(which);
            if( gamepad ) {
                SDL_CloseGamepad(gamepad);  /* the gamepad was unplugged. */
                int index = player_del_gamepad(which);
                if( index )
                    dd_log("\2Gamepad [%d] removed", which);
            }
        }
        break; case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
            SDL_JoystickID which = event->gaxis.which;
            int index = player_findoradd_gamepad(which);
            if( index )
                dd_log("\4Gamepad [%d] axis %s -> %d", index, SDL_GetGamepadStringForAxis((SDL_GamepadAxis) event->gaxis.axis), (int) event->gaxis.value);
        }
        break; case SDL_EVENT_JOYSTICK_BATTERY_UPDATED: {
            SDL_JoystickID which = event->jbattery.which;
            if (SDL_IsGamepad(which)) {  /* this is only reported for joysticks, so make sure this joystick is _actually_ a gamepad. */
                int index = player_findoradd_gamepad(which);
                if( index )
                    dd_log("\5Gamepad [%d] battery -> %s - %d%%", index, battery_state_string(event->jbattery.state), event->jbattery.percent);
            }
        }
        break; case SDL_EVENT_GAMEPAD_BUTTON_UP: case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
            SDL_JoystickID which = event->gbutton.which;
            int index = player_findoradd_gamepad(which);
            if( index )
                dd_log("\4Gamepad [%d] button %s -> %s", index, SDL_GetGamepadStringForButton((SDL_GamepadButton) event->gbutton.button), event->gbutton.down ? "PRESSED" : "RELEASED");
        }

    }
    return event;
}


#if 0

/* gamepad-polling.c ... */

/*
 * This example code looks for the current gamepad state once per frame,
 * and draws a visual representation of it. See 01-joystick-polling for the
 * equivalent example code for the lower-level joystick API.
 *
 * This code is public domain. Feel free to use it for any purpose!
 */

/* SDL can handle multiple gamepads, but for simplicity, this program only
   deals with the first gamepad it sees. */

#define window window2

/* We will use this renderer to draw into this window every frame. */
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static SDL_Texture *texture = NULL;
static SDL_Gamepad *gamepad = NULL;

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    char *png_path = NULL;
    SDL_Surface *surface = NULL;

    SDL_SetAppMetadata("Example Input Gamepad Polling", "1.0", "com.example.input-gamepad-polling");

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("examples/input/gamepad-polling", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_SetRenderLogicalPresentation(renderer, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_LOGICAL_PRESENTATION_STRETCH)) {
        return SDL_APP_FAILURE;
    }

    /* Textures are pixel data that we upload to the video hardware for fast drawing. Lots of 2D
       engines refer to these as "sprites." We'll do a static texture (upload once, draw many
       times) with data from a bitmap file. */

    /* SDL_Surface is pixel data the CPU can access. SDL_Texture is pixel data the GPU can access.
       Load a .png into a surface, move it to a texture from there. */
    SDL_asprintf(&png_path, "%sgamepad_front.png", SDL_GetBasePath());  /* allocate a string of the full file path */
    surface = SDL_LoadPNG(png_path);
    if (!surface) {
        SDL_Log("Couldn't load bitmap: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_free(png_path);  /* done with this, the file is loaded. */

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_Log("Couldn't create static texture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_DestroySurface(surface);  /* done with this, the texture has a copy of the pixels now. */

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    } else if (event->type == SDL_EVENT_GAMEPAD_ADDED) {
        /* this event is sent for each hotplugged gamepad, but also each already-connected gamepad during SDL_Init(). */
        if (gamepad == NULL) {  /* we don't have a stick yet and one was added, open it! */
            gamepad = SDL_OpenGamepad(event->gdevice.which);
            if (!gamepad) {
                SDL_Log("Failed to open gamepad ID %u: %s", (unsigned int) event->gdevice.which, SDL_GetError());
            }
        }
    } else if (event->type == SDL_EVENT_GAMEPAD_REMOVED) {
        if (gamepad && (SDL_GetGamepadID(gamepad) == event->gdevice.which)) {
            SDL_CloseGamepad(gamepad);  /* our controller was unplugged. */
            gamepad = NULL;
        }
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    const char *text = "Plug in a gamepad, please.";
    static uint64_t leftthumblast = 0xFFFFFFFF;
    static uint64_t rightthumblast = 0xFFFFFFFF;
    const uint64_t now = SDL_GetTicks();
    Sint16 axis_x, axis_y;
    float x, y;
    int i;

    if (gamepad) {  /* we have a stick opened? */
        text = SDL_GetGamepadName(gamepad);
    }

    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);  /* white */
    SDL_RenderClear(renderer);

    /* note that you can get input as events, instead of polling, which is
       better since it won't miss button presses if the system is lagging,
       but often times checking the current state per-frame is good enough,
       and maybe better if you'd rather _drop_ inputs due to lag. */

    if (gamepad) {  /* we have a stick opened? */
        /* where to draw the buttons */
        const SDL_FRect buttons[] = {
            { 497, 266, 38,  38 },   /* SDL_GAMEPAD_BUTTON_SOUTH */
            { 550, 217, 38,  38 },   /* SDL_GAMEPAD_BUTTON_EAST */
            { 445, 221, 38,  38 },   /* SDL_GAMEPAD_BUTTON_WEST */
            { 499, 173, 38,  38 },   /* SDL_GAMEPAD_BUTTON_NORTH */
            { 235, 228, 32,  29 },   /* SDL_GAMEPAD_BUTTON_BACK */
            { 287, 195, 69,  69 },   /* SDL_GAMEPAD_BUTTON_GUIDE */
            { 377, 228, 32,  29 },   /* SDL_GAMEPAD_BUTTON_START */
            { 91,  234, 63,  63 },   /* SDL_GAMEPAD_BUTTON_LEFT_STICK */
            { 381, 354, 63,  63 },   /* SDL_GAMEPAD_BUTTON_RIGHT_STICK */
            { 74,  73,  102, 29 },   /* SDL_GAMEPAD_BUTTON_LEFT_SHOULDER */
            { 468, 73,  102, 29 },   /* SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER */
            { 207, 316, 32,  32 },   /* SDL_GAMEPAD_BUTTON_DPAD_UP */
            { 207, 384, 32,  32 },   /* SDL_GAMEPAD_BUTTON_DPAD_DOWN */
            { 173, 351, 32,  32 },   /* SDL_GAMEPAD_BUTTON_DPAD_LEFT */
            { 242, 351, 32,  32 },   /* SDL_GAMEPAD_BUTTON_DPAD_RIGHT */
            { 310, 286, 23,  27 },   /* SDL_GAMEPAD_BUTTON_MISC1 */
            /* there are other buttons: paddles on the back of the gamepad, touchpads, etc, but this is good enough for now. */
        };

        SDL_RenderTexture(renderer, texture, NULL, NULL);  /* draw the gamepad picture to the whole window. */

        /* draw green boxes over buttons that are currently pressed. */
        SDL_SetRenderDrawColor(renderer, 0x00, 0xFF, 0x00, 0xFF);  /* green */
        for (int i = 0; i < SDL_arraysize(buttons); i++) {
            if (SDL_GetGamepadButton(gamepad, (SDL_GamepadButton) i)) {
                SDL_RenderFillRect(renderer, &buttons[i]);
            }
        }

        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0x00, 0xFF);  /* yellow */

        /* left thumb axis. */
        axis_x = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX);
        axis_y = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY);
        if ((SDL_abs(axis_x) > 1000) || (SDL_abs(axis_y) > 1000)) {   /* zero means centered, but it might be a little off zero... */
            leftthumblast = now;  /* keep drawing, we're still moving. */
        }
        if ((now - leftthumblast) < 500) {  /* draw if there was movement in the last half-second. */
            const SDL_FRect box = { 107 + ((axis_x / 32767.0f) * 30.0f), 252 + ((axis_y / 32767.0f) * 30.0f), 30, 30 };
            SDL_RenderFillRect(renderer, &box);
        }

        /* right thumb axis. */
        axis_x = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTX);
        axis_y = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTY);
        if ((SDL_abs(axis_x) > 1000) || (SDL_abs(axis_y) > 1000)) {   /* zero means centered, but it might be a little off zero... */
            rightthumblast = now;  /* keep drawing, we're still moving. */
        }
        if ((now - rightthumblast) < 500) {  /* draw if there was movement in the last half-second. */
            const SDL_FRect box = { 397 + ((axis_x / 32767.0f) * 30.0f), 370 + ((axis_y / 32767.0f) * 30.0f), 30, 30 };
            SDL_RenderFillRect(renderer, &box);
        }

        /* left trigger. */
        axis_y = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
        if (axis_y > 1000) {   /* zero means unpressed, but it might be a little off zero... */
            const float height = ((axis_y / 32767.0f) * 65.0f);
            const SDL_FRect box = { 127, 1 + (65.0f - height), 37, height };
            SDL_RenderFillRect(renderer, &box);
        }

        /* right trigger. */
        axis_y = SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);
        if (axis_y > 1000) {   /* zero means unpressed, but it might be a little off zero... */
            const float height = ((axis_y / 32767.0f) * 65.0f);
            const SDL_FRect box = { 481, 1 + (65.0f - height), 37, height };
            SDL_RenderFillRect(renderer, &box);
        }
    }

    x = (((float) WINDOW_WIDTH) - (SDL_strlen(text) * SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE)) / 2.0f;
    if (gamepad) {
        y = (float) (WINDOW_HEIGHT - (SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE + 2));
    } else {
        y = (((float) WINDOW_HEIGHT) - SDL_DEBUG_TEXT_FONT_CHARACTER_SIZE) / 2.0f;
    }
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0xFF, 0xFF);  /* blue */
    SDL_RenderDebugText(renderer, x, y, text);
    SDL_RenderPresent(renderer);

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    SDL_DestroyTexture(texture);
    SDL_CloseGamepad(gamepad);
    /* SDL will clean up the window/renderer for us. */
}

#endif

static const struct {
    const char *key;
    const char *value;
} extended_metadata[] = {
    { SDL_PROP_APP_METADATA_URL_STRING, "https://github.com/r-lyeh/kit" },
    { SDL_PROP_APP_METADATA_CREATOR_STRING, "Kitdevs" },
    { SDL_PROP_APP_METADATA_COPYRIGHT_STRING, "Placed in the public domain" },
    { SDL_PROP_APP_METADATA_TYPE_STRING, "game" }
};

void main(event ev) {
    if( ev.init ) {
        if (!SDL_SetAppMetadata("Kitdemo input", "1.0", "com.kitdemo.input")) {
            app.quit(-1);
        }

        for (int i = 0; i < SDL_arraysize(extended_metadata); i++) {
            if (!SDL_SetAppMetadataProperty(extended_metadata[i].key, extended_metadata[i].value)) {
                app.quit(-1);
            }
        }

        //if (!render.open(640/0.85, 480/0.85, 0.85, 0))
        if (!SDL_CreateWindowAndRenderer("kitdemo input", 640, 480, SDL_WINDOW_RESIZABLE, &window.handle, &render.handle))
            app.quit(-1);

        render.vsync(0);
        //    SDL_SetWindowRelativeMouseMode(window.handle, true);
        SDL_SetHintWithPriority(SDL_HINT_WINDOWS_RAW_KEYBOARD, "1", SDL_HINT_OVERRIDE);
    }

    if( ev.emit ) {
        SDL_Event *event = ev.emit;

        // !!!!!!!!!!!!!!!!!!!!
        kit_event(event);
        // !!!!!!!!!!!!!!!!!!!!

        switch (event->type) {
            case SDL_EVENT_QUIT:
                app.quit(0);
            case SDL_EVENT_KEY_DOWN:;
                SDL_Keycode sym = event->key.key;
                if(sym == SDLK_ESCAPE) app.quit(0);
        }
    }

    if( ev.tick ) {
        SDL_SetRenderDrawColor(render.handle, 0, 0, 0, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(render.handle);

        char *debug_string = va( 
            "\6%d\n"
            "%s\n"
            "%s\n"
            "%c%c\n",
            time.fps(),
            debug_player(1),
            debug_player(2),
            'a' | 32, 'A' | 32
        );
        dd_print(0, 0, 8, 1.0f, debug_string);
        dd_flush_log();

        SDL_RenderPresent(render.handle);        
    }
}
