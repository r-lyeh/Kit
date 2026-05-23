#include "kit.h"
const char *hints;





const char *HELP =
"B    Toggle bold\n"
"A    Toggle alignment\n"
"R    Toggle layout direction\n"
"I    Toggle italic\n"
"O    Toggle outline\n"
"U    Toggle underline\n"
"S    Toggle strike-through\n"
"H    Toggle hinting & pixel grid\n"
"UP   Decrease font size\n"
"DOWN Increase font size\n"
",    Decrease spacing\n"
".    Increase spacing\n"
;

// Mixed English, Chinese, Hebrew, and emoji text
#define DEFAULT_TEXT    "The quick brown fox\njumped over the "\
    "\xe5\xad\xa6\xe4\xb9\xa0\xe6\x9f\x90\xe8\xaf\xbe\xe7\xa8\x8b\xe5\xbf\x85\xe8\xaf\xbb\xe7\x9a\x84\n"\
    "\xd7\x90\xd7\x91\xd7\x92 "\
    "\xf0\x9f\x98\x89🥰💀✌️🌴🐢🐐🍄🍻👑📸😬"

#define MAX_FALLBACKS   4

#define WIDTH   640
#define HEIGHT  480

typedef struct {
    bool done;
    SDL_Window *window;
    SDL_Renderer *renderer;

    // dynamic
    TTF_Text *caption;
    SDL_Rect captionRect;

    // baked
    SDL_Texture *messageTex;
    SDL_FRect messageRect;
} Scene;

static void DrawScene(Scene *scene, unsigned rgba) {
    SDL_Renderer *renderer = scene->renderer;

    /* Clear the background to background color */
    SDL_SetRenderDrawColor(renderer, color.r(rgba), color.g(rgba), color.b(rgba), color.a(rgba));
    SDL_RenderClear(renderer);

    // update text (1)
    TTF_DrawRendererText(scene->caption, (float)scene->captionRect.x, (float)scene->captionRect.y);

    // blit baked texture (2)
    SDL_RenderTexture(renderer, scene->messageTex, NULL, &scene->messageRect);

    SDL_RenderPresent(renderer);
}

static void AdjustTextOffset(TTF_Text *text, int xoffset, int yoffset) {
    int x, y;

    TTF_GetTextPosition(text, &x, &y);
    x += xoffset;
    y += yoffset;
    TTF_SetTextPosition(text, x, y);
}

static void HandleKeyDown(Scene *scene, SDL_Event *event) {
    switch (event->key.key) {
    default:

    break; case SDLK_ESCAPE:
        scene->done = true;

    break; case SDLK_A:;
        /* A: Cycle alignment */
        int wrap = font_align(1, 0);
        /**/ if( wrap == 'l' ) font_align(1, "c");
        else if( wrap == 'c' ) font_align(1, "r");
        else if( wrap == 'r' ) font_align(1, "l");

    break; case SDLK_B:
        /* B: Toggle bold style */
        font_bold( 1, font_bold(1,-1) ^ 1);

    break; case SDLK_I:
        /* I: Toggle italic style */
        font_italic( 1, font_italic(1,-1) ^ 1);

    break; case SDLK_S:
        /* S: Toggle strike-through style */
        font_strike( 1, font_strike(1,-1) ^ 1);

    break; case SDLK_U:
        /* U: Toggle underline style */
        font_underline( 1, font_underline(1,-1) ^ 1);

    break; case SDLK_O:
        /* O: Toggle outline */
        font_outline( 1, font_outline(1,-1) ^ 1 );

    break; case SDLK_DOWN:;
        /* DOWN: Increase font size */
        font_size(1, font_size(1, -1) + 1.0f);

    break; case SDLK_UP:;
        /* UP: Decrease font size */
        font_size(1, font_size(1, -1) - 1.0f);

    break; case SDLK_H:;
        /* H: Toggle hinting & pixel grid */
        int hint = font_hinting(1, 0); printf("%c\n", hint);
        /**/ if (hint == '0') font_hinting(1, "n");
        else if (hint == 'n') font_hinting(1, "l");
        else if (hint == 'l') font_hinting(1, "m");
        else if (hint == 'm') //font_hinting(1, "s");
//        else if (hint == 's') 
            font_hinting(1, "0");

    break; case SDLK_R:;
        /* R: Toggle layout direction */
        int dir = font_direction(1, 0);
        /**/ if (dir == 'l') font_direction(1, "r");
        else if (dir == 'r') font_direction(1, "t");
        else if (dir == 't') font_direction(1, "b");
        else if (dir == 'b') font_direction(1, "l");

    break; case SDLK_COMMA:
        /* ,: Decrease layout spacing */
        font_spacing(1, font_spacing(1,0.5)-1);

    break; case SDLK_PERIOD:
        /* .: Increase layout spacing */
        font_spacing(1, font_spacing(1,0.5)+1);
    }
}

void main(event ev)
{
    Scene scene = {0};
    unsigned white = color.rgba(200,100,0,255);
    unsigned black = color.rgba(255,255,255,255);

    int argc = __argc;
    const char **argv = __argv;
    int i;
    for (i=1; argv[i] && argv[i][0] == '-'; ++i) {}
    argv += i;
    argc -= i;

    /* Open the font file with the requested point size */
    #define MY_FONT  "demos/res/mplus-1p-medium.ttf"     // "demos/art/fonts/NotoSansCJKjp-Regular.otf"
    #define MY_EMOJI "demos/art/fonts/EmojiOneColor.otf" // "demos/art/fonts/NotoColorEmoji-Regular.ttf"
    int myfont = font.open(atof(os.arg("--size=18.0")), os.arg("--font=" MY_FONT "," MY_EMOJI ",c:\\windows\\fonts\\arial.ttf"));

    /* Create a window */
    scene.window = SDL_CreateWindow("font", WIDTH, HEIGHT, 0);
    if (!scene.window) os.die(va("cannot create window %s", SDL_GetError()));
    
    scene.renderer = SDL_CreateRenderer(scene.window, NULL); // SDL_GPU_RENDERER);
    if (!scene.renderer) os.die("cannot create renderer");
    SDL_SetRenderVSync(scene.renderer, 1);

    TTF_TextEngine *engine = TTF_CreateRendererTextEngine(scene.renderer);
    if (!engine) os.die("cannot create ttf renderer");

    /* Show which font file we're looking at */
    scene.caption = TTF_CreateText(engine, font.handle(myfont), va("Font test\n%s\n%s", DEFAULT_TEXT, HELP), 0);
    TTF_SetTextColor(scene.caption, color.r(black),color.g(black),color.b(black),color.a(black));
    scene.captionRect.x = 4;
    scene.captionRect.y = 4;
    TTF_GetTextSize(scene.caption, &scene.captionRect.w, &scene.captionRect.h);

    /* Render and center the message */
    int wrap_width = 0;
    SDL_Surface *text = font.bake(myfont, atoi(os.arg("--quality=2")), black, white, wrap_width, DEFAULT_TEXT);
    if (!text) os.die(va("Couldn't bake font: %s", SDL_GetError()));
    scene.messageRect.x = (float)((WIDTH - text->w)/2);
    scene.messageRect.y = (float)((HEIGHT - text->h)/2);
    scene.messageRect.w = (float)text->w;
    scene.messageRect.h = (float)text->h;
    scene.messageTex = SDL_CreateTextureFromSurface(scene.renderer, text);
    SDL_DestroySurface(text);

    SDL_Log("Font is generally %d big, and string is %d big", TTF_GetFontHeight(font.handle(myfont)), text->h);

    /* Wait for a keystroke, and blit text on mouse press */
    while (!scene.done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            //SDL_ConvertEventToRenderCoordinates(scene.renderer, &event);

            switch (event.type) {
                case SDL_EVENT_QUIT:
                    scene.done = true;
                    break;

                case SDL_EVENT_KEY_DOWN:
                    HandleKeyDown(&scene, &event);

                default:
                    break;
            }
        }
        DrawScene(&scene, white);
    }

    const char *error = SDL_GetError();
    if(error && error[0]) SDL_Log("Error(s) found: %s", error);

    SDL_DestroyTexture(scene.messageTex);
    TTF_DestroyText(scene.caption);
    TTF_DestroyRendererTextEngine(engine);
    font.close(&myfont);
    TTF_Quit();
    SDL_Quit();

    app.quit(0);
}
