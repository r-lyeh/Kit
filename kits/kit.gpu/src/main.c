/*
 * main.c  –  Entry point
 *
 * Keeps main() as thin as possible; all real logic lives in app.c.
 */

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>   /* Required so SDL can redefine main on Windows */
#include <stdio.h>

#include "app.h"

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    AppState app = {0};

    if (!app_init(&app)) {
        SDL_Log("Fatal: app_init failed");
        return 1;
    }

    /* ── Main loop ─────────────────────────────────────────────────────── */
    while (app.running) {
        app_handle_events(&app);
        app_update(&app);
        app_render(&app);
    }

    app_destroy(&app);
    return 0;
}
