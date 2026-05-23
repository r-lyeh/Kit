/*
 * gpu_pipeline.c  –  Material-keyed graphics pipeline cache
 */

#include "gpu_pipeline.h"
#include "mesh.h"   /* Vertex layout */
#include <SDL3/SDL.h>
#include <stdlib.h>
#include <string.h>

#ifndef SHADER_PATH
#  define SHADER_PATH "shaders/"
#endif

/* ── Shader loading ────────────────────────────────────────────────────────*/

static SDL_GPUShader *load_shader(SDL_GPUDevice     *gpu,
                                   const char        *filename,
                                   SDL_GPUShaderStage stage,
                                   uint32_t           uniform_bufs,
                                   uint32_t           samplers)
{
    char path[512];
    SDL_snprintf(path, sizeof(path), "%s%s", SHADER_PATH, filename);
    size_t sz   = 0;
    void  *code = SDL_LoadFile(path, &sz);
    if (!code) { SDL_Log("load_shader: '%s': %s", path, SDL_GetError()); return NULL; }

    SDL_GPUShaderCreateInfo ci = {
        .code                = (const Uint8*)code,
        .code_size           = sz,
        .entrypoint          = "main",
        .format              = SDL_GPU_SHADERFORMAT_SPIRV,
        .stage               = stage,
        .num_uniform_buffers = uniform_bufs,
        .num_samplers        = samplers,
    };
    SDL_GPUShader *s = SDL_CreateGPUShader(gpu, &ci);
    if (!s) SDL_Log("SDL_CreateGPUShader '%s': %s", filename, SDL_GetError());
    SDL_free(code);
    return s;
}

/* ── Vertex layout ─────────────────────────────────────────────────────────*/

/* All attributes are FLOAT4 (16 bytes each) – matches the all-vec4 Vertex struct.
 * Stride = 4 × 16 = 64 bytes, naturally aligned on all targets.             */
static const SDL_GPUVertexAttribute k_attrs[] = {
    { 0, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, offsetof(Vertex, position) },
    { 1, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, offsetof(Vertex, normal)   },
    { 2, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, offsetof(Vertex, texcoord) },
    { 3, 0, SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4, offsetof(Vertex, tangent)  },
};
static const SDL_GPUVertexBufferDescription k_vbuf[] = {{
    .slot = 0, .pitch = sizeof(Vertex),
    .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX, .instance_step_rate = 0,
}};

/* ── Pipeline cache ────────────────────────────────────────────────────────*/

typedef struct CacheEntry {
    PipelineKey              key;
    SDL_GPUGraphicsPipeline *pipeline;
    bool                     valid;
} CacheEntry;

struct PipelineCache {
    CacheEntry    entries[PIPELINE_CACHE_MAX];
    uint32_t      count;
    SDL_GPUDevice *gpu;
    SDL_Window    *window;
};

PipelineCache *pipeline_cache_create(SDL_GPUDevice *gpu, SDL_Window *window)
{
    PipelineCache *c = (PipelineCache*)calloc(1, sizeof(PipelineCache));
    c->gpu    = gpu;
    c->window = window;

    /* Pre-warm the most common pipelines: opaque back-culled + opaque double-sided */
    PipelineKey k0 = { SDL_GPU_CULLMODE_BACK, BLEND_MODE_OPAQUE };
    PipelineKey k1 = { SDL_GPU_CULLMODE_NONE, BLEND_MODE_OPAQUE };
    pipeline_cache_get(c, gpu, window, k0);
    pipeline_cache_get(c, gpu, window, k1);
    return c;
}

void pipeline_cache_destroy(SDL_GPUDevice *gpu, PipelineCache *cache)
{
    if (!cache) return;
    for (uint32_t i = 0; i < cache->count; i++)
        if (cache->entries[i].valid)
            SDL_ReleaseGPUGraphicsPipeline(gpu, cache->entries[i].pipeline);
    free(cache);
}

static SDL_GPUGraphicsPipeline *build_pipeline(SDL_GPUDevice *gpu,
                                                SDL_Window    *window,
                                                PipelineKey    key)
{
    /* 2 vert UBOs (VP + model), 3 frag UBOs (material + camera + scalability),
     * 8 frag samplers (5 material + 3 IBL)                                  */
    SDL_GPUShader *vert = load_shader(gpu, "mesh.vert.spv",
                                       SDL_GPU_SHADERSTAGE_VERTEX,   2, 0);
    SDL_GPUShader *frag = load_shader(gpu, "mesh.frag.spv",
                                       SDL_GPU_SHADERSTAGE_FRAGMENT, 3, 8);
    if (!vert || !frag) {
        if (vert) SDL_ReleaseGPUShader(gpu, vert);
        if (frag) SDL_ReleaseGPUShader(gpu, frag);
        return NULL;
    }

    /* Offscreen render target uses R8G8B8A8_UNORM.
     * We must NOT use SDL_GetGPUSwapchainTextureFormat here because the
     * pipeline renders to the offscreen RT (R8G8B8A8), not the swapchain
     * (which is B8G8R8A8 on most Windows Vulkan drivers).              */
    SDL_GPUTextureFormat swapchain_fmt = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    (void)window;  /* no longer needed for format query */

    /* Blend state per mode */
    SDL_GPUColorTargetBlendState blend = {0};
    switch (key.blend_mode) {
    case BLEND_MODE_OPAQUE:
    case BLEND_MODE_MASK:
        blend.enable_blend = false;
        break;
    case BLEND_MODE_BLEND:
        blend.enable_blend          = true;
        blend.src_color_blendfactor = SDL_GPU_BLENDFACTOR_SRC_ALPHA;
        blend.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        blend.color_blend_op        = SDL_GPU_BLENDOP_ADD;
        blend.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
        blend.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        blend.alpha_blend_op        = SDL_GPU_BLENDOP_ADD;
        break;
    default: break;
    }

    SDL_GPUColorTargetDescription color_desc = {
        .format      = swapchain_fmt,
        .blend_state = blend,
    };

    SDL_GPUGraphicsPipelineCreateInfo ci = {
        .vertex_shader   = vert,
        .fragment_shader = frag,
        .vertex_input_state = {
            .vertex_buffer_descriptions = k_vbuf,
            .num_vertex_buffers         = 1,
            .vertex_attributes          = k_attrs,
            .num_vertex_attributes      = 4,
        },
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .rasterizer_state = {
            .fill_mode  = SDL_GPU_FILLMODE_FILL,
            .cull_mode  = key.cull_mode,
            .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
        },
        .depth_stencil_state = {
            .enable_depth_test  = true,
            /* Disable depth writes for BLEND so transparent surfaces
             * don't occlude each other incorrectly.                  */
            .enable_depth_write = (key.blend_mode != BLEND_MODE_BLEND),
            .compare_op         = SDL_GPU_COMPAREOP_LESS,
        },
        .target_info = {
            .color_target_descriptions = &color_desc,
            .num_color_targets         = 1,
            .depth_stencil_format      = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
            .has_depth_stencil_target  = true,
        },
    };

    SDL_GPUGraphicsPipeline *pipe = SDL_CreateGPUGraphicsPipeline(gpu, &ci);
    if (!pipe) SDL_Log("build_pipeline: %s", SDL_GetError());

    SDL_ReleaseGPUShader(gpu, vert);
    SDL_ReleaseGPUShader(gpu, frag);
    return pipe;
}

SDL_GPUGraphicsPipeline *pipeline_cache_get(PipelineCache *cache,
                                              SDL_GPUDevice *gpu,
                                              SDL_Window    *window,
                                              PipelineKey    key)
{
    if (!gpu)    gpu    = cache->gpu;
    if (!window) window = cache->window;

    /* Cache hit */
    for (uint32_t i = 0; i < cache->count; i++) {
        CacheEntry *e = &cache->entries[i];
        if (e->valid &&
            e->key.cull_mode  == key.cull_mode &&
            e->key.blend_mode == key.blend_mode)
            return e->pipeline;
    }

    /* Cache miss – build and store */
    if (cache->count >= PIPELINE_CACHE_MAX) {
        SDL_Log("pipeline_cache: full!");
        return NULL;
    }

    SDL_GPUGraphicsPipeline *pipe = build_pipeline(gpu, window, key);
    if (!pipe) return NULL;

    CacheEntry *e = &cache->entries[cache->count++];
    e->key      = key;
    e->pipeline = pipe;
    e->valid    = true;

    SDL_Log("pipeline_cache: built pipeline cull=%d blend=%d (total=%u)",
            key.cull_mode, key.blend_mode, cache->count);
    return pipe;
}
