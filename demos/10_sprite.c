// WIP! WIP! WIP! WIP! come back later

#include "kit.h"
#include "_mesh.h"
#include "_sprite.h"

void dd_text(float x, float y, const char *col, float sc, const char *t) {
    unsigned rgba = color.hex(col);
    SDL_SetRenderDrawColor(render.handle, color.r(rgba), color.g(rgba), color.b(rgba), 255);
    sc += !sc;
    float bakx, baky;
    SDL_GetRenderScale(render.handle, &bakx, &baky);
    SDL_SetRenderScale(render.handle, sc, sc);
    SDL_RenderDebugText(render.handle, x/sc, y/sc, t);
    SDL_SetRenderScale(render.handle, bakx, baky);
}

// quake style lighting
// format:
//   [SPEED] + [A: lights off (0%), M: normal intensity (100%), Z: double intensity (200%)] ...
// examples:
// style0 = "M"; // Normal steady light.
// style1 = "10MMNMMOMMOMMNONMMONQNMMO"; // Flicker (common for fluorescent lights).
// style2 = "10ABCDEFGHIJKLMNOPQRSTUVWXYZYXWVUTSRQPONMLKJIHGFEDCBA"; // Slow strong pulse.
// style3 = "100MMMMMAAAAAMMMMMAAAAAABCDEFGABCDEFG"; // Candle-like flicker.
// style4 = "10MAMAMAMAMAMA"; // Fast strobe.
// style5 = "10JKLMNOPQRSTUVWXYZYXWVUTSRQPONMLKJ"; // gentle pulse
float luma(const char *light) {
    const char *text = light + strspn(light, "0123456789");
    int index = (int)(elapsed.ss() * (atoi(light) + !atoi(light))) % strlen(text);
    return (text[index] - 'A') / (float)('M'-'A');
}


void main(event ev) {
    static unsigned tex = 0;
    static unsigned mou = 0;
    static unsigned spk = 0;
    static unsigned bgm = 0, mus = 0;
    enum { WINDOW_WIDTH = 1456, WINDOW_HEIGHT = 816 };

    if( ev.init ) {
        SDL_SetAppMetadata("Example Renderer Geometry", "1.0", "com.example.renderer-geometry");

        if( !render.open(WINDOW_WIDTH, WINDOW_HEIGHT, 0.85, 0) )
            app.quit(-1);

        window.title("audio+sprite+mouse+text");

        tex = texture.open("demos/art/Rusted Music Studio - Assets/Rusted (27).png");
        mou = texture.open("demos/art/Cursor_1.png");

        bgm = audio.open("demos/art/spacelifeNo14.ogg");
        mus = audio.open("demos/art/Dreaming-of-cyberpunk.ogg");
        spk = speaker.loop(speaker.open(NULL),1);
        speaker.play(spk, bgm);
    }
    if( ev.quit ) {
        texture.close(&tex);
    }
    if( ev.emit ) {
        if( ev.emit->type == SDL_EVENT_QUIT ) {
            if( dialog.prompt("Exit Game","Are you sure that you want to quit?", 2, "No", "Yes") == 2 )
                app.quit(0);  /* end the program, reporting success to the OS. */
        }
        if( ev.emit->type == SDL_EVENT_KEY_DOWN ) {
            if( keyboard.down("F11") || (keyboard.down("enter") && keyboard.held("Alt")) ) window_fullscreen( window_fullscreen(-1) ^ 1 ); // on()
            printf("%d %d\n", ev.emit->key.key, ev.emit->key.scancode);
        }
        if( ev.emit->type == SDL_EVENT_MOUSE_WHEEL ) {
            static float wheel_x, wheel_y;
            wheel_x = (float)ev.emit->wheel.x;
            wheel_y = (float)ev.emit->wheel.y;
        }
        if( ev.emit->type == SDL_EVENT_TEXT_INPUT ) {
            static array_(char) str = 0; array_resize(str, 0);
            array_concat(str, ev.emit->text.text);
        }
    }
    if( ev.tick ) {
        /* we'll have the triangle grow and shrink over a few seconds. */
        const uint64_t now = elapsed.ms();
        const float direction = ((now % 2000) >= 1000) ? 1.0f : -1.0f;
        const float scale = ((float) (((int) (now % 1000)) - 500) / 500.0f) * direction;
        const float size = 200.0f + (200.0f * scale);

        /* start with a dark blue canvas. */
        render.clear(color.hex("#001"));

        static float prev = 0;
        float lit = luma("10MMMMMMMMKKLLLL");
        lit = prev = lit * 0.05 + prev * 0.95; 
        float4 tint = {lit,lit,lit,1};

        sprite(tex, float2(0,0), float2(WINDOW_WIDTH, WINDOW_HEIGHT), float2(0,0), tint, 1);

        static int counter = 0; counter = elapsed.ss();
        if(0) sprite_blit(mou, float2(WINDOW_WIDTH/2,WINDOW_HEIGHT/2), float2(16,16), float2(0,0), tint, elapsed.ss(), tile_uv(counter,224,16,16));

        mouse_cursor(2);

        static array_(float4) rects = 0; // x,y,w,h
        static array_(const char*) name = 0;
        static array_(const char*) script = 0;
        static array_(char*)       tags = 0;
        static array_(int)         seen = 0;
        static array_(int)         used = 0;
        ONCE {
            array_push(rects, float4(1105,497,1149,555));
            array_push(name, "Lamp|This is my beloved lamp"); // if(seen==0) say(Lamp) else say(This is my beloved lamp)
            array_push(script, 0);
            array_push(seen, 0);
            array_push(used, 0);

            array_push(rects, float4(920,510,1018,539));
            array_push(name, "Hi-Fi"); // info = used ? Oops, the play button is broken now... : Hi-Fi; if(used==1) say(Oh Yeah!), play(bgm)  
            array_push(script, "My vintage Hi-Fi... with a play button|Oh yeah!|Oops, the play button is broken now...");
            array_push(seen, 0);
            array_push(used, 0);
        }
        if( mouse.down("r") ) { // @fixme
            // dd_text(0, 0, "#ff0", 4, va("M:%d,%d,%d,LIT:%f", mouse.get("x"), mouse.get("y"), mouse.get("l"), lit));
            float4 r = float4(mouse.get("x")-8, mouse.get("y")-8, mouse.get("x")+8, mouse.get("y")+8);
            //array_push(rects, r);
            //array_push(name, "?");
        }
        SDL_SetRenderDrawColor(render.handle, 255, 255, 255, 5);
        int item = -1;
        int mx = mouse.get("x"), my = mouse.get("y"), lmb = mouse.get("l");
        for each_array(rects, i) {
            if( mx > rects[i].x && mx < rects[i].z )
            if( my > rects[i].y && my < rects[i].w ) {
                item = i;
            }
            if(1) continue;
            SDL_FRect r = { rects[i].x, rects[i].y, rects[i].z-rects[i].x, rects[i].w-rects[i].y };
            SDL_RenderFillRect(render.handle, &r);
        }

        // @todo: LMB -> walk to object or location
        // @todo: LMB while close to object -> examine
        // @todo: RMB while close to object -> use

        static int lmb_old = 0; 
        int down = (lmb_old < lmb);
        lmb_old = lmb;

        if( item >= 0 ) {
            {
                const char *look = name[item];
                int times = seen[item];
                int counter = 0;
                for each_string(s, look, "|") {
                    if( counter++ == times )
                    dd_text(
                        (WINDOW_WIDTH - strlen(s)*8*4)/2, WINDOW_HEIGHT - 8*4, "#ff0",
                        4, va("%d %s", times, s));
                }
            }
            if( script[item] ) {
                int times = used[item];

                int counter = 0;
                for each_string(s, script[item], "|") {
                    if( counter++ == times )
                    dd_text(
                        (WINDOW_WIDTH - strlen(s)*8*4)/2, 8, "#ff0",
                        4, va("%d %s", times, s));
                }

                if( down ) { 
                    int cnt = strcnt(script[item], '|');
                    used[item] = used[item] + 1;
                    if( used[item] > cnt ) used[item] = cnt;

                    if( used[item] == 1 ) speaker.play(spk, mus);
                }
            }
        }

        render.present();
    }
}

const char *hints;
