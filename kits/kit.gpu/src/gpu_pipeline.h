#pragma once
/*
 * gpu_pipeline.h  –  Material-keyed pipeline cache
 *
 * The same GLSL shaders are used for all materials. What varies per-material
 * is rasterizer state (cull mode for double-sided) and blend state (opaque,
 * mask, blend). We cache one SDL_GPUGraphicsPipeline per unique combination
 * so we never create duplicates.
 *
 * Key dimensions:
 *   cull_mode  : BACK (default) | NONE (double-sided)
 *   blend_mode : OPAQUE | MASK | BLEND
 */

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum BlendMode {
    BLEND_MODE_OPAQUE = 0,
    BLEND_MODE_MASK   = 1,
    BLEND_MODE_BLEND  = 2,
    BLEND_MODE_COUNT  = 3,
} BlendMode;

typedef struct PipelineKey {
    SDL_GPUCullMode cull_mode;   /* BACK or NONE */
    BlendMode       blend_mode;
} PipelineKey;

/* Max distinct pipelines: 2 cull modes × 3 blend modes = 6 */
#define PIPELINE_CACHE_MAX 8

typedef struct PipelineCache PipelineCache;

PipelineCache           *pipeline_cache_create(SDL_GPUDevice *gpu, SDL_Window *window);
void                     pipeline_cache_destroy(SDL_GPUDevice *gpu, PipelineCache *cache);
SDL_GPUGraphicsPipeline *pipeline_cache_get(PipelineCache *cache,
                                             SDL_GPUDevice *gpu,
                                             SDL_Window    *window,
                                             PipelineKey    key);
