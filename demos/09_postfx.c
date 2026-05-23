// kit.fx demo
// Build: cl kit.fx\demo.c
//
// Assumes shaders were pre-compiled beforehand:
//   glslc -fshader-stage=frag vhs.glsl      -o vhs.frag.spv
//   glslc -fshader-stage=frag vignette.glsl -o vignette.frag.spv
//   glslc -fshader-stage=frag fxaa.glsl     -o fxaa.frag.spv

#include "kit.h"

// ---------------------------------------------------------------------------
// Helpers: draw a placeholder scene into an SDL_GPUTexture

static void render_scene(SDL_GPUTexture *rt) {
    SDL_GPUCommandBuffer *cmdbuf = SDL_AcquireGPUCommandBuffer(fx.handle);
    SDL_GPUColorTargetInfo ct = {
        .texture     = rt,
        .load_op     = SDL_GPU_LOADOP_CLEAR,
        .store_op    = SDL_GPU_STOREOP_STORE,
        .clear_color = { .r=0.10f, .g=0.30f, .b=0.50f, .a=1.0f },
    };
    SDL_GPURenderPass *rp = SDL_BeginGPURenderPass(cmdbuf, &ct, 1, NULL);
    // ... replace with your real scene draw calls ...
    SDL_EndGPURenderPass(rp);
    SDL_SubmitGPUCommandBuffer(cmdbuf);
}

// ---------------------------------------------------------------------------

void main(event ev) {
    static int   fxaa_pass, vhs_pass, vignette_pass;
    static float elapsed;

    // --- init
    if (ev.init) {
        if (!render.open(1280, 720, 0.85f, 0))
            app.quit(-1);
        window.title("kit.fx demo");

        imgui.open();

        fx.open();

        fxaa_pass     = fx.add("demos/art/fx/fxaa.frag.spv",     NULL);
        vhs_pass      = fx.add("demos/art/fx/vhs.frag.spv",      NULL);
        vignette_pass = fx.add("demos/art/fx/vignette.frag.spv", NULL);

        if (fxaa_pass < 0 || vhs_pass < 0 || vignette_pass < 0)
            app.quit(-1);

        // initial uniforms
        // vhs:      u[0] = { time, noise_str, jitter, color_bleed }
        // vignette: u[0] = { radius, softness, strength, _ }
        // fxaa:     u[0] = { 1/w, 1/h, _, _ }  (updated per frame)
        fx.uniform(vhs_pass,      0, (float*)&(float4){ 0.f, 0.05f, 0.005f, 0.003f });
        fx.uniform(vignette_pass, 0, (float*)&(float4){ 0.75f, 0.45f, 0.8f,  0.f   });

        fx.enable(fxaa_pass, false); // off by default; toggle with F
    }

    // --- events
    if (ev.emit) {
        //SDL_ConvertEventToRenderCoordinates(render.handle, ev.emit);
        ev.emit = imgui.event(ev.emit);
        if(!ev.emit) return;
        if (ev.emit->type == SDL_EVENT_QUIT) app.quit(0);
        if (ev.emit->type == SDL_EVENT_KEY_DOWN) {
            switch (ev.emit->key.key) {
            case SDLK_ESCAPE: app.quit(0); break;
            case SDLK_F:
                fx.enable(fxaa_pass, fx.enable(fxaa_pass, -1) ^ 1);
                break;
            case SDLK_V:
                fx.enable(vhs_pass, fx.enable(vhs_pass, -1) ^ 1);
                break;
            case SDLK_B:
                fx.enable(vignette_pass, fx.enable(vignette_pass, -1) ^ 1);
                break;
            }
        }
    }

    // --- tick (variable dt)
    if (ev.tick) {
        float2_t sz = render.size();
        elapsed += ev.tick;

        // update time for VHS (u[0].x)
        float* vhs_u = fx.uniform(vhs_pass, 0, NULL);
        vhs_u[0] = elapsed;
        fx.uniform(vhs_pass, 0, vhs_u);

        // update inv_resolution for FXAA (u[0].xy)
        fx.uniform(fxaa_pass, 0, (float*)&(float4){ 1.f/sz.w, 1.f/sz.h, 0, 0 });

        // --- render
        render.clear(KIT_BLACK);

        // 1. get scene render target (NULL if all passes disabled)
        SDL_GPUTexture *scene_rt = fx_begin();

        if (scene_rt) {
            // 2. render 3D/2D scene into scene_rt via SDL_GPU
            render_scene(scene_rt);
        } else {
            // No postfx: draw directly via SDL_Renderer as normal
            SDL_SetRenderDrawColor(render.handle, 25, 76, 127, 255);
            SDL_RenderClear(render.handle);
        }

        // 3. run FX chain + blit result into SDL_Renderer framebuffer
        fx.end();

        // 4. draw any SDL_Renderer HUD / UI on top
        {
            imgui.begin();
            static int open = 1;
            if (ui.window("PostFX", &open))
            {
                bool on;
                on = fx.enable(fxaa_pass, -1);     if( ui.boolean("FXAA", &on) )        fx.enable(fxaa_pass, on);
                on = fx.enable(vhs_pass, -1);      if( ui.boolean("VHS", &on) )         fx.enable(vhs_pass, on);
                on = fx.enable(vignette_pass, -1); if( ui.boolean("Vignette", &on) )    fx.enable(vignette_pass, on);
                ui.window_end();
            }
            imgui.end();
        }

        render.present();
    }

    // --- quit
    if (ev.quit) {
        imgui.close();
        fx.close();
    }
}

const char *hints;
