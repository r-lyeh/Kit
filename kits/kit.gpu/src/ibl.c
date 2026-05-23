/*
 * ibl.c  –  Image-Based Lighting precomputation
 *
 * All compute pipelines use SDL_GPUComputePipelineCreateInfo which embeds
 * the shader code directly (no separate SDL_GPUShader object for compute).
 *
 * SPIR-V resource set layout for compute (SDL3 docs):
 *   set 0: samplers + read-only storage textures + read-only storage buffers
 *   set 1: read-write storage textures + read-write storage buffers
 *   set 2: uniform buffers
 */

#include <stb_image.h>   /* declarations only – implementation in texture_cache.c */

#include "ibl.h"
#include <SDL3/SDL.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef SHADER_PATH
#  define SHADER_PATH "shaders/"
#endif

/* ── Compute pipeline loader ─────────────────────────────────────────────── */

static SDL_GPUComputePipeline *make_compute_pipeline(
    SDL_GPUDevice *gpu,
    const char    *spv_name,
    uint32_t       num_samplers,
    uint32_t       num_ro_textures,
    uint32_t       num_rw_textures,
    uint32_t       num_uniform_bufs,
    uint32_t       tx, uint32_t ty, uint32_t tz)
{
    char path[512];
    SDL_snprintf(path, sizeof(path), "%s%s", SHADER_PATH, spv_name);
    size_t sz   = 0;
    void  *code = SDL_LoadFile(path, &sz);
    if (!code) {
        SDL_Log("ibl: cannot load '%s': %s", path, SDL_GetError());
        return NULL;
    }

    SDL_GPUComputePipelineCreateInfo ci = {
        .code                           = (const Uint8*)code,
        .code_size                      = sz,
        .entrypoint                     = "main",
        .format                         = SDL_GPU_SHADERFORMAT_SPIRV,
        .num_samplers                   = num_samplers,
        .num_readonly_storage_textures  = num_ro_textures,
        .num_readwrite_storage_textures = num_rw_textures,
        .num_uniform_buffers            = num_uniform_bufs,
        .threadcount_x                  = tx,
        .threadcount_y                  = ty,
        .threadcount_z                  = tz,
    };

    SDL_GPUComputePipeline *pipe = SDL_CreateGPUComputePipeline(gpu, &ci);
    if (!pipe)
        SDL_Log("ibl: CreateGPUComputePipeline '%s': %s", spv_name, SDL_GetError());
    SDL_free(code);
    return pipe;
}

/* ── Texture helpers ─────────────────────────────────────────────────────── */

static SDL_GPUTexture *create_cubemap(SDL_GPUDevice *gpu,
                                       uint32_t size, uint32_t mips,
                                       SDL_GPUTextureFormat fmt,
                                       SDL_GPUTextureUsageFlags usage)
{
    SDL_GPUTextureCreateInfo ci = {
        .type                 = SDL_GPU_TEXTURETYPE_CUBE,
        .format               = fmt,
        .usage                = usage,
        .width                = size,
        .height               = size,
        .layer_count_or_depth = 6,
        .num_levels           = mips,
        .sample_count         = SDL_GPU_SAMPLECOUNT_1,
    };
    SDL_GPUTexture *t = SDL_CreateGPUTexture(gpu, &ci);
    if (!t) SDL_Log("ibl: create_cubemap: %s", SDL_GetError());
    return t;
}

static SDL_GPUTexture *create_tex2d(SDL_GPUDevice *gpu,
                                     uint32_t w, uint32_t h,
                                     SDL_GPUTextureFormat fmt,
                                     SDL_GPUTextureUsageFlags usage)
{
    SDL_GPUTextureCreateInfo ci = {
        .type                 = SDL_GPU_TEXTURETYPE_2D,
        .format               = fmt,
        .usage                = usage,
        .width                = w,
        .height               = h,
        .layer_count_or_depth = 1,
        .num_levels           = 1,
        .sample_count         = SDL_GPU_SAMPLECOUNT_1,
    };
    SDL_GPUTexture *t = SDL_CreateGPUTexture(gpu, &ci);
    if (!t) SDL_Log("ibl: create_tex2d: %s", SDL_GetError());
    return t;
}

static SDL_GPUSampler *make_sampler_cube(SDL_GPUDevice *gpu)
{
    SDL_GPUSamplerCreateInfo ci = {
        .min_filter     = SDL_GPU_FILTER_LINEAR,
        .mag_filter     = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode    = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .min_lod        = 0.f,
        .max_lod        = (float)IBL_PREFILTER_MIPS,
    };
    return SDL_CreateGPUSampler(gpu, &ci);
}

static SDL_GPUSampler *make_sampler_lut(SDL_GPUDevice *gpu)
{
    SDL_GPUSamplerCreateInfo ci = {
        .min_filter     = SDL_GPU_FILTER_LINEAR,
        .mag_filter     = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode    = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .min_lod        = 0.f,
        .max_lod        = 0.f,
    };
    return SDL_CreateGPUSampler(gpu, &ci);
}

/* Upload RGBA32F to a 2D texture */
static SDL_GPUTexture *upload_equirect(SDL_GPUDevice *gpu,
                                        const float   *pixels,
                                        int w, int h)
{
    uint32_t size = (uint32_t)(w * h * 4 * sizeof(float));

    SDL_GPUTextureCreateInfo tci = {
        .type                 = SDL_GPU_TEXTURETYPE_2D,
        .format               = SDL_GPU_TEXTUREFORMAT_R32G32B32A32_FLOAT,
        .usage                = SDL_GPU_TEXTUREUSAGE_SAMPLER,
        .width                = (uint32_t)w,
        .height               = (uint32_t)h,
        .layer_count_or_depth = 1,
        .num_levels           = 1,
        .sample_count         = SDL_GPU_SAMPLECOUNT_1,
    };
    SDL_GPUTexture *tex = SDL_CreateGPUTexture(gpu, &tci);
    if (!tex) return NULL;

    SDL_GPUTransferBufferCreateInfo tbci = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = size
    };
    SDL_GPUTransferBuffer *tb = SDL_CreateGPUTransferBuffer(gpu, &tbci);
    if (!tb) { SDL_ReleaseGPUTexture(gpu, tex); return NULL; }

    memcpy(SDL_MapGPUTransferBuffer(gpu, tb, false), pixels, size);
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

static SDL_GPUTexture *make_grey_equirect(SDL_GPUDevice *gpu)
{
    float px[4] = {0.5f, 0.5f, 0.5f, 1.0f};
    return upload_equirect(gpu, px, 1, 1);
}

/* ── Dispatch helpers ────────────────────────────────────────────────────── */

static void dispatch_equirect_to_cube(SDL_GPUDevice  *gpu,
                                       SDL_GPUTexture *equirect,
                                       SDL_GPUSampler *samp_2d,
                                       SDL_GPUTexture *cube_out,
                                       uint32_t        face_size)
{
    /* 1 sampler, 0 ro, 1 rw – shader writes one face per z thread group.
     * SDL3 binds the cubemap as a 2D array; each layer = one face.
     * We dispatch 6 z-groups and the shader uses gl_GlobalInvocationID.z
     * as the array layer (face index). One pass writes all 6 faces.        */
    SDL_GPUComputePipeline *pipe = make_compute_pipeline(gpu,
        "equirect_to_cube.comp.spv", 1, 0, 1, 1, 8, 8, 1);
    if (!pipe) return;

    for (uint32_t face = 0; face < 6; face++) {
        SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(gpu);

        SDL_GPUStorageTextureReadWriteBinding rw = {
            .texture = cube_out, .mip_level = 0, .layer = face, .cycle = false
        };
        SDL_GPUComputePass *cp = SDL_BeginGPUComputePass(cmd, &rw, 1, NULL, 0);
        SDL_BindGPUComputePipeline(cp, pipe);

        SDL_GPUTextureSamplerBinding samp_bind = { .texture = equirect, .sampler = samp_2d };
        SDL_BindGPUComputeSamplers(cp, 0, &samp_bind, 1);

        /* Face index uniform – pad to 16 bytes for std140 alignment */
        uint32_t face_ubo[4] = { face, 0, 0, 0 };
        SDL_PushGPUComputeUniformData(cmd, 0, face_ubo, sizeof(face_ubo));

        uint32_t g = (face_size + 7) / 8;
        SDL_DispatchGPUCompute(cp, g, g, 1);   /* z=1, face selected via uniform */
        SDL_EndGPUComputePass(cp);
        SDL_SubmitGPUCommandBuffer(cmd);
    }
    SDL_ReleaseGPUComputePipeline(gpu, pipe);
}

static void dispatch_irradiance(SDL_GPUDevice  *gpu,
                                 SDL_GPUTexture *env_cube,
                                 SDL_GPUSampler *samp_cube,
                                 SDL_GPUTexture *irr_out)
{
    /* One pass per face. Shader reads face index from uniform. */
    SDL_GPUComputePipeline *pipe = make_compute_pipeline(gpu,
        "irradiance_conv.comp.spv", 1, 0, 1, 1, 8, 8, 1);
    if (!pipe) return;

    for (uint32_t face = 0; face < 6; face++) {
        SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(gpu);
        uint32_t face_ubo[4] = { face, 0, 0, 0 };
        SDL_PushGPUComputeUniformData(cmd, 0, face_ubo, sizeof(face_ubo));

        SDL_GPUStorageTextureReadWriteBinding rw = {
            .texture = irr_out, .mip_level = 0, .layer = face, .cycle = false
        };
        SDL_GPUComputePass *cp = SDL_BeginGPUComputePass(cmd, &rw, 1, NULL, 0);
        SDL_BindGPUComputePipeline(cp, pipe);

        SDL_GPUTextureSamplerBinding samp_bind = { .texture = env_cube, .sampler = samp_cube };
        SDL_BindGPUComputeSamplers(cp, 0, &samp_bind, 1);

        uint32_t g = (IBL_IRRADIANCE_SIZE + 7) / 8;
        SDL_DispatchGPUCompute(cp, g, g, 1);
        SDL_EndGPUComputePass(cp);
        SDL_SubmitGPUCommandBuffer(cmd);
    }
    SDL_ReleaseGPUComputePipeline(gpu, pipe);
}

static void dispatch_prefilter(SDL_GPUDevice  *gpu,
                                SDL_GPUTexture *env_cube,
                                SDL_GPUSampler *samp_cube,
                                SDL_GPUTexture *pf_out,
                                float           env_res,
                                uint32_t        samples)
{
    /* Loop per mip × per face. Uniform: {roughness, env_res, face, _pad} */
    SDL_GPUComputePipeline *pipe = make_compute_pipeline(gpu,
        "prefilter_env.comp.spv", 1, 0, 1, 1, 8, 8, 1);
    if (!pipe) return;

    for (uint32_t mip = 0; mip < IBL_PREFILTER_MIPS; mip++) {
        float    roughness = (IBL_PREFILTER_MIPS > 1)
                           ? (float)mip / (float)(IBL_PREFILTER_MIPS - 1)
                           : 0.0f;
        uint32_t mip_sz    = SDL_max(1u, (uint32_t)(IBL_PREFILTER_SIZE >> mip));

        for (uint32_t face = 0; face < 6; face++) {
            float pc[4] = { roughness, env_res, (float)face, (float)samples };

            SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(gpu);
            SDL_PushGPUComputeUniformData(cmd, 0, pc, sizeof(pc));

            SDL_GPUStorageTextureReadWriteBinding rw = {
                .texture = pf_out, .mip_level = mip, .layer = face, .cycle = false
            };
            SDL_GPUComputePass *cp = SDL_BeginGPUComputePass(cmd, &rw, 1, NULL, 0);
            SDL_BindGPUComputePipeline(cp, pipe);

            SDL_GPUTextureSamplerBinding samp_bind = { .texture = env_cube, .sampler = samp_cube };
            SDL_BindGPUComputeSamplers(cp, 0, &samp_bind, 1);

            uint32_t g = (mip_sz + 7) / 8;
            SDL_DispatchGPUCompute(cp, g, g, 1);
            SDL_EndGPUComputePass(cp);
            SDL_SubmitGPUCommandBuffer(cmd);
        }
    }
    SDL_ReleaseGPUComputePipeline(gpu, pipe);
}

static void dispatch_brdf_lut(SDL_GPUDevice *gpu, SDL_GPUTexture *lut_out)
{
    /* 0 samplers, 0 ro, 1 rw (lut output), 0 uniforms */
    SDL_GPUComputePipeline *pipe = make_compute_pipeline(gpu,
        "brdf_lut.comp.spv", 0, 0, 1, 0, 8, 8, 1);
    if (!pipe) return;

    SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(gpu);

    SDL_GPUStorageTextureReadWriteBinding rw = {
        .texture = lut_out, .mip_level = 0, .layer = 0, .cycle = false
    };
    SDL_GPUComputePass *cp = SDL_BeginGPUComputePass(cmd, &rw, 1, NULL, 0);
    SDL_BindGPUComputePipeline(cp, pipe);

    uint32_t g = (IBL_BRDF_LUT_SIZE + 7) / 8;
    SDL_DispatchGPUCompute(cp, g, g, 1);
    SDL_EndGPUComputePass(cp);
    SDL_SubmitGPUCommandBuffer(cmd);
    SDL_ReleaseGPUComputePipeline(gpu, pipe);
}

/* ── ibl_load ────────────────────────────────────────────────────────────── */

bool ibl_load(IBL *ibl, SDL_GPUDevice *gpu, const char *hdr_path,
              uint32_t prefilter_samples)
{
    memset(ibl, 0, sizeof(IBL));

    /* 1. Load equirectangular HDR (or grey fallback) */
    SDL_GPUTexture *equirect = NULL;

    if (hdr_path) {
        int w, h, ch;
        stbi_set_flip_vertically_on_load(1);
        float *px = stbi_loadf(hdr_path, &w, &h, &ch, 4);
        stbi_set_flip_vertically_on_load(0);
        if (px) {
            equirect = upload_equirect(gpu, px, w, h);
            stbi_image_free(px);
            SDL_Log("ibl: loaded '%s' (%dx%d)", hdr_path, w, h);
        } else {
            SDL_Log("ibl: stbi_loadf('%s'): %s – using grey", hdr_path, stbi_failure_reason());
        }
    }
    if (!equirect) equirect = make_grey_equirect(gpu);
    if (!equirect) return false;

    SDL_GPUSamplerCreateInfo samp2d_ci = {
        .min_filter     = SDL_GPU_FILTER_LINEAR,
        .mag_filter     = SDL_GPU_FILTER_LINEAR,
        .mipmap_mode    = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_REPEAT,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .min_lod = 0.f, .max_lod = 0.f,
    };
    SDL_GPUSampler *samp_2d   = SDL_CreateGPUSampler(gpu, &samp2d_ci);
    SDL_GPUSampler *samp_cube = make_sampler_cube(gpu);

    /* 2. Env cubemap – intermediate, write-only during bake */
    SDL_GPUTexture *env_cube = create_cubemap(gpu, IBL_PREFILTER_SIZE, 1,
        SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT,
        SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_WRITE);

    dispatch_equirect_to_cube(gpu, equirect, samp_2d, env_cube, IBL_PREFILTER_SIZE);

    /* 3. Irradiance cubemap */
    ibl->irradiance = create_cubemap(gpu, IBL_IRRADIANCE_SIZE, 1,
        SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT,
        SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_WRITE);

    dispatch_irradiance(gpu, env_cube, samp_cube, ibl->irradiance);

    /* 4. Specular prefilter cubemap */
    ibl->prefilter = create_cubemap(gpu, IBL_PREFILTER_SIZE, IBL_PREFILTER_MIPS,
        SDL_GPU_TEXTUREFORMAT_R16G16B16A16_FLOAT,
        SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_WRITE);

    dispatch_prefilter(gpu, env_cube, samp_cube, ibl->prefilter,
                    (float)IBL_PREFILTER_SIZE, prefilter_samples);

    /* 5. BRDF LUT */
    ibl->brdf_lut = create_tex2d(gpu, IBL_BRDF_LUT_SIZE, IBL_BRDF_LUT_SIZE,
        SDL_GPU_TEXTUREFORMAT_R16G16_FLOAT,
        SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COMPUTE_STORAGE_WRITE);

    dispatch_brdf_lut(gpu, ibl->brdf_lut);

    ibl->sampler_cube = samp_cube;
    ibl->sampler_lut  = make_sampler_lut(gpu);

    /* Wait for all compute work, then release temporaries */
    SDL_WaitForGPUIdle(gpu);
    SDL_ReleaseGPUTexture(gpu, equirect);
    SDL_ReleaseGPUTexture(gpu, env_cube);
    SDL_ReleaseGPUSampler(gpu, samp_2d);

    SDL_Log("ibl: ready  irr=%upx  pf=%upx×%umips  lut=%upx",
            IBL_IRRADIANCE_SIZE, IBL_PREFILTER_SIZE,
            IBL_PREFILTER_MIPS, IBL_BRDF_LUT_SIZE);
    return true;
}

void ibl_destroy(IBL *ibl, SDL_GPUDevice *gpu)
{
    if (!ibl) return;
    if (ibl->irradiance)   SDL_ReleaseGPUTexture(gpu, ibl->irradiance);
    if (ibl->prefilter)    SDL_ReleaseGPUTexture(gpu, ibl->prefilter);
    if (ibl->brdf_lut)     SDL_ReleaseGPUTexture(gpu, ibl->brdf_lut);
    if (ibl->sampler_cube) SDL_ReleaseGPUSampler(gpu, ibl->sampler_cube);
    if (ibl->sampler_lut)  SDL_ReleaseGPUSampler(gpu, ibl->sampler_lut);
    memset(ibl, 0, sizeof(IBL));
}
