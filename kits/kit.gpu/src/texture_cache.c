/*
 * texture_cache.c  –  GPU texture cache (stb_image backend)
 *
 * Key design decisions:
 *   - File-based textures keyed by canonical path string
 *   - Memory-based (GLB embedded) textures keyed by a uint64 (image index)
 *     so there are NO temp files and NO pointer-reuse collisions
 *   - Uses stbi_load_from_memory directly for embedded images
 *   - Provides tcache_flat_normal() = (128,128,255,255) so the normal-map
 *     slot always decodes to a valid (0,0,1) tangent-space normal when absent
 */

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STBI_ONLY_HDR
#include <stb_image.h>

#include "texture_cache.h"
#include <SDL3/SDL.h>
#include <stdlib.h>
#include <string.h>

/* ─── Internal entry ───────────────────────────────────────────────────────── */

typedef enum { KEY_PATH, KEY_MEMORY } KeyKind;

typedef struct CacheEntry {
    KeyKind  kind;
    char     path[512];   /* KEY_PATH */
    uint64_t mem_key;     /* KEY_MEMORY */
    GpuTex   tex;
} CacheEntry;

struct TextureCache {
    CacheEntry entries[TCACHE_MAX_TEXTURES];
    uint32_t   count;
    GpuTex     white;
    GpuTex     flat_normal;
};

/* ─── GPU upload ───────────────────────────────────────────────────────────── */

static SDL_GPUSampler *make_sampler(SDL_GPUDevice *gpu)
{
    /* num_levels=1: mipmap_mode must be NEAREST.
     * LINEAR mip mode with a single mip level is undefined behaviour on some
     * Vulkan/D3D12 drivers and returns black (blue channel first to go).   */
    SDL_GPUSamplerCreateInfo si = {
        .min_filter        = SDL_GPU_FILTER_LINEAR,
        .mag_filter        = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode       = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
        .address_mode_u    = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        .address_mode_v    = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        .address_mode_w    = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        .max_anisotropy    = 1.f,   /* anisotropy requires mip chain; disable for single mip */
        .enable_anisotropy = false,
        .min_lod           = 0.f,
        .max_lod           = 0.f,   /* clamp to mip 0 only */
    };
    return SDL_CreateGPUSampler(gpu, &si);
}

static SDL_GPUTexture *upload_rgba8(SDL_GPUDevice *gpu,
                                    const uint8_t *pixels,
                                    int w, int h)
{
    uint32_t size = (uint32_t)(w * h * 4);

    SDL_GPUTextureCreateInfo tci = {
        .type                 = SDL_GPU_TEXTURETYPE_2D,
        .format               = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
        .usage                = SDL_GPU_TEXTUREUSAGE_SAMPLER,
        .width                = (uint32_t)w,
        .height               = (uint32_t)h,
        .layer_count_or_depth = 1,
        .num_levels           = 1,
        .sample_count         = SDL_GPU_SAMPLECOUNT_1,
    };
    SDL_GPUTexture *tex = SDL_CreateGPUTexture(gpu, &tci);
    if (!tex) { SDL_Log("upload_rgba8: %s", SDL_GetError()); return NULL; }

    SDL_GPUTransferBufferCreateInfo tbci = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = size
    };
    SDL_GPUTransferBuffer *tb = SDL_CreateGPUTransferBuffer(gpu, &tbci);
    if (!tb) { SDL_ReleaseGPUTexture(gpu, tex); return NULL; }

    void *mapped = SDL_MapGPUTransferBuffer(gpu, tb, false);
    memcpy(mapped, pixels, size);
    SDL_UnmapGPUTransferBuffer(gpu, tb);

    SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(gpu);
    SDL_GPUCopyPass      *cp  = SDL_BeginGPUCopyPass(cmd);

    SDL_GPUTextureTransferInfo src = {
        .transfer_buffer = tb, .offset = 0,
        .pixels_per_row  = (uint32_t)w, .rows_per_layer = (uint32_t)h,
    };
    SDL_GPUTextureRegion dst = {
        .texture = tex, .mip_level = 0, .layer = 0,
        .x=0,.y=0,.z=0,.w=(uint32_t)w,.h=(uint32_t)h,.d=1,
    };
    SDL_UploadToGPUTexture(cp, &src, &dst, false);
    SDL_EndGPUCopyPass(cp);
    SDL_SubmitGPUCommandBuffer(cmd);
    SDL_ReleaseGPUTransferBuffer(gpu, tb);
    return tex;
}

static GpuTex make_1x1(SDL_GPUDevice *gpu, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    uint8_t px[4] = {r, g, b, a};
    GpuTex gt = {0};
    gt.texture = upload_rgba8(gpu, px, 1, 1);
    gt.sampler = make_sampler(gpu);
    return gt;
}

/* ─── stbi decode helper ───────────────────────────────────────────────────── */

static GpuTex decode_and_upload(SDL_GPUDevice *gpu,
                                 const uint8_t *data, int len,
                                 const char *debug_label,
                                 GpuTex fallback)
{
    int w, h, ch;
    stbi_set_flip_vertically_on_load(0);
    uint8_t *px = stbi_load_from_memory(data, len, &w, &h, &ch, 4);
    if (!px) {
        SDL_Log("tcache: stbi failed '%s': %s", debug_label, stbi_failure_reason());
        return fallback;
    }

    /* Log first pixel before freeing */
    SDL_Log("tcache: '%s' (%dx%d ch%d) pixel[0]=R%d G%d B%d A%d",
            debug_label, w, h, ch, px[0], px[1], px[2], px[3]);

    GpuTex gt = {0};
    gt.texture = upload_rgba8(gpu, px, w, h);
    gt.sampler = make_sampler(gpu);
    stbi_image_free(px);

    if (!gt.texture || !gt.sampler) {
        if (gt.texture) SDL_ReleaseGPUTexture(gpu, gt.texture);
        if (gt.sampler) SDL_ReleaseGPUSampler(gpu, gt.sampler);
        return fallback;
    }
    return gt;
}

/* ─── tcache_create / destroy ──────────────────────────────────────────────── */

TextureCache *tcache_create(SDL_GPUDevice *gpu)
{
    TextureCache *tc = (TextureCache*)calloc(1, sizeof(TextureCache));
    if (!tc) return NULL;
    tc->white       = make_1x1(gpu, 255, 255, 255, 255);
    /* Flat normal: tangent-space (0,0,1) encodes as (128,128,255) in UNORM */
    tc->flat_normal = make_1x1(gpu, 128, 128, 255, 255);
    return tc;
}

void tcache_destroy(SDL_GPUDevice *gpu, TextureCache *tc)
{
    if (!tc) return;
    for (uint32_t i=0; i<tc->count; i++) {
        SDL_ReleaseGPUTexture(gpu, tc->entries[i].tex.texture);
        SDL_ReleaseGPUSampler(gpu, tc->entries[i].tex.sampler);
    }
    if (tc->white.texture)       SDL_ReleaseGPUTexture(gpu, tc->white.texture);
    if (tc->white.sampler)       SDL_ReleaseGPUSampler(gpu, tc->white.sampler);
    if (tc->flat_normal.texture) SDL_ReleaseGPUTexture(gpu, tc->flat_normal.texture);
    if (tc->flat_normal.sampler) SDL_ReleaseGPUSampler(gpu, tc->flat_normal.sampler);
    free(tc);
}

/* ─── tcache_load_file ─────────────────────────────────────────────────────── */

GpuTex tcache_load_file(SDL_GPUDevice *gpu, TextureCache *tc, const char *path)
{
    if (!path || !path[0]) return tc->white;

    for (uint32_t i=0; i<tc->count; i++)
        if (tc->entries[i].kind == KEY_PATH &&
            SDL_strcmp(tc->entries[i].path, path) == 0)
            return tc->entries[i].tex;

    if (tc->count >= TCACHE_MAX_TEXTURES) {
        SDL_Log("tcache: full, returning white"); return tc->white;
    }

    /* Read file into memory, then decode */
    size_t file_sz = 0;
    void  *file_data = SDL_LoadFile(path, &file_sz);
    if (!file_data) {
        SDL_Log("tcache: can't read '%s': %s", path, SDL_GetError());
        return tc->white;
    }

    GpuTex gt = decode_and_upload(gpu,
                    (const uint8_t*)file_data, (int)file_sz, path, tc->white);
    SDL_free(file_data);

    if (gt.texture == tc->white.texture) return tc->white;

    CacheEntry *e = &tc->entries[tc->count++];
    e->kind = KEY_PATH;
    SDL_strlcpy(e->path, path, sizeof(e->path));
    e->tex = gt;
    return gt;
}

/* ─── tcache_load_memory ───────────────────────────────────────────────────── */

GpuTex tcache_load_memory(SDL_GPUDevice *gpu, TextureCache *tc,
                           uint64_t key, const uint8_t *data, size_t size)
{
    if (!data || !size) return tc->white;

    for (uint32_t i=0; i<tc->count; i++)
        if (tc->entries[i].kind == KEY_MEMORY &&
            tc->entries[i].mem_key == key)
            return tc->entries[i].tex;

    if (tc->count >= TCACHE_MAX_TEXTURES) {
        SDL_Log("tcache: full"); return tc->white;
    }

    char label[32];
    SDL_snprintf(label, sizeof(label), "<embedded #%llu>", (unsigned long long)key);

    GpuTex gt = decode_and_upload(gpu, data, (int)size, label, tc->white);
    if (gt.texture == tc->white.texture) return tc->white;

    CacheEntry *e = &tc->entries[tc->count++];
    e->kind    = KEY_MEMORY;
    e->mem_key = key;
    e->tex     = gt;
    return gt;
}

GpuTex tcache_white(const TextureCache *tc)       { return tc->white; }
GpuTex tcache_flat_normal(const TextureCache *tc) { return tc->flat_normal; }
