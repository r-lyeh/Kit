/*
 * mesh.c  –  GPU buffer management + textured draw calls
 */

#include "mesh.h"
#include <SDL3/SDL.h>
#include <stdlib.h>
#include <string.h>

/* ─── Material UBO (std140, 48 bytes) ────────────────────────────────────────
 * Sent as push constant / uniform buffer to slot 2 in the fragment shader.
 * Padded to 16-byte alignment throughout.
 * ─────────────────────────────────────────────────────────────────────────── */
/*
 * MaterialUBO – std140 layout must match the GLSL declaration exactly.
 *
 * std140 rule: a vec3 is 16-byte aligned and occupies 16 bytes (not 12).
 * So after base_color[4] (16 bytes) comes emissive at offset 16, and
 * metallic_factor starts at offset 32 – meaning emissive needs a 4-byte pad.
 *
 * Offsets:
 *   0   base_color[4]       16 bytes
 *  16   emissive_factor[3]  12 bytes
 *  28   _emissive_pad        4 bytes  ← padding after vec3
 *  32   metallic_factor      4 bytes
 *  36   roughness_factor     4 bytes
 *  40   normal_scale         4 bytes
 *  44   occlusion_strength   4 bytes
 *  48   alpha_cutoff         4 bytes
 *  52   normal_y_sign        4 bytes
 *  56   _pad[2]              8 bytes
 *  Total: 64 bytes
 */
/* MaterialUBO – vec3 emissive promoted to vec4 (packs normal_y_sign into .w)
 * to eliminate all std140 vec3 padding ambiguity.
 * Offsets:
 *   0  base_color[4]      16 bytes
 *  16  emissive_normal_y  16 bytes  ([0..2]=emissive, [3]=normal_y_sign)
 *  32  metallic_factor     4 bytes
 *  36  roughness_factor    4 bytes
 *  40  normal_scale        4 bytes
 *  44  occlusion_strength  4 bytes
 *  48  alpha_cutoff        4 bytes
 *  52  mr_swizzle          4 bytes
 *  56  _pad[2]             8 bytes
 *  Total: 64 bytes                                                          */
typedef struct MaterialUBO {
    float base_color[4];         /* offset  0 */
    float emissive_normal_y[4];  /* offset 16: [0..2]=emissive, [3]=normal_y_sign */
    float metallic_factor;       /* offset 32 */
    float roughness_factor;      /* offset 36 */
    float normal_scale;          /* offset 40 */
    float occlusion_strength;    /* offset 44 */
    float alpha_cutoff;          /* offset 48 */
    float mr_swizzle;            /* offset 52 */
    float _pad[2];               /* offset 56 → total 64 */
} MaterialUBO;

/* ─── Buffer upload helper ───────────────────────────────────────────────────── */

static SDL_GPUBuffer *upload_buffer(SDL_GPUDevice        *gpu,
                                    SDL_GPUBufferUsageFlags usage,
                                    const void           *data,
                                    uint32_t              size)
{
    SDL_GPUBufferCreateInfo bci = { .usage = usage, .size = size };
    SDL_GPUBuffer *buf = SDL_CreateGPUBuffer(gpu, &bci);
    if (!buf) { SDL_Log("CreateGPUBuffer: %s", SDL_GetError()); return NULL; }

    SDL_GPUTransferBufferCreateInfo tbci = {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD, .size = size
    };
    SDL_GPUTransferBuffer *tb = SDL_CreateGPUTransferBuffer(gpu, &tbci);
    if (!tb) { SDL_ReleaseGPUBuffer(gpu, buf); return NULL; }

    void *mapped = SDL_MapGPUTransferBuffer(gpu, tb, false);
    memcpy(mapped, data, size);
    SDL_UnmapGPUTransferBuffer(gpu, tb);

    SDL_GPUCommandBuffer *cmd = SDL_AcquireGPUCommandBuffer(gpu);
    SDL_GPUCopyPass      *cp  = SDL_BeginGPUCopyPass(cmd);
    SDL_GPUTransferBufferLocation src = { .transfer_buffer = tb, .offset = 0 };
    SDL_GPUBufferRegion           dst = { .buffer = buf, .offset = 0, .size = size };
    SDL_UploadToGPUBuffer(cp, &src, &dst, false);
    SDL_EndGPUCopyPass(cp);
    SDL_SubmitGPUCommandBuffer(cmd);
    SDL_ReleaseGPUTransferBuffer(gpu, tb);
    return buf;
}

/* ─── mesh_create ─────────────────────────────────────────────────────────────── */

Mesh *mesh_create(SDL_GPUDevice  *gpu,
                  const Vertex   *vertices, uint32_t vertex_count,
                  const uint32_t *indices,  uint32_t index_count)
{
    Mesh *m = (Mesh*)calloc(1, sizeof(Mesh));
    if (!m) return NULL;

    m->vertex_count = vertex_count;
    m->index_count  = index_count;

    /* Identity transform */
    m->transform[0]=m->transform[5]=m->transform[10]=m->transform[15]=1.f;

    /* Default material: white, roughness=1, metallic=0 */
    m->material.base_color[0] = m->material.base_color[1] =
    m->material.base_color[2] = m->material.base_color[3] = 1.f;
    m->material.roughness_factor  = 1.f;
    m->material.metallic_factor   = 0.f;
    m->material.normal_scale      = 1.f;
    m->material.occlusion_strength= 1.f;
    m->material.emissive_factor[0] =
    m->material.emissive_factor[1] =
    m->material.emissive_factor[2] = 0.f;
    m->material.normal_y_sign       = 1.f;
    m->material.mr_swizzle          = 1.f;
    m->material.blend_mode          = BLEND_MODE_OPAQUE;
    m->material.double_sided        = false;   /* spec-correct default */

    m->vertex_buf = upload_buffer(gpu, SDL_GPU_BUFFERUSAGE_VERTEX,
                                  vertices, vertex_count * sizeof(Vertex));
    if (index_count && indices)
        m->index_buf = upload_buffer(gpu, SDL_GPU_BUFFERUSAGE_INDEX,
                                     indices, index_count * sizeof(uint32_t));

    if (!m->vertex_buf || (index_count && !m->index_buf)) {
        mesh_destroy(gpu, m); return NULL;
    }
    return m;
}

/* ─── mesh_destroy ────────────────────────────────────────────────────────────── */

void mesh_destroy(SDL_GPUDevice *gpu, Mesh *m)
{
    if (!m) return;
    if (m->vertex_buf) SDL_ReleaseGPUBuffer(gpu, m->vertex_buf);
    if (m->index_buf)  SDL_ReleaseGPUBuffer(gpu, m->index_buf);
    /* Note: textures are owned by TextureCache, not by Mesh */
    free(m);
}

/* ─── mesh_draw ───────────────────────────────────────────────────────────────── */

void mesh_draw(Mesh *mesh, SDL_GPURenderPass *pass, SDL_GPUCommandBuffer *cmd)
{
    /* Slot 1: model matrix */
    SDL_PushGPUVertexUniformData(cmd, 1, mesh->transform, sizeof(mesh->transform));

    /* Slot 0 (frag): material factors */
    MaterialUBO ubo = {
        .base_color          = { mesh->material.base_color[0],
                                  mesh->material.base_color[1],
                                  mesh->material.base_color[2],
                                  mesh->material.base_color[3] },
        .emissive_normal_y   = { mesh->material.emissive_factor[0],
                                  mesh->material.emissive_factor[1],
                                  mesh->material.emissive_factor[2],
                                  mesh->material.normal_y_sign },
        .metallic_factor     = mesh->material.metallic_factor,
        .roughness_factor    = mesh->material.roughness_factor,
        .normal_scale        = mesh->material.normal_scale,
        .occlusion_strength  = mesh->material.occlusion_strength,
        .alpha_cutoff        = mesh->material.alpha_cutoff,
        .mr_swizzle          = mesh->material.mr_swizzle,
    };
    SDL_PushGPUFragmentUniformData(cmd, 0, &ubo, sizeof(ubo));

    /* Bind textures – 5 samplers match the fragment shader binding order:
     *   0 = albedo          1 = metallic_roughness
     *   2 = normal_map      3 = occlusion       4 = emissive         */
    SDL_GPUTextureSamplerBinding bindings[5] = {
        { mesh->material.albedo.texture,             mesh->material.albedo.sampler             },
        { mesh->material.metallic_roughness.texture, mesh->material.metallic_roughness.sampler },
        { mesh->material.normal_map.texture,         mesh->material.normal_map.sampler         },
        { mesh->material.occlusion.texture,          mesh->material.occlusion.sampler          },
        { mesh->material.emissive.texture,           mesh->material.emissive.sampler           },
    };
    SDL_BindGPUFragmentSamplers(pass, 0, bindings, 5);

    /* Vertex / index buffers */
    SDL_GPUBufferBinding vb = { .buffer = mesh->vertex_buf, .offset = 0 };
    SDL_BindGPUVertexBuffers(pass, 0, &vb, 1);

    if (mesh->index_buf) {
        SDL_GPUBufferBinding ib = { .buffer = mesh->index_buf, .offset = 0 };
        SDL_BindGPUIndexBuffer(pass, &ib, SDL_GPU_INDEXELEMENTSIZE_32BIT);
        SDL_DrawGPUIndexedPrimitives(pass, mesh->index_count, 1, 0, 0, 0);
    } else {
        SDL_DrawGPUPrimitives(pass, mesh->vertex_count, 1, 0, 0);
    }
}

/* ─── Scene ───────────────────────────────────────────────────────────────────── */

Scene *scene_create(SDL_GPUDevice *gpu, SDL_Window *window)
{
    Scene *s = (Scene*)calloc(1, sizeof(Scene));
    if (!s) return NULL;
    s->tcache         = tcache_create(gpu);
    s->pipeline_cache = pipeline_cache_create(gpu, window);
    return s;
}

void scene_destroy(SDL_GPUDevice *gpu, Scene *scene)
{
    if (!scene) return;
    for (uint32_t i = 0; i < scene->mesh_count; i++)
        mesh_destroy(gpu, scene->meshes[i]);
    tcache_destroy(gpu, scene->tcache);
    pipeline_cache_destroy(gpu, scene->pipeline_cache);
    free(scene);
}

bool scene_add_mesh(Scene *scene, Mesh *mesh)
{
    if (scene->mesh_count >= SCENE_MAX_MESHES) return false;
    scene->meshes[scene->mesh_count++] = mesh;
    return true;
}

/* Draw opaque and mask meshes (any order, depth write on) */
void scene_draw_opaque(Scene *scene, SDL_GPURenderPass *pass, SDL_GPUCommandBuffer *cmd)
{
    for (uint32_t i = 0; i < scene->mesh_count; i++) {
        Mesh *m = scene->meshes[i];
        if (m->material.blend_mode == BLEND_MODE_BLEND) continue;
        PipelineKey key = {
            .cull_mode  = m->material.double_sided
                        ? SDL_GPU_CULLMODE_NONE
                        : SDL_GPU_CULLMODE_BACK,
            .blend_mode = m->material.blend_mode,
        };
        SDL_GPUGraphicsPipeline *pipe =
            pipeline_cache_get(scene->pipeline_cache, NULL, NULL, key);
        if (pipe) SDL_BindGPUGraphicsPipeline(pass, pipe);
        mesh_draw(m, pass, cmd);
    }
}

/* Comparison for back-to-front sort (qsort callback) */
static float s_cam_pos[3];
static int mesh_dist_cmp(const void *a, const void *b)
{
    const Mesh *ma = *(const Mesh *const*)a;
    const Mesh *mb = *(const Mesh *const*)b;
    /* Use translation column of transform as mesh origin */
    float da = 0, db = 0;
    for (int i = 0; i < 3; i++) {
        float diff_a = ma->transform[12+i] - s_cam_pos[i];
        float diff_b = mb->transform[12+i] - s_cam_pos[i];
        da += diff_a * diff_a;
        db += diff_b * diff_b;
    }
    return (da < db) ? 1 : (da > db) ? -1 : 0;  /* far→near */
}

/* Draw transparent (BLEND) meshes back-to-front, depth write off */
void scene_draw_blend(Scene *scene, SDL_GPURenderPass *pass,
                      SDL_GPUCommandBuffer *cmd, const float cam_pos[3])
{
    /* Collect BLEND meshes */
    Mesh *blend_list[SCENE_MAX_MESHES];
    uint32_t blend_count = 0;
    for (uint32_t i = 0; i < scene->mesh_count; i++)
        if (scene->meshes[i]->material.blend_mode == BLEND_MODE_BLEND)
            blend_list[blend_count++] = scene->meshes[i];
    if (blend_count == 0) return;

    /* Sort back-to-front */
    s_cam_pos[0] = cam_pos[0];
    s_cam_pos[1] = cam_pos[1];
    s_cam_pos[2] = cam_pos[2];
    qsort(blend_list, blend_count, sizeof(Mesh*), mesh_dist_cmp);

    /* Draw – always double-sided for transparent geometry */
    PipelineKey key = { SDL_GPU_CULLMODE_NONE, BLEND_MODE_BLEND };
    SDL_GPUGraphicsPipeline *pipe =
        pipeline_cache_get(scene->pipeline_cache, NULL, NULL, key);
    if (pipe) SDL_BindGPUGraphicsPipeline(pass, pipe);

    for (uint32_t i = 0; i < blend_count; i++)
        mesh_draw(blend_list[i], pass, cmd);
}

/* ─── Placeholder ─────────────────────────────────────────────────────────────── */

void scene_add_placeholder(SDL_GPUDevice *gpu, Scene *scene)
{
    /* All attributes are float[4] now. Unused w/zw components zeroed. */
    static const Vertex verts[] = {
        {{ 0.0f,  0.5f, 0.f, 1.f}, {0.f,0.f,1.f,0.f}, {0.5f,0.f,0.f,0.f}, {1.f,0.f,0.f, 1.f}},
        {{ 0.5f, -0.5f, 0.f, 1.f}, {0.f,0.f,1.f,0.f}, {1.0f,1.f,0.f,0.f}, {1.f,0.f,0.f, 1.f}},
        {{-0.5f, -0.5f, 0.f, 1.f}, {0.f,0.f,1.f,0.f}, {0.0f,1.f,0.f,0.f}, {1.f,0.f,0.f, 1.f}},
    };
    Mesh *m = mesh_create(gpu, verts, 3, NULL, 0);
    if (!m) return;
    /* Give the placeholder a white albedo from the cache */
    GpuTex white = tcache_white(scene->tcache);
    m->material.albedo             = white;
    m->material.metallic_roughness = white;
    m->material.normal_map         = tcache_flat_normal(scene->tcache);
    m->material.occlusion          = white;
    m->material.emissive           = white;
    scene_add_mesh(scene, m);
}
