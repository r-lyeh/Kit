#include "kit.h"
const char *hints;

#if 0

// tty/console launcher
void main(event ev) {
    if( os.argc() > 1 && SDL_isdigit(os.argv(1)[0]) ) {
        __argv[1] = string.open("--main=%s", os.argv(1));
        app.reload();
    } else {
        for( int i = 0; i < array_count(mains)-1; ++i ) {
            printf("%s %2d\t/* %s */\n", os.argv(0), i, mains[i].name);
        }
    }
    app.quit(0);
}

#else

// gui launcher
void main(event ev) {

    if(ev.init) {
        // if( os.argc() > 1 && file.exists(os.argv(1)) ) return lua_init(), luaj_init(), lua_runfile(argv(1)), 0;
        if( os.argc() > 1 && SDL_isdigit(os.argv(1)[0]) ) {
            __argv[1] = string.open("--main=%s", os.argv(1));
            app.reload();
        }
        render.open(0,0,0.75,SDL_WINDOW_BORDERLESS);
        imgui.open();
    }
    if(ev.quit) {
        imgui.close();
    }
    if(ev.emit) {
        SDL_ConvertEventToRenderCoordinates(render.handle, ev.emit);
        SDL_Event *event = imgui.event(ev.emit);
        if( !event ) return; // if event consumed

        if( event->type == SDL_EVENT_QUIT)
            app.quit(0);
    }
    if(ev.tick) {

        render.clear(0);

        imgui.begin();

        ui2_tick();

        static int open = 1/*UI_OPEN*/ | 4/*UI_CENTER*/; // | 16 /*UI2_FULLSCREEN*/;
        if( ui.window("Kit.launcher", &open) ) {

            for( int i = 0; i < array_count(mains)-1; ++i) {
            int choice = ui.buttons(2, va("%s #%d", mains[i].name, i), "...");
            if( choice == 1 ) system(va("%s %d",os.argv(0),i));
            if( choice == 2 ) system(va("%s \"%s\"", ifdef(KIT_WINDOWS,"start \"\"",ifdef(KIT_MACOS,"open","xdg-open")), mains[i].path));
            }

            ui.window_end();
        }

        imgui.end();

        render.present();

        if(keyboard.down("escape") || !(open & 1/*UI_OPEN*/)) window.show(0), app.quit(0);
    }
}

#endif

#include "demos/00_entrypoints.c"   // Explains init/emit/tick/quit lifecycle and multiple mains
#include "demos/01_empty.c"         // Basic window + clear color
#include "demos/01_loop.c"          // Basic window + color fade
#include "demos/01_undecorated.c"   // Custom window layout
#include "demos/01_tray.c"          // System tray integration
#include "demos/01_triangle.c"      // Rendered geometry via mesh helper in a transparent window
#include "demos/01_vsync.c"         // Vsync modes, FPS measurement
#include "demos/02_list.c"          // Asset/VFS layer + archive.dir()
#include "demos/03_audio.c"         // 3D audio + speaker + listener
#include "demos/04_webcam.c"        // Texture from webcam feed
#include "demos/05_lua.c"           // Lua scripting + hot-reload. Minimal, one script file
#include "demos/06_ui.c"            // Showcases ui abstraction: thin imgui wrapper intro
#include "demos/06_imgui1.c"        // Raw Dear ImGui, demo window
#include "demos/06_imgui2.c"        // Dear ImGui with explicit SDL_Window/SDL_Renderer handles
#include "demos/07_input.c"         // Input API showcase: keyboard/mouse/gamepad/touch/bindings combined
#include "demos/08_font.c"          // TTF rendering, bold/italic/alignment/direction...
#include "demos/09_postfx.c"        // GPU post-effects (VHS, vignette, FXAA)
#include "demos/10_sprite.c"        // Combined Sprites+Audio+Mouse+Lighting demo
#include "demos/11_bvh.c"           // Morton BVH collision detection, 10000 boxes. Algorithm-heavy
#include "demos/11_dd.c"            // 3D debug draw, orbit camera, full 3D scene. Most complex
