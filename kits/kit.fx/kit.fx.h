// post-processing FX chain
// - rlyeh, μLicensed
//
// Ported from r-lyeh/v2 OpenGL postfx chain.
// Uses SDL_GPU offscreen rendering + SDL_Renderer blit.
// Works regardless of which SDL_Renderer backend is active.
//
// ARCHITECTURE
// ------------
// SDL_GPU owns two ping-pong textures (offscreen, never claimed to a window).
// Your scene renders into FX.rt[FX.src]. The chain processes passes,
// ping-ponging between the two. The final result is downloaded to an
// SDL_Surface, wrapped into an SDL_Texture, and blitted via render.handle
// at the end of the frame: before render.present().
//
// When SDL_Renderer is using the GPU backend (SDL 3.2+), we skip the
// readback and wrap the SDL_GPUTexture directly via
// SDL_PROP_TEXTURE_CREATE_GPU_TEXTURE_POINTER instead.
//
// USAGE
// -----
//   #define FX_C   // once, before including
//   #include "kit.fx.h"
//
//   // init:
//   fx_init();
//   fx_add("vhs",      "shaders/vhs.frag.spv",      NULL);
//   fx_add("vignette", "shaders/vignette.frag.spv", NULL);
//   fx_add("fxaa",     "shaders/fxaa.frag.spv",     NULL);
//
//   // each tick:
//   SDL_GPUTexture *scene_rt = fx_begin();   // render your scene here
//   render_my_scene(scene_rt);
//   fx_end();                                // runs chain + blits to renderer
//
// SHADER CONVENTIONS  (see bottom of file)

#ifndef FX_H
#define FX_H

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

// ---------------------------------------------------------------------------
// API

// Call once after render.open(). Picks up render.handle + window.handle.
bool fx_init(void);

// Add a pass. frag_spv is required. if vert_spv is NULL, the built-in fullscreen triangle will be used.
// Returns pass index or -1.
int  fx_add(const char *frag_spv, const char *vert_spv);

// Toggle a pass on/off if valid argument is supplied. Returns current state in any case
bool fx_enable(int idx, int on);

// Set a per-pass uniform vec4 slot if valid argument is supplied. Returns current float4 state in any case
float* fx_uniform(int idx, int slot, float v[4]);

// Call at start of tick, before rendering your scene.
// Returns the SDL_GPUTexture to render your scene into.
// Returns NULL if no passes are enabled (render directly to swapchain via SDL_Renderer instead).
SDL_GPUTexture *fx_begin(void);

// Call after your scene is rendered into the RT returned by fx_begin().
// Runs the chain and blits the result via render.handle.
void fx_end(void);

// Free all resources.
void fx_destroy(void);

#endif // FX_H

// ---------------------------------------------------------------------------
// IMPLEMENTATION
// ---------------------------------------------------------------------------

#ifdef FX_C
#pragma once
#include <stdio.h>

// ---------------------------------------------------------------------------
// Limits / tunables

#ifndef FX_MAX_PASSES
#define FX_MAX_PASSES   32
#endif
#ifndef FX_MAX_UNIFORMS
#define FX_MAX_UNIFORMS  8   // vec4 slots per pass (= 128 bytes)
#endif

// ---------------------------------------------------------------------------
// Types

typedef struct { float x,y,z,w; } fx_vec4;

typedef struct {
    SDL_GPUGraphicsPipeline *pipeline;
    fx_vec4                  u[FX_MAX_UNIFORMS];
    int                      ucount;   // how many slots populated
    bool                     enabled;
} fx_pass;

typedef struct {
    // GPU resources
    SDL_GPUDevice   *dev;
    SDL_GPUSampler  *sampler;
    SDL_GPUTexture  *rt[2];        // ping-pong offscreen RTs
    int              src;          // rt[src] = scene renders here
    int              w, h;
    SDL_GPUTextureFormat fmt;

    // passes
    fx_pass  passes[FX_MAX_PASSES];
    int      pass_count;
    int      enabled_count;

    // blit back to SDL_Renderer
    SDL_Texture     *sdl_tex;      // wraps final GPU result for SDL_RenderTexture
    bool             gpu_renderer; // true = SDL_Renderer uses GPU backend (no readback)
} fx_t;

// ---------------------------------------------------------------------------
// Global instance (kit style: one per app, like `render`, `window`, etc.)

fx_t FX; // global instance

// ---------------------------------------------------------------------------
// Built-in fullscreen-triangle vertex shader SPIR-V from GLSL source:
//
//   #version 450
//   layout(location=0) out vec2 v_uv;
//   void main() {
//       // Fullscreen triangle: no vertex buffer needed
//       vec2 pos = vec2((gl_VertexIndex & 2) * 2.0 - 1.0,
//                       (gl_VertexIndex & 1) * 4.0 - 1.0);
//       gl_Position = vec4(pos, 0.0, 1.0);
//       v_uv = pos * 0.5 + 0.5;
//   }
//
// To regenerate:  glslc -fshader-stage=vert vert.glsl -o vert.spv
// Then embed:     xxd -i vert.spv

static const unsigned char full_quad_spv[] = {
    // SPIR-V header
  0x03, 0x02, 0x23, 0x07, 0x00, 0x00, 0x01, 0x00, 0x0b, 0x00, 0x0d, 0x00,
  0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x02, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x06, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x47, 0x4c, 0x53, 0x4c, 0x2e, 0x73, 0x74, 0x64, 0x2e, 0x34, 0x35, 0x30,
  0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x04, 0x00, 0x00, 0x00, 0x6d, 0x61, 0x69, 0x6e, 0x00, 0x00, 0x00, 0x00,
  0x18, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x29, 0x00, 0x00, 0x00,
  0x03, 0x00, 0x03, 0x00, 0x02, 0x00, 0x00, 0x00, 0xc2, 0x01, 0x00, 0x00,
  0x04, 0x00, 0x0a, 0x00, 0x47, 0x4c, 0x5f, 0x47, 0x4f, 0x4f, 0x47, 0x4c,
  0x45, 0x5f, 0x63, 0x70, 0x70, 0x5f, 0x73, 0x74, 0x79, 0x6c, 0x65, 0x5f,
  0x6c, 0x69, 0x6e, 0x65, 0x5f, 0x64, 0x69, 0x72, 0x65, 0x63, 0x74, 0x69,
  0x76, 0x65, 0x00, 0x00, 0x04, 0x00, 0x08, 0x00, 0x47, 0x4c, 0x5f, 0x47,
  0x4f, 0x4f, 0x47, 0x4c, 0x45, 0x5f, 0x69, 0x6e, 0x63, 0x6c, 0x75, 0x64,
  0x65, 0x5f, 0x64, 0x69, 0x72, 0x65, 0x63, 0x74, 0x69, 0x76, 0x65, 0x00,
  0x05, 0x00, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00, 0x6d, 0x61, 0x69, 0x6e,
  0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x03, 0x00, 0x0c, 0x00, 0x00, 0x00,
  0x70, 0x6f, 0x73, 0x00, 0x05, 0x00, 0x06, 0x00, 0x16, 0x00, 0x00, 0x00,
  0x67, 0x6c, 0x5f, 0x50, 0x65, 0x72, 0x56, 0x65, 0x72, 0x74, 0x65, 0x78,
  0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x06, 0x00, 0x16, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x67, 0x6c, 0x5f, 0x50, 0x6f, 0x73, 0x69, 0x74,
  0x69, 0x6f, 0x6e, 0x00, 0x06, 0x00, 0x07, 0x00, 0x16, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x67, 0x6c, 0x5f, 0x50, 0x6f, 0x69, 0x6e, 0x74,
  0x53, 0x69, 0x7a, 0x65, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x07, 0x00,
  0x16, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x67, 0x6c, 0x5f, 0x43,
  0x6c, 0x69, 0x70, 0x44, 0x69, 0x73, 0x74, 0x61, 0x6e, 0x63, 0x65, 0x00,
  0x06, 0x00, 0x07, 0x00, 0x16, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
  0x67, 0x6c, 0x5f, 0x43, 0x75, 0x6c, 0x6c, 0x44, 0x69, 0x73, 0x74, 0x61,
  0x6e, 0x63, 0x65, 0x00, 0x05, 0x00, 0x03, 0x00, 0x18, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x06, 0x00, 0x1c, 0x00, 0x00, 0x00,
  0x67, 0x6c, 0x5f, 0x56, 0x65, 0x72, 0x74, 0x65, 0x78, 0x49, 0x6e, 0x64,
  0x65, 0x78, 0x00, 0x00, 0x05, 0x00, 0x04, 0x00, 0x29, 0x00, 0x00, 0x00,
  0x76, 0x5f, 0x75, 0x76, 0x00, 0x00, 0x00, 0x00, 0x47, 0x00, 0x03, 0x00,
  0x16, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x48, 0x00, 0x05, 0x00,
  0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x48, 0x00, 0x05, 0x00, 0x16, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x48, 0x00, 0x05, 0x00, 0x16, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
  0x0b, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x48, 0x00, 0x05, 0x00,
  0x16, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x00, 0x00,
  0x04, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00, 0x1c, 0x00, 0x00, 0x00,
  0x0b, 0x00, 0x00, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x47, 0x00, 0x04, 0x00,
  0x29, 0x00, 0x00, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x13, 0x00, 0x02, 0x00, 0x02, 0x00, 0x00, 0x00, 0x21, 0x00, 0x03, 0x00,
  0x03, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x16, 0x00, 0x03, 0x00,
  0x06, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x17, 0x00, 0x04, 0x00,
  0x07, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
  0x15, 0x00, 0x04, 0x00, 0x08, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x2b, 0x00, 0x04, 0x00, 0x08, 0x00, 0x00, 0x00,
  0x09, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x04, 0x00,
  0x0a, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,
  0x20, 0x00, 0x04, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
  0x0a, 0x00, 0x00, 0x00, 0x2b, 0x00, 0x04, 0x00, 0x06, 0x00, 0x00, 0x00,
  0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xbf, 0x2c, 0x00, 0x05, 0x00,
  0x07, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00,
  0x0d, 0x00, 0x00, 0x00, 0x2b, 0x00, 0x04, 0x00, 0x06, 0x00, 0x00, 0x00,
  0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0x2c, 0x00, 0x05, 0x00,
  0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00,
  0x0d, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x05, 0x00, 0x07, 0x00, 0x00, 0x00,
  0x11, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00,
  0x2c, 0x00, 0x06, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00,
  0x0e, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00,
  0x17, 0x00, 0x04, 0x00, 0x13, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
  0x04, 0x00, 0x00, 0x00, 0x2b, 0x00, 0x04, 0x00, 0x08, 0x00, 0x00, 0x00,
  0x14, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x04, 0x00,
  0x15, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00,
  0x1e, 0x00, 0x06, 0x00, 0x16, 0x00, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00,
  0x06, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x00,
  0x20, 0x00, 0x04, 0x00, 0x17, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
  0x16, 0x00, 0x00, 0x00, 0x3b, 0x00, 0x04, 0x00, 0x17, 0x00, 0x00, 0x00,
  0x18, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x15, 0x00, 0x04, 0x00,
  0x19, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x2b, 0x00, 0x04, 0x00, 0x19, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x00, 0x1b, 0x00, 0x00, 0x00,
  0x01, 0x00, 0x00, 0x00, 0x19, 0x00, 0x00, 0x00, 0x3b, 0x00, 0x04, 0x00,
  0x1b, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x20, 0x00, 0x04, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
  0x07, 0x00, 0x00, 0x00, 0x2b, 0x00, 0x04, 0x00, 0x06, 0x00, 0x00, 0x00,
  0x21, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2b, 0x00, 0x04, 0x00,
  0x06, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3f,
  0x20, 0x00, 0x04, 0x00, 0x26, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
  0x13, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x00, 0x28, 0x00, 0x00, 0x00,
  0x03, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x3b, 0x00, 0x04, 0x00,
  0x28, 0x00, 0x00, 0x00, 0x29, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
  0x2b, 0x00, 0x04, 0x00, 0x06, 0x00, 0x00, 0x00, 0x2d, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x3f, 0x36, 0x00, 0x05, 0x00, 0x02, 0x00, 0x00, 0x00,
  0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
  0xf8, 0x00, 0x02, 0x00, 0x05, 0x00, 0x00, 0x00, 0x3b, 0x00, 0x04, 0x00,
  0x0b, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00,
  0x3e, 0x00, 0x03, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00,
  0x3d, 0x00, 0x04, 0x00, 0x19, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x00, 0x00,
  0x1c, 0x00, 0x00, 0x00, 0x41, 0x00, 0x05, 0x00, 0x1e, 0x00, 0x00, 0x00,
  0x1f, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x1d, 0x00, 0x00, 0x00,
  0x3d, 0x00, 0x04, 0x00, 0x07, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00,
  0x1f, 0x00, 0x00, 0x00, 0x51, 0x00, 0x05, 0x00, 0x06, 0x00, 0x00, 0x00,
  0x23, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x51, 0x00, 0x05, 0x00, 0x06, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
  0x20, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x50, 0x00, 0x07, 0x00,
  0x13, 0x00, 0x00, 0x00, 0x25, 0x00, 0x00, 0x00, 0x23, 0x00, 0x00, 0x00,
  0x24, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00,
  0x41, 0x00, 0x05, 0x00, 0x26, 0x00, 0x00, 0x00, 0x27, 0x00, 0x00, 0x00,
  0x18, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x03, 0x00,
  0x27, 0x00, 0x00, 0x00, 0x25, 0x00, 0x00, 0x00, 0x3d, 0x00, 0x04, 0x00,
  0x19, 0x00, 0x00, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00,
  0x41, 0x00, 0x05, 0x00, 0x1e, 0x00, 0x00, 0x00, 0x2b, 0x00, 0x00, 0x00,
  0x0c, 0x00, 0x00, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x3d, 0x00, 0x04, 0x00,
  0x07, 0x00, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x2b, 0x00, 0x00, 0x00,
  0x8e, 0x00, 0x05, 0x00, 0x07, 0x00, 0x00, 0x00, 0x2e, 0x00, 0x00, 0x00,
  0x2c, 0x00, 0x00, 0x00, 0x2d, 0x00, 0x00, 0x00, 0x50, 0x00, 0x05, 0x00,
  0x07, 0x00, 0x00, 0x00, 0x2f, 0x00, 0x00, 0x00, 0x2d, 0x00, 0x00, 0x00,
  0x2d, 0x00, 0x00, 0x00, 0x81, 0x00, 0x05, 0x00, 0x07, 0x00, 0x00, 0x00,
  0x30, 0x00, 0x00, 0x00, 0x2e, 0x00, 0x00, 0x00, 0x2f, 0x00, 0x00, 0x00,
  0x3e, 0x00, 0x03, 0x00, 0x29, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00,
  0xfd, 0x00, 0x01, 0x00, 0x38, 0x00, 0x01, 0x00
};

// ---------------------------------------------------------------------------
// Internal helpers

static void *fx__read_file(const char *path, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    *out_size = (size_t)ftell(f);
    rewind(f);
    void *buf = SDL_malloc(*out_size);
    if (buf) fread(buf, 1, *out_size, f);
    fclose(f);
    return buf;
}

static SDL_GPUShader *fx__shader(SDL_GPUDevice *dev, const char *path,
                                  SDL_GPUShaderStage stage,
                                  Uint32 num_samplers, Uint32 num_ubos)
{
    void *code = NULL;
    size_t size = 0;
    bool heap = false;

    if (path && path[0]) {
        code = fx__read_file(path, &size);
        heap = (code != NULL);
    }
    if (!code && stage == SDL_GPU_SHADERSTAGE_VERTEX) {
        code = (void*)full_quad_spv;
        size = sizeof(full_quad_spv);
    }
    if (!code) {
        SDL_Log("[kit.fx] missing shader: %s", path ? path : "(null)");
        return NULL;
    }

    SDL_GPUShaderCreateInfo ci = {
        .code                = code,
        .code_size           = size,
        .entrypoint          = "main",
        .format              = SDL_GPU_SHADERFORMAT_SPIRV,
        .stage               = stage,
        .num_samplers        = num_samplers,
        .num_uniform_buffers = num_ubos,
    };
    SDL_GPUShader *sh = SDL_CreateGPUShader(dev, &ci);
    if (!sh) SDL_Log("[kit.fx] SDL_CreateGPUShader failed (%s): %s",
                     path ? path : "built-in", SDL_GetError());
    if (heap) SDL_free(code);
    return sh;
}

static SDL_GPUTexture *fx__make_rt(SDL_GPUDevice *dev, int w, int h,
                                    SDL_GPUTextureFormat fmt)
{
    SDL_GPUTextureCreateInfo ci = {
        .type                 = SDL_GPU_TEXTURETYPE_2D,
        .format               = fmt,
        .usage                = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET |
                                SDL_GPU_TEXTUREUSAGE_SAMPLER,
        .width                = (Uint32)w,
        .height               = (Uint32)h,
        .layer_count_or_depth = 1,
        .num_levels           = 1,
    };
    SDL_GPUTexture *t = SDL_CreateGPUTexture(dev, &ci);
    if (!t) SDL_Log("[kit.fx] SDL_CreateGPUTexture failed: %s", SDL_GetError());
    return t;
}

static void fx__rebuild_sdl_tex(void);  // forward decl

// ---------------------------------------------------------------------------
// Public API

bool fx_init(void) {
    SDL_memset(&FX, 0, sizeof(FX));

    // Detect whether SDL_Renderer is using the GPU backend.
    // If so, we can wrap SDL_GPUTexture directly: no readback needed.
    const char *rname = SDL_GetRendererName(render.handle);
    FX.gpu_renderer = rname && SDL_strcmp(rname, "gpu") == 0;

    // Create a GPU device. We don't claim a window: purely offscreen.
    // On the GPU renderer path SDL may already have a device; we create
    // a separate one to stay independent.
    FX.dev = SDL_CreateGPUDevice(
        SDL_GPU_SHADERFORMAT_SPIRV |
//        SDL_GPU_SHADERFORMAT_DXIL  |
//        SDL_GPU_SHADERFORMAT_MSL |
        0,
        false, NULL);
    if (!FX.dev) {
        SDL_Log("[kit.fx] SDL_CreateGPUDevice failed: %s", SDL_GetError());
        return false;
    }

    // Pick a format that works for both GPU render target and SDL_Texture.
    FX.fmt = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;

    float2_t sz = render.size();
    FX.w = (int)sz.w;
    FX.h = (int)sz.h;

    FX.rt[0] = fx__make_rt(FX.dev, FX.w, FX.h, FX.fmt);
    FX.rt[1] = fx__make_rt(FX.dev, FX.w, FX.h, FX.fmt);
    if (!FX.rt[0] || !FX.rt[1]) return false;

    SDL_GPUSamplerCreateInfo sci = {
        .min_filter     = SDL_GPU_FILTER_LINEAR,
        .mag_filter     = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode    = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    };
    FX.sampler = SDL_CreateGPUSampler(FX.dev, &sci);
    if (!FX.sampler) return false;

    SDL_Log("[kit.fx] init ok. renderer=%s gpu_renderer=%d size=%dx%d",
            rname, FX.gpu_renderer, FX.w, FX.h);
    return true;
}

int fx_add(const char *frag_spv, const char *vert_spv) {
    if (FX.pass_count >= FX_MAX_PASSES) {
        SDL_Log("[kit.fx] too many passes");
        return -1;
    }
    int idx = FX.pass_count;
    fx_pass *p = &FX.passes[idx];
    SDL_memset(p, 0, sizeof(*p));
    p->enabled = true;

    SDL_GPUShader *vert = fx__shader(FX.dev, vert_spv,
                                     SDL_GPU_SHADERSTAGE_VERTEX, 0, 0);
    SDL_GPUShader *frag = fx__shader(FX.dev, frag_spv,
                                     SDL_GPU_SHADERSTAGE_FRAGMENT, 1, 1);
    if (!vert || !frag) {
        if (vert) SDL_ReleaseGPUShader(FX.dev, vert);
        if (frag) SDL_ReleaseGPUShader(FX.dev, frag);
        return -1;
    }

    SDL_GPUColorTargetDescription ctd = { .format = FX.fmt };
    SDL_GPUGraphicsPipelineCreateInfo pci = {
        .vertex_shader    = vert,
        .fragment_shader  = frag,
        .primitive_type   = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .target_info = {
            .color_target_descriptions = &ctd,
            .num_color_targets         = 1,
        },
        .rasterizer_state  = { .fill_mode = SDL_GPU_FILLMODE_FILL },
        .multisample_state = { .sample_count = SDL_GPU_SAMPLECOUNT_1 },
        .depth_stencil_state = {
            .enable_depth_test  = false,
            .enable_depth_write = false,
        },
    };
    p->pipeline = SDL_CreateGPUGraphicsPipeline(FX.dev, &pci);
    SDL_ReleaseGPUShader(FX.dev, vert);
    SDL_ReleaseGPUShader(FX.dev, frag);

    if (!p->pipeline) {
        SDL_Log("[kit.fx] SDL_CreateGPUGraphicsPipeline failed (%s) %s", frag_spv, SDL_GetError());
        return -1;
    }

    FX.pass_count++;
    FX.enabled_count++;
    SDL_Log("[kit.fx] pass[%d] '%s' added", idx, frag_spv);
    return idx;
}

bool fx_enable(int idx, int on) {
    if (idx < 0 || idx >= FX.pass_count) return false;
    bool was = FX.passes[idx].enabled;
    if ( on < 0 || on > 1 ) return was;
    FX.passes[idx].enabled = on;
    if (was && !on) FX.enabled_count--;
    if (!was &&  on) FX.enabled_count++;
    return on;
}

float* fx_uniform(int idx, int slot, float v[4]) {
    if (idx  < 0 || idx  >= FX.pass_count)    return NULL;
    if (slot < 0 || slot >= FX_MAX_UNIFORMS)   return NULL;
    if( v ) {
        FX.passes[idx].u[slot] = (fx_vec4){v[0],v[1],v[2],v[3]};
        if (slot >= FX.passes[idx].ucount)
            FX.passes[idx].ucount = slot + 1;
    }
    return &FX.passes[idx].u[slot].x;
}

// ---------------------------------------------------------------------------

static void fx__resize_if_needed(void) {
    float2_t sz = render.size();
    int w = (int)sz.w, h = (int)sz.h;
    if (w == FX.w && h == FX.h) return;
    SDL_ReleaseGPUTexture(FX.dev, FX.rt[0]);
    SDL_ReleaseGPUTexture(FX.dev, FX.rt[1]);
    if (FX.sdl_tex && !FX.gpu_renderer) {
        SDL_DestroyTexture(FX.sdl_tex);
        FX.sdl_tex = NULL;
    }
    FX.rt[0] = fx__make_rt(FX.dev, w, h, FX.fmt);
    FX.rt[1] = fx__make_rt(FX.dev, w, h, FX.fmt);
    FX.w = w; FX.h = h;
}

// Wrap the final SDL_GPUTexture as an SDL_Texture for SDL_Renderer.
// Two paths:
//   A) GPU renderer (SDL 3.2+): use SDL_PROP_TEXTURE_CREATE_GPU_TEXTURE_POINTER
//     : zero copy, but requires renderer's internal GPU device to match ours.
//      Since we create a separate device this may fail; fall through to B.
//   B) Generic: readback via SDL_DownloadFromGPUTexture -> surface -> SDL_Texture.
static void fx__rebuild_sdl_tex_gpu_path(SDL_GPUTexture *src_gpu_tex) {
    // Only valid when SDL_Renderer uses its own GPU backend device.
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_FORMAT_NUMBER,
                          SDL_PIXELFORMAT_RGBA32);
    SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_ACCESS_NUMBER,
                          SDL_TEXTUREACCESS_STATIC);
    SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_WIDTH_NUMBER,  FX.w);
    SDL_SetNumberProperty(props, SDL_PROP_TEXTURE_CREATE_HEIGHT_NUMBER, FX.h);
    SDL_SetPointerProperty(props, SDL_PROP_TEXTURE_CREATE_GPU_TEXTURE_POINTER,
                           src_gpu_tex);
    SDL_Texture *t = SDL_CreateTextureWithProperties(render.handle, props);
    SDL_DestroyProperties(props);
    if (t) {
        if (FX.sdl_tex) SDL_DestroyTexture(FX.sdl_tex);
        FX.sdl_tex = t;
        return;
    }
    SDL_Log("[kit.fx] GPU wrap failed (%s), falling back to readback", SDL_GetError());
    FX.gpu_renderer = false; // don't try again
}

static void fx__rebuild_sdl_tex_readback(SDL_GPUTexture *src_gpu_tex) {
    // Create or reuse an SDL_Texture for CPU readback path.
    if (!FX.sdl_tex) {
        FX.sdl_tex = SDL_CreateTexture(render.handle,
                                        SDL_PIXELFORMAT_RGBA32,
                                        SDL_TEXTUREACCESS_STREAMING,
                                        FX.w, FX.h);
        if (!FX.sdl_tex) {
            SDL_Log("[kit.fx] SDL_CreateTexture failed: %s", SDL_GetError());
            return;
        }
    }

    // Download GPU -> CPU
    SDL_Surface *surf = SDL_CreateSurface(FX.w, FX.h, SDL_PIXELFORMAT_RGBA32);
    if (!surf) return;

    SDL_GPUCommandBuffer *cmdbuf = SDL_AcquireGPUCommandBuffer(FX.dev);
    SDL_GPUCopyPass *cp = SDL_BeginGPUCopyPass(cmdbuf);

    SDL_GPUTextureRegion region = {
        .texture = src_gpu_tex,
        .w = (Uint32)FX.w, .h = (Uint32)FX.h, .d = 1,
    };
    SDL_GPUTextureTransferInfo dst = {
        .transfer_buffer = NULL, // filled below
        .pixels_per_row  = (Uint32)FX.w,
        .rows_per_layer  = (Uint32)FX.h,
    };

    // Transfer buffer for readback
    SDL_GPUTransferBufferCreateInfo tbci = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_DOWNLOAD,
        .size  = (Uint32)(FX.w * FX.h * 4),
    };
    SDL_GPUTransferBuffer *tb = SDL_CreateGPUTransferBuffer(FX.dev, &tbci);
    dst.transfer_buffer = tb;

    SDL_DownloadFromGPUTexture(cp, &region, &dst);
    SDL_EndGPUCopyPass(cp);

    SDL_GPUFence *fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmdbuf);
    SDL_WaitForGPUFences(FX.dev, true, &fence, 1);
    SDL_ReleaseGPUFence(FX.dev, fence);

    void *mapped = SDL_MapGPUTransferBuffer(FX.dev, tb, false);
    if (mapped) {
        SDL_memcpy(surf->pixels, mapped, (size_t)(FX.w * FX.h * 4));
        SDL_UnmapGPUTransferBuffer(FX.dev, tb);
    }
    SDL_ReleaseGPUTransferBuffer(FX.dev, tb);

    SDL_UpdateTexture(FX.sdl_tex, NULL, surf->pixels, surf->pitch);
    SDL_DestroySurface(surf);
}

// ---------------------------------------------------------------------------

SDL_GPUTexture *fx_begin(void) {
    if (FX.enabled_count == 0) return NULL;
    fx__resize_if_needed();
    FX.src = 0;
    return FX.rt[FX.src];
}

void fx_end(void) {
    if (FX.enabled_count == 0) return;

    // Collect enabled pass indices
    int ei[FX_MAX_PASSES];
    int ec = 0;
    for (int i = 0; i < FX.pass_count; i++)
        if (FX.passes[i].enabled) ei[ec++] = i;

    int read = FX.src;
    int write = 1 - read;

    SDL_GPUCommandBuffer *cmdbuf = SDL_AcquireGPUCommandBuffer(FX.dev);

    for (int e = 0; e < ec; e++) {
        fx_pass *p = &FX.passes[ei[e]];
        bool last = (e == ec - 1);

        SDL_GPUColorTargetInfo ct = {
            .texture   = FX.rt[write],
            .load_op   = SDL_GPU_LOADOP_DONT_CARE,
            .store_op  = SDL_GPU_STOREOP_STORE,
            .cycle     = false,
        };
        SDL_GPURenderPass *rp = SDL_BeginGPURenderPass(cmdbuf, &ct, 1, NULL);

        SDL_BindGPUGraphicsPipeline(rp, p->pipeline);

        SDL_GPUTextureSamplerBinding tsb = {
            .texture = FX.rt[read],
            .sampler = FX.sampler,
        };
        SDL_BindGPUFragmentSamplers(rp, 0, &tsb, 1);

        // Always push at least one vec4 (= 16 bytes) so the UBO slot is valid
        int ucount = p->ucount > 0 ? p->ucount : 1;
        SDL_PushGPUFragmentUniformData(cmdbuf, 0,
            p->u, (Uint32)(ucount * sizeof(fx_vec4)));

        SDL_DrawGPUPrimitives(rp, 3, 1, 0, 0);
        SDL_EndGPURenderPass(rp);

        read  = write;
        write = 1 - write;

        (void)last; // last pass result stays in FX.rt[read] after loop
    }

    // Submit GPU work (without acquiring a fence if we're going readback:
    // the readback path acquires its own fence internally).
    SDL_SubmitGPUCommandBuffer(cmdbuf);

    // Blit final result (FX.rt[read]) back to SDL_Renderer
    SDL_GPUTexture *final_rt = FX.rt[read];

    if (FX.gpu_renderer) {
        fx__rebuild_sdl_tex_gpu_path(final_rt);
    } else {
        fx__rebuild_sdl_tex_readback(final_rt);
    }

    if (FX.sdl_tex) {
        SDL_RenderTexture(render.handle, FX.sdl_tex, NULL, NULL);
    }
}

void fx_destroy(void) {
    for (int i = 0; i < FX.pass_count; i++)
        if (FX.passes[i].pipeline)
            SDL_ReleaseGPUGraphicsPipeline(FX.dev, FX.passes[i].pipeline);
    if (FX.rt[0])   SDL_ReleaseGPUTexture(FX.dev, FX.rt[0]);
    if (FX.rt[1])   SDL_ReleaseGPUTexture(FX.dev, FX.rt[1]);
    if (FX.sampler) SDL_ReleaseGPUSampler(FX.dev, FX.sampler);
    if (FX.sdl_tex) SDL_DestroyTexture(FX.sdl_tex);
    if (FX.dev)     SDL_DestroyGPUDevice(FX.dev);
    SDL_memset(&FX, 0, sizeof(FX));
}

#endif // FX_C

// ===========================================================================
// SHADER CONVENTIONS
// ===========================================================================
//
// Vertex (built-in, no file needed: fullscreen triangle, no vertex buffer):
//
//   #version 450
//   layout(location=0) out vec2 v_uv;
//   void main() {
//       vec2 pos = vec2((gl_VertexIndex & 2) * 2.0 - 1.0,
//                       (gl_VertexIndex & 1) * 4.0 - 1.0);
//       gl_Position = vec4(pos, 0.0, 1.0);
//       v_uv = pos * 0.5 + 0.5;
//   }
//
//
// Fragment skeleton:
//
//   #version 450
//   layout(location=0) in  vec2 v_uv;
//   layout(location=0) out vec4 out_color;
//
//   layout(set=2, binding=0) uniform sampler2D u_tex; // source RT
//
//   layout(set=3, binding=0) uniform Params {
//       vec4 u[8]; // fx_vec4 array, up to FX_MAX_UNIFORMS slots
//   };
//
//   void main() {
//       out_color = texture(u_tex, v_uv);
//       // ... effect ...
//   }
//
//
// SDL_GPU descriptor set layout:
//   set=0  vertex uniform buffers
//   set=1  vertex samplers
//   set=2  fragment samplers   <- u_tex
//   set=3  fragment uniforms   <- Params
//
//
// Compile:
//   glslc -fshader-stage=frag vhs.frag.glsl   -o vhs.frag.spv
//   glslc -fshader-stage=vert tri.vert.glsl   -o tri.vert.spv          (optional)
