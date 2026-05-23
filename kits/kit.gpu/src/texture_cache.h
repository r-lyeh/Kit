#pragma once
/*
 * texture_cache.h  –  GPU texture + sampler cache
 *
 * Supports two load paths:
 *   tcache_load_file()   – load PNG/JPEG/BMP from a file path
 *   tcache_load_memory() – load directly from a raw byte buffer (GLB embedded)
 *
 * Special fallback textures (never fail):
 *   tcache_white()       – 1×1 RGBA (255,255,255,255)  neutral for all maps
 *   tcache_flat_normal() – 1×1 RGBA (128,128,255,255)  identity normal map
 */

#include <SDL3/SDL.h>
#include <stdint.h>

#define TCACHE_MAX_TEXTURES 512

typedef struct {
    SDL_GPUTexture *texture;
    SDL_GPUSampler *sampler;
} GpuTex;

typedef struct TextureCache TextureCache;

TextureCache *tcache_create(SDL_GPUDevice *gpu);
void          tcache_destroy(SDL_GPUDevice *gpu, TextureCache *tc);

/* Load from file path (cached by path string) */
GpuTex tcache_load_file(SDL_GPUDevice *gpu, TextureCache *tc, const char *path);

/* Load from raw memory (cached by unique integer key, e.g. image index in glTF) */
GpuTex tcache_load_memory(SDL_GPUDevice *gpu, TextureCache *tc,
                           uint64_t key, const uint8_t *data, size_t size);

/* Fallback textures */
GpuTex tcache_white(const TextureCache *tc);
GpuTex tcache_flat_normal(const TextureCache *tc);
