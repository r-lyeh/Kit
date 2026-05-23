// Demonstrates enabling/disabling renderer vsync correctly.

#include "kit.h"
const char *hints;


void main(event ev)
{
    static int vsync = -999;
    static bool running = true;
    static uint64_t lastCounter;
    static uint64_t frequency;
    static int frames = 0;
    static double timer_ = 0.0;
    static float x = 0.0f, speed = 800.0f;

    if(ev.init) {

        if(!render.open(1280,720,0.85,0))
            app.quit(-1);

        SDL_Log("Renderer backend: %s", SDL_GetRendererName(render.handle));

        // ------------------------------------------------------------
        // DISABLE VSYNC
        // ------------------------------------------------------------
        if (!SDL_SetRenderVSync(render.handle, 0)) {
            SDL_Log("SDL_SetRenderVSync failed: %s", SDL_GetError());
        }

        if (SDL_GetRenderVSync(render.handle, &vsync)) {
            SDL_Log("Current vsync setting: %d", vsync);
        } else {
            SDL_Log("SDL_GetRenderVSync failed: %s", SDL_GetError());
        }

        lastCounter = SDL_GetPerformanceCounter();
        frequency   = SDL_GetPerformanceFrequency();
    }

    // --------------------------------------------------------
    // EVENTS
    // --------------------------------------------------------
    if(ev.emit) {
        SDL_Event *event = ev.emit;

        if (event->type == SDL_EVENT_QUIT) {
            app.quit(0);
        }

        if (event->type == SDL_EVENT_KEY_DOWN) {

            // ESC quits
            if (event->key.key == SDLK_ESCAPE) {
                app.quit(0);
            }

            // V toggles vsync
            if (event->key.key == SDLK_V) {

                int current = 0;

                if (SDL_GetRenderVSync(render.handle, &current)) {

                    int newValue = current ? 0 : 1;

                    if (!SDL_SetRenderVSync(render.handle, newValue)) {
                        SDL_Log("Failed to set vsync: %s", SDL_GetError());
                    } else {
                        SDL_Log("VSync changed to: %d", newValue);
                    }
                }
            }

            // F toggles fullscreen
            if (event->key.key == SDLK_F) {

                SDL_WindowFlags flags = SDL_GetWindowFlags(window.handle);

                bool fullscreen =
                    (flags & SDL_WINDOW_FULLSCREEN) != 0;

                SDL_SetWindowFullscreen(window.handle, !fullscreen);

                SDL_Log(
                    "Fullscreen: %s",
                    fullscreen ? "OFF" : "ON"
                );
            }
        }
    }

    // --------------------------------------------------------
    // TIMING
    // --------------------------------------------------------
    if(ev.tick) {

        uint64_t now = SDL_GetPerformanceCounter();

        double dt =
            (double)(now - lastCounter) / (double)frequency;

        lastCounter = now;

        timer_ += dt;
        frames++;

        // print FPS once per second
        if (timer_ >= 1.0) {

            double fps = (double)frames / timer_;

            SDL_Log("FPS: %.2f", fps);

            timer_ = 0.0;
            frames = 0;
        }

        // --------------------------------------------------------
        // SIMPLE ANIMATION
        // --------------------------------------------------------
        x += speed * (float)dt;

        if (x > 1280.0f) {
            x = -100.0f;
        }

        // --------------------------------------------------------
        // RENDER
        // --------------------------------------------------------
        SDL_SetRenderDrawColor(render.handle, 20, 20, 20, 255);
        SDL_RenderClear(render.handle);

        SDL_FRect rect = {
            x,
            300.0f,
            100.0f,
            100.0f
        };

        SDL_SetRenderDrawColor(render.handle, 255, 80, 80, 255);
        SDL_RenderFillRect(render.handle, &rect);

        SDL_RenderPresent(render.handle);
    }
}
