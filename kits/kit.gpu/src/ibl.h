#pragma once
/*
 * ibl.h  –  Image-Based Lighting
 *
 * Loads an equirectangular HDR environment map and derives three GPU
 * resources used by the PBR fragment shader:
 *
 *   ibl.irradiance   – 32×32 cubemap,  diffuse irradiance (pre-integrated)
 *   ibl.prefilter    – 128×128 cubemap, specular GGX prefilter, full mip chain
 *                      mip 0 = roughness 0 (mirror), last mip = roughness 1
 *   ibl.brdf_lut     – 512×512 RG16F 2-D texture,
 *                      U = NdotV, V = roughness → (scale, bias) for F term
 *
 * All convolutions run on the GPU as compute passes at startup.
 *
 * Usage:
 *   IBL ibl = {0};
 *   ibl_load(&ibl, gpu, "assets/env.hdr");   // or pass NULL for a solid grey
 *   // bind ibl.irradiance / ibl.prefilter / ibl.brdf_lut in render pass
 *   ibl_destroy(&ibl, gpu);
 */

#include <SDL3/SDL.h>
#include <stdbool.h>

/* Number of mip levels in the specular prefilter cubemap.
 * Each level maps to roughness = level / (IBL_PREFILTER_MIPS - 1).        */
#define IBL_PREFILTER_MIPS   5      /* 128 → 64 → 32 → 16 → 8 px          */
#define IBL_PREFILTER_SIZE   128
#define IBL_IRRADIANCE_SIZE  32
#define IBL_BRDF_LUT_SIZE    512

typedef struct IBL {
    SDL_GPUTexture  *irradiance;     /* cubemap, RGBA16F                    */
    SDL_GPUTexture  *prefilter;      /* cubemap, RGBA16F, IBL_PREFILTER_MIPS */
    SDL_GPUTexture  *brdf_lut;       /* 2D,      RG16F                      */
    SDL_GPUSampler  *sampler_cube;   /* linear, clamp-to-edge               */
    SDL_GPUSampler  *sampler_lut;    /* linear, clamp-to-edge               */
} IBL;

/*
 * Load and precompute IBL from an equirectangular .hdr file.
 * Pass path=NULL to generate a neutral grey environment (always works).
 */
bool ibl_load(IBL *ibl, SDL_GPUDevice *gpu, const char *hdr_path,
              uint32_t prefilter_samples);  /* e.g. 64..1024 */

void ibl_destroy(IBL *ibl, SDL_GPUDevice *gpu);
