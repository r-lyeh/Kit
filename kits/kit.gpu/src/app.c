/*
 * app.c  –  Application lifecycle
 */

#include "app.h"
#include "camera.h"
#include "gltf_loader.h"
#include "ibl.h"
#include "scalability.h"
#include "mesh.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Create the offscreen RGBA8 colour render target */
static SDL_GPUTexture *create_render_target(SDL_GPUDevice *gpu, uint32_t w, uint32_t h)
{
    SDL_GPUTextureCreateInfo ci = {
        .type                 = SDL_GPU_TEXTURETYPE_2D,
        .format               = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
        .usage                = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET
                              | SDL_GPU_TEXTUREUSAGE_SAMPLER,
        .width                = w,
        .height               = h,
        .layer_count_or_depth = 1,
        .num_levels           = 1,
        .sample_count         = SDL_GPU_SAMPLECOUNT_1,
    };
    SDL_GPUTexture *t = SDL_CreateGPUTexture(gpu, &ci);
    if (!t) SDL_Log("create_render_target: %s", SDL_GetError());
    return t;
}

static SDL_GPUTexture *create_depth_texture(AppState *app, uint32_t w, uint32_t h)
{
    SDL_GPUTextureCreateInfo info = {
        .type                 = SDL_GPU_TEXTURETYPE_2D,
        .format               = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
        .usage                = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
        .width                = w,
        .height               = h,
        .layer_count_or_depth = 1,
        .num_levels           = 1,
        .sample_count         = SDL_GPU_SAMPLECOUNT_1,
    };
    SDL_GPUTexture *tex = SDL_CreateGPUTexture(app->gpu, &info);
    if (!tex) SDL_Log("create_depth_texture: %s", SDL_GetError());
    return tex;
}

bool app_init(AppState *app)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init: %s", SDL_GetError());
        return false;
    }

    /* Scalability – load from file next to the executable, or use defaults.
     * SDL_GetBasePath() returns the directory of the executable, which is
     * reliable regardless of the working directory (e.g. Visual Studio).   */
    sc_defaults(&app->sc);
    {
        const char *base  = SDL_GetBasePath();
        char        ini_path[512];
        SDL_snprintf(ini_path, sizeof(ini_path), "%sscalability.ini", base ? base : "");
        if (!sc_load(&app->sc, ini_path)) {
            SDL_Log("scalability: no ini found at '%s', writing defaults", ini_path);
            sc_save(&app->sc, ini_path);
        }
    }
    sc_print(&app->sc);

    app->window = SDL_CreateWindow(APP_TITLE, APP_WIDTH, APP_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!app->window) { SDL_Log("SDL_CreateWindow: %s", SDL_GetError()); return false; }

    /* Request SPIRV only – our shaders are compiled to SPIR-V.
     * Requesting DXIL|MSL as well would let SDL pick D3D12/Metal which
     * would then reject the .spv files with "Incompatible shader format". */
    app->gpu = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);
    if (!app->gpu) { SDL_Log("SDL_CreateGPUDevice: %s", SDL_GetError()); return false; }

    SDL_Log("GPU backend: %s", SDL_GetGPUDeviceDriver(app->gpu));

    if (!SDL_ClaimWindowForGPUDevice(app->gpu, app->window)) {
        SDL_Log("SDL_ClaimWindowForGPUDevice: %s", SDL_GetError()); return false;
    }

    /* Offscreen render target */
    app->rt_w         = APP_WIDTH;
    app->rt_h         = APP_HEIGHT;
    app->render_target = create_render_target(app->gpu, app->rt_w, app->rt_h);
    if (!app->render_target) return false;

    /* SDL_Renderer – primary compositor and presentation owner.
     *
     * We create it with the "gpu" driver and pass our existing SDL_GPUDevice
     * via SDL_PROP_RENDERER_CREATE_GPU_DEVICE_POINTER. This means both
     * SDL_GPU and SDL_Renderer share the same Vulkan device and command queue,
     * with zero cross-device synchronisation overhead.
     *
     * SDL_Renderer owns the window swapchain and calls SDL_RenderPresent().
     * SDL_GPU renders the 3D scene to an offscreen texture (render_target),
     * which is then wrapped as an SDL_Texture and composited by SDL_Renderer
     * as the base layer before any 2D sprites/HUD are drawn on top.          */
    {
        SDL_PropertiesID rprops = SDL_CreateProperties();
        SDL_SetStringProperty (rprops, SDL_PROP_RENDERER_CREATE_NAME_STRING,
                               "gpu");  /* must share device → must use gpu driver */
        SDL_SetPointerProperty (rprops, SDL_PROP_RENDERER_CREATE_WINDOW_POINTER,
                                app->window);
        SDL_SetPointerProperty (rprops, SDL_PROP_RENDERER_CREATE_GPU_DEVICE_POINTER,
                                app->gpu);
        app->renderer_2d = SDL_CreateRendererWithProperties(rprops);
        SDL_DestroyProperties(rprops);
        if (!app->renderer_2d) {
            SDL_Log("SDL_Renderer (gpu): %s", SDL_GetError());
            return false;
        }
        SDL_Log("2D renderer: %s", SDL_GetRendererName(app->renderer_2d));
    }

    /* Wrap the offscreen GPU render target as an SDL_Texture.
     * SDL_PROP_TEXTURE_CREATE_GPU_TEXTURE_POINTER lets SDL_Renderer
     * reference an existing SDL_GPUTexture directly – no copy, no readback.
     * Create this once; reuse every frame via SDL_RenderTexture().          */
    {
        SDL_PropertiesID tprops = SDL_CreateProperties();
        SDL_SetPointerProperty(tprops, SDL_PROP_TEXTURE_CREATE_GPU_TEXTURE_POINTER,
                               app->render_target);
        /* ABGR8888 = R8G8B8A8_UNORM in little-endian memory (Windows/Linux x64).
         * TARGET access tells SDL_Renderer this texture is written externally.  */
        SDL_SetNumberProperty (tprops, SDL_PROP_TEXTURE_CREATE_FORMAT_NUMBER,
                               SDL_PIXELFORMAT_ABGR8888);
        SDL_SetNumberProperty (tprops, SDL_PROP_TEXTURE_CREATE_ACCESS_NUMBER,
                               SDL_TEXTUREACCESS_TARGET);
        SDL_SetNumberProperty (tprops, SDL_PROP_TEXTURE_CREATE_WIDTH_NUMBER,
                               app->rt_w);
        SDL_SetNumberProperty (tprops, SDL_PROP_TEXTURE_CREATE_HEIGHT_NUMBER,
                               app->rt_h);
        app->rt_as_sdl_tex = SDL_CreateTextureWithProperties(app->renderer_2d, tprops);
        SDL_DestroyProperties(tprops);
        if (!app->rt_as_sdl_tex) {
            SDL_Log("SDL_CreateTextureWithProperties (gpu tex wrap): %s",
                    SDL_GetError());
            return false;
        }
        /* Opaque blit – ignore alpha channel in the GPU render target.
         * Without this SDL_Renderer alpha-blends the texture onto the
         * cleared background, making dark/transparent areas show through. */
        SDL_SetTextureBlendMode(app->rt_as_sdl_tex, SDL_BLENDMODE_NONE);
    }

    app->depth_w = APP_WIDTH;
    app->depth_h = APP_HEIGHT;
    app->depth_texture = create_depth_texture(app, app->depth_w, app->depth_h);
    if (!app->depth_texture) return false;

    app->camera = camera_create();
    camera_set_perspective(app->camera, SDL_PI_F / 3.0f,
        (float)APP_WIDTH / (float)APP_HEIGHT, 0.1f, 1000.0f);
    camera_look_at(app->camera,
        (float[3]){0.f, 1.5f, 4.f},
        (float[3]){0.f, 0.f,  0.f},
        (float[3]){0.f, 1.f,  0.f});

    /* IBL – always initialise so Vulkan has valid sampler bindings.
     * When sc_render_ibl=0 the shader uses the hemisphere fallback path
     * and the IBL textures are grey dummies (no expensive convolution). */
    if (sc_flag_ibl(&app->sc)) {
        ibl_load(&app->ibl, app->gpu, "assets/env.hdr",
                 (uint32_t)sc_ibl_samples(&app->sc));
    } else {
        SDL_Log("scalability: IBL disabled – creating grey fallback textures");
        ibl_load(&app->ibl, app->gpu, NULL, 64);  /* grey env, minimal samples */
    }

    app->scene = scene_create(app->gpu, app->window);
    /* Accept a glTF path as a command-line argument or drag-and-drop.
     * __argc/__argv are MSVC globals; fall back to a default path.    */
#if defined(_MSC_VER)
    const char *gltf_path = (__argc > 1) ? __argv[1] : "assets/Box.glb";
#else
    const char *gltf_path = "assets/Box.glb";
#endif
    if (!gltf_load_into_scene(app->gpu, gltf_path, app->scene)) {
        SDL_Log("Could not load '%s' – rendering placeholder triangle", gltf_path);
        scene_add_placeholder(app->gpu, app->scene);
    }

    app->last_ticks = SDL_GetTicks();
    app->running    = true;
    return true;
}

void app_handle_events(AppState *app)
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
        case SDL_EVENT_QUIT:                          app->running = false; break;
        case SDL_EVENT_KEY_DOWN:
            if (e.key.key == SDLK_ESCAPE) app->running = false;
            /* Press N to toggle normal map green-channel flip (GL vs DX baked maps) */
            if (e.key.key == SDLK_N) {
                for (uint32_t i = 0; i < app->scene->mesh_count; i++) {
                    float *sign = &app->scene->meshes[i]->material.normal_y_sign;
                    *sign = (*sign > 0.f) ? -1.f : 1.f;
                }
                SDL_Log("Normal Y sign toggled to %.0f",
                        app->scene->mesh_count > 0
                        ? app->scene->meshes[0]->material.normal_y_sign : 0.f);
            }
            break;
        case SDL_EVENT_WINDOW_RESIZED:                app->resized = true;  break;
        default: break;
        }
        camera_handle_event(app->camera, &e);
    }
}

void app_update(AppState *app)
{
    uint64_t now    = SDL_GetTicks();
    app->delta_time = (float)(now - app->last_ticks) / 1000.0f;
    app->last_ticks = now;

    if (app->resized) {
        int w, h;
        SDL_GetWindowSize(app->window, &w, &h);
        if ((uint32_t)w != app->depth_w || (uint32_t)h != app->depth_h) {
            /* Recreate offscreen RT and its SDL_Texture wrapper */
            SDL_DestroyTexture(app->rt_as_sdl_tex);
            SDL_ReleaseGPUTexture(app->gpu, app->render_target);
            app->rt_w = (uint32_t)w; app->rt_h = (uint32_t)h;
            app->render_target = create_render_target(app->gpu, app->rt_w, app->rt_h);
            {
                SDL_PropertiesID tp = SDL_CreateProperties();
                SDL_SetPointerProperty(tp, SDL_PROP_TEXTURE_CREATE_GPU_TEXTURE_POINTER,
                                       app->render_target);
                SDL_SetNumberProperty(tp, SDL_PROP_TEXTURE_CREATE_FORMAT_NUMBER,
                                      SDL_PIXELFORMAT_ABGR8888);
                SDL_SetNumberProperty(tp, SDL_PROP_TEXTURE_CREATE_ACCESS_NUMBER,
                                      SDL_TEXTUREACCESS_TARGET);
                SDL_SetNumberProperty(tp, SDL_PROP_TEXTURE_CREATE_WIDTH_NUMBER,  app->rt_w);
                SDL_SetNumberProperty(tp, SDL_PROP_TEXTURE_CREATE_HEIGHT_NUMBER, app->rt_h);
                app->rt_as_sdl_tex = SDL_CreateTextureWithProperties(app->renderer_2d, tp);
                SDL_DestroyProperties(tp);
            }
            SDL_ReleaseGPUTexture(app->gpu, app->depth_texture);
            app->depth_w = (uint32_t)w;
            app->depth_h = (uint32_t)h;
            app->depth_texture = create_depth_texture(app, app->depth_w, app->depth_h);
            camera_set_aspect(app->camera, (float)w / (float)h);
        }
        app->resized = false;
    }

    camera_update(app->camera, app->delta_time);
}

void app_render(AppState *app)
{
    SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(app->gpu);
    if (!cmd) return;

    /* Depth buffer for the GPU render pass */
    SDL_GPUDepthStencilTargetInfo depth_target = {
        .texture     = app->depth_texture,
        .load_op     = SDL_GPU_LOADOP_CLEAR,
        .store_op    = SDL_GPU_STOREOP_DONT_CARE,
        .clear_depth = 1.0f,
        .cycle       = false,
    };

    /* ── GPU render pass: render 3D scene to offscreen texture ──────────────
     * Colour target is our render_target (not the swapchain).
     * This lets SDL_Renderer composite 2D layers on top afterwards.        */
    SDL_GPUColorTargetInfo rt_target = {
        .texture     = app->render_target,
        .load_op     = SDL_GPU_LOADOP_CLEAR,
        .store_op    = SDL_GPU_STOREOP_STORE,
        .clear_color = {0.07f, 0.07f, 0.10f, 1.0f},
        .cycle       = false,
    };

    SDL_GPURenderPass *pass = SDL_BeginGPURenderPass(cmd, &rt_target, 1, &depth_target);

    if (app->ibl.irradiance && app->ibl.sampler_cube) {
        SDL_GPUTextureSamplerBinding ibl_bind[3] = {
            { app->ibl.irradiance, app->ibl.sampler_cube },
            { app->ibl.prefilter,  app->ibl.sampler_cube },
            { app->ibl.brdf_lut,   app->ibl.sampler_lut  },
        };
        SDL_BindGPUFragmentSamplers(pass, 5, ibl_bind, 3);
    }

    float vp[16];
    camera_get_viewproj(app->camera, vp);
    SDL_PushGPUVertexUniformData(cmd, 0, vp, sizeof(vp));

    float eye4[4] = {0};
    camera_get_eye(app->camera, eye4);
    SDL_PushGPUFragmentUniformData(cmd, 1, eye4, sizeof(eye4));

    float sc_ubo[4] = { sc_flag_ibl(&app->sc) ? 1.0f : 0.0f, 0.f, 0.f, 0.f };
    SDL_PushGPUFragmentUniformData(cmd, 2, sc_ubo, sizeof(sc_ubo));

    scene_draw_opaque(app->scene, pass, cmd);
    scene_draw_blend(app->scene, pass, cmd, eye4);

    SDL_EndGPURenderPass(pass);
    SDL_SubmitGPUCommandBuffer(cmd);

    /* ── SDL_Renderer compositor pass ───────────────────────────────────────
     * SDL_Renderer owns the window swapchain and final presentation.
     *
     * rt_as_sdl_tex wraps the GPU render target (created once at init via
     * SDL_PROP_TEXTURE_CREATE_GPU_TEXTURE_POINTER) so SDL_Renderer can blit
     * it as the base 3D layer. 2D sprites/HUD/UI are drawn on top before
     * SDL_RenderPresent() flips to screen.
     *
     * No GPU→CPU readback occurs. Both SDL_GPU and SDL_Renderer share the
     * same Vulkan device so the texture reference is a zero-copy operation. */
    /* Clear first (in case the window has stale content), then blit 3D. */
    SDL_SetRenderDrawColor(app->renderer_2d, 0, 0, 0, 255);
    SDL_RenderClear(app->renderer_2d);
    SDL_RenderTexture(app->renderer_2d, app->rt_as_sdl_tex, NULL, NULL);

    /* ── INSERT 2D LAYERS HERE ───────────────────────────────────────────────
     * Example: SDL_RenderTexture(app->renderer_2d, hud_tex, NULL, &hud_rect);
     *          SDL_RenderTexture(app->renderer_2d, sprite, &src, &dst);       */

    static double t = 0, counter = 0, fps = 0; double now = SDL_GetTicks();
    if( (now-t) >= 1000 ) {
        fps = counter / ((now-t) / 1000.0);
        counter = 0;
        t = now;
        printf("%5.2ffps\n", fps);
    } else {
        counter++;
    }
    char buf[64]; snprintf(buf, 64, "%5.2ffps", fps);
    SDL_SetRenderDrawColor(app->renderer_2d, 255, 255, 255, 255);
    SDL_RenderDebugText(app->renderer_2d, 0, 0, buf);

    SDL_RenderPresent(app->renderer_2d);

    app->frame_index = (app->frame_index + 1) % MAX_FRAMES_IN_FLIGHT;
}

void app_destroy(AppState *app)
{
    SDL_WaitForGPUIdle(app->gpu);
    scene_destroy(app->gpu, app->scene);
    ibl_destroy(&app->ibl, app->gpu);
    camera_destroy(app->camera);
    if (app->rt_as_sdl_tex)  SDL_DestroyTexture(app->rt_as_sdl_tex);
    SDL_ReleaseGPUTexture(app->gpu, app->render_target);
    SDL_ReleaseGPUTexture(app->gpu, app->depth_texture);
    if (app->renderer_2d) SDL_DestroyRenderer(app->renderer_2d);
    SDL_ReleaseWindowFromGPUDevice(app->gpu, app->window);
    SDL_DestroyGPUDevice(app->gpu);
    SDL_DestroyWindow(app->window);
    SDL_Quit();
}
