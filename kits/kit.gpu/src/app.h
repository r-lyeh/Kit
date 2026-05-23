#pragma once
/*
 * app.h  –  Central application context
 */

#include <SDL3/SDL.h>
#include "ibl.h"
#include "scalability.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct Camera   Camera;
typedef struct Scene    Scene;

#define APP_TITLE        "Kit"
#define APP_WIDTH        1280
#define APP_HEIGHT       720
#define MAX_FRAMES_IN_FLIGHT 2

typedef struct AppState {
    SDL_Window          *window;
    SDL_GPUDevice       *gpu;
    SDL_GPUCommandBuffer *cmd_bufs[MAX_FRAMES_IN_FLIGHT];
    uint32_t              frame_index;
    SDL_GPUTexture       *depth_texture;
    uint32_t              depth_w, depth_h;
    Camera   *camera;
    Scene    *scene;
    IBL          ibl;
    Scalability  sc;
    uint64_t  last_ticks;
    float     delta_time;
    /* Offscreen render target (GPU renders here, SDL_Renderer composites it) */
    SDL_GPUTexture  *render_target;     /* RGBA8 offscreen colour buffer */
    uint32_t         rt_w, rt_h;

    /* SDL_Renderer (gpu driver, shared device) – primary compositor and
     * presentation owner. Wraps the GPU render target as an SDL_Texture
     * so 2D layers can be drawn on top before SDL_RenderPresent().     */
    SDL_Renderer    *renderer_2d;
    SDL_Texture     *rt_as_sdl_tex;  /* render_target wrapped for SDL_Renderer */

    bool      running;
    bool      resized;
} AppState;

bool app_init(AppState *app);
void app_handle_events(AppState *app);
void app_update(AppState *app);
void app_render(AppState *app);
void app_destroy(AppState *app);
