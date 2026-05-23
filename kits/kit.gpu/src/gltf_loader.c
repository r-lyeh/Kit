/*
 * gltf_loader.c  –  glTF/GLB loader (cgltf) v3
 *
 * Texture loading fixes:
 *   - GLB embedded images loaded via stbi_load_from_memory (no temp files)
 *   - Keyed by image index in the cgltf_data array → no pointer aliasing
 *   - Normal map falls back to flat_normal (128,128,255) not white (255,255,255)
 *   - External URIs still path-cached via tcache_load_file
 *   - Logs every map that fails so you can spot missing textures immediately
 */

#define CGLTF_IMPLEMENTATION
#include <cgltf.h>

/* stb_image declarations only – implementation is in texture_cache.c */
#include <stb_image.h>

#include "gltf_loader.h"
#include "tangent_gen.h"
#include "mesh.h"

#include <SDL3/SDL.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

/* ─── Path helpers ─────────────────────────────────────────────────────────── */

static void resolve_uri(const char *gltf_path, const char *uri,
                         char *out, size_t out_sz)
{
    const char *sep = NULL;
    for (const char *p = gltf_path; *p; p++)
        if (*p=='/' || *p=='\\') sep = p;

    if (sep) {
        size_t dlen = (size_t)(sep - gltf_path) + 1;
        if (dlen >= out_sz) dlen = out_sz-1;
        memcpy(out, gltf_path, dlen);
        SDL_strlcpy(out+dlen, uri, out_sz-dlen);
    } else {
        SDL_strlcpy(out, uri, out_sz);
    }
}

/* ─── Matrix helpers ───────────────────────────────────────────────────────── */

static void node_world_matrix(const cgltf_node *node, float out[16])
{
    float local[16];
    cgltf_node_transform_local(node, local);
    if (node->parent) {
        float parent[16];
        node_world_matrix(node->parent, parent);
        float tmp[16] = {0};
        for (int r=0;r<4;r++)
            for (int c=0;c<4;c++)
                for (int k=0;k<4;k++)
                    tmp[c*4+r] += parent[k*4+r] * local[c*4+k];
        memcpy(out, tmp, 64);
    } else {
        memcpy(out, local, 64);
    }
}

/* ─── Accessor helpers ─────────────────────────────────────────────────────── */

static void read_vec2(const cgltf_accessor *a, cgltf_size i, float o[2])
{ cgltf_accessor_read_float(a,i,o,2); }
static void read_vec3(const cgltf_accessor *a, cgltf_size i, float o[3])
{ cgltf_accessor_read_float(a,i,o,3); }
static void read_vec4(const cgltf_accessor *a, cgltf_size i, float o[4])
{ cgltf_accessor_read_float(a,i,o,4); }
static uint32_t read_index(const cgltf_accessor *a, cgltf_size i)
{ uint32_t v=0; cgltf_accessor_read_uint(a,i,&v,1); return v; }

/* ─── Texture loading ──────────────────────────────────────────────────────── */

/*
 * Load a cgltf_texture, picking the right cache path:
 *   - Embedded (buffer_view) → tcache_load_memory keyed by image index
 *   - External URI           → tcache_load_file keyed by absolute path
 *   - Missing                → fallback (caller decides: white or flat_normal)
 */
static GpuTex load_tex(SDL_GPUDevice       *gpu,
                        TextureCache        *tc,
                        const cgltf_data    *data,
                        const cgltf_texture *tex,
                        const char          *gltf_path,
                        GpuTex               fallback)
{
    if (!tex || !tex->image) return fallback;

    const cgltf_image *img = tex->image;

    if (img->buffer_view) {
        /* Image index in data->images[] – stable, unique cache key per image */
        uint64_t img_idx = (uint64_t)(img - data->images);
        const cgltf_buffer_view *bv = img->buffer_view;
        const uint8_t *buf = (const uint8_t*)bv->buffer->data + bv->offset;
        SDL_Log("    load_tex: embedded image[%llu] '%s' %zu bytes",
                (unsigned long long)img_idx,
                img->name ? img->name : img->uri ? img->uri : "?",
                bv->size);
        return tcache_load_memory(gpu, tc, img_idx, buf, bv->size);
    }

    if (img->uri) {
        char abs[512];
        resolve_uri(gltf_path, img->uri, abs, sizeof(abs));
        return tcache_load_file(gpu, tc, abs);
    }

    return fallback;
}

/* ─── Material loading ─────────────────────────────────────────────────────── */

static void load_material(SDL_GPUDevice        *gpu,
                           TextureCache         *tc,
                           const cgltf_data     *data,
                           const cgltf_material *src,
                           Material             *dst,
                           const char           *gltf_path)
{
    GpuTex white  = tcache_white(tc);
    GpuTex flat_n = tcache_flat_normal(tc);

    /* Defaults */
    dst->albedo             = white;
    dst->metallic_roughness = white;
    dst->normal_map         = flat_n;   /* ← identity normal, not white */
    dst->occlusion          = white;
    dst->emissive           = white;

    dst->base_color[0] = dst->base_color[1] =
    dst->base_color[2] = dst->base_color[3] = 1.f;
    dst->metallic_factor    = 1.f;
    dst->roughness_factor   = 1.f;
    dst->normal_scale       = 1.f;
    dst->occlusion_strength = 1.f;
    dst->emissive_factor[0] = dst->emissive_factor[1] = dst->emissive_factor[2] = 0.f;
    dst->alpha_cutoff        = 0.5f;
    dst->normal_y_sign       = 1.0f;   /* glTF = OpenGL convention by spec */
    dst->double_sided        = false;

    if (!src) return;

    dst->double_sided  = src->double_sided;
    dst->alpha_cutoff  = src->alpha_cutoff;
    dst->normal_y_sign = 1.0f;
    /* Alpha mode → blend pipeline selection */
    switch (src->alpha_mode) {
    case cgltf_alpha_mode_blend:  dst->blend_mode = BLEND_MODE_BLEND;  break;
    case cgltf_alpha_mode_mask:   dst->blend_mode = BLEND_MODE_MASK;   break;
    default:                      dst->blend_mode = BLEND_MODE_OPAQUE; break;
    }   /* glTF spec mandates OpenGL convention */
    dst->mr_swizzle    = 1.0f;   /* assume spec-correct until probed below */

    /* PBR metallic-roughness */
    if (src->has_pbr_metallic_roughness) {
        const cgltf_pbr_metallic_roughness *pbr = &src->pbr_metallic_roughness;
        for (int i=0;i<4;i++) dst->base_color[i] = pbr->base_color_factor[i];
        dst->metallic_factor  = pbr->metallic_factor;
        dst->roughness_factor = pbr->roughness_factor;

        if (pbr->base_color_texture.texture)
            dst->albedo = load_tex(gpu,tc,data,
                pbr->base_color_texture.texture, gltf_path, white);

        if (pbr->metallic_roughness_texture.texture) {
            dst->metallic_roughness = load_tex(gpu,tc,data,
                pbr->metallic_roughness_texture.texture, gltf_path, white);

            /* Auto-detect channel swizzle by peeking at the raw image bytes.
             * glTF spec: B=metallic, G=roughness.
             * Legacy (pre-spec) assets like DamagedHelmet: R=metallic, G=roughness.
             * Heuristic: if the B channel of the first few pixels is near-zero
             * while R has significant values, assume legacy R=metallic layout. */
            const cgltf_image *mr_img = pbr->metallic_roughness_texture.texture->image;
            if (mr_img && mr_img->buffer_view) {
                const cgltf_buffer_view *bv = mr_img->buffer_view;
                const uint8_t *raw = (const uint8_t*)bv->buffer->data + bv->offset;

                /* stb_image decodes JPEG in R,G,B order → probe as JPEG.
                 * Sample 16 pixels at stride to get a representative spread. */
                int pw, ph, pch;
                uint8_t *probe = stbi_load_from_memory(raw, (int)bv->size,
                                                        &pw, &ph, &pch, 3);
                if (probe && pw > 0 && ph > 0) {
                    int stride = (pw * ph) / 16;
                    if (stride < 1) stride = 1;
                    float sum_r = 0, sum_b = 0;
                    int   count = 0;
                    for (int pi = 0; pi < pw * ph && count < 16; pi += stride, count++) {
                        sum_r += probe[pi * 3 + 0];
                        sum_b += probe[pi * 3 + 2];
                    }
                    stbi_image_free(probe);

                    /* If R significantly exceeds B, metallic is in R channel */
                    if (sum_r > sum_b * 3.0f && sum_r > 10.0f * count) {
                        dst->mr_swizzle = -1.0f;
                        SDL_Log("  metallic_roughness: detected legacy R=metallic layout (sum_r=%.0f sum_b=%.0f) – swizzle enabled",
                                sum_r, sum_b);
                    } else {
                        dst->mr_swizzle = 1.0f;
                        SDL_Log("  metallic_roughness: spec-correct B=metallic layout (sum_r=%.0f sum_b=%.0f)",
                                sum_r, sum_b);
                    }
                }
            }
        }
    }

    /* Normal – fallback to flat_normal so TBN is never corrupted */
    if (src->normal_texture.texture) {
        dst->normal_map  = load_tex(gpu,tc,data,
            src->normal_texture.texture, gltf_path, flat_n);
        dst->normal_scale = src->normal_texture.scale;
        if (dst->normal_scale == 0.f) dst->normal_scale = 1.f; /* guard */
    }

    /* Occlusion */
    if (src->occlusion_texture.texture) {
        dst->occlusion = load_tex(gpu,tc,data,
            src->occlusion_texture.texture, gltf_path, white);
        dst->occlusion_strength = src->occlusion_texture.scale;
    }

    /* Emissive */
    if (src->emissive_texture.texture)
        dst->emissive = load_tex(gpu,tc,data,
            src->emissive_texture.texture, gltf_path, white);
    for (int i=0;i<3;i++) dst->emissive_factor[i] = src->emissive_factor[i];

    /* Debug: log what we loaded */
    SDL_Log("  material '%s': albedo=%s nm=%s mr=%s ao=%s em=%s  M=%.2f R=%.2f",
        src->name ? src->name : "?",
        (dst->albedo.texture            != white.texture)  ? "OK" : "white",
        (dst->normal_map.texture        != flat_n.texture) ? "OK" : "flat",
        (dst->metallic_roughness.texture!= white.texture)  ? "OK" : "white",
        (dst->occlusion.texture         != white.texture)  ? "OK" : "white",
        (dst->emissive.texture          != white.texture)  ? "OK" : "white",
        dst->metallic_factor, dst->roughness_factor);
}

/* ─── Primitive loading ────────────────────────────────────────────────────── */

static Mesh *load_primitive(SDL_GPUDevice         *gpu,
                             TextureCache          *tc,
                             const cgltf_data      *data,
                             const cgltf_primitive *prim,
                             const float            transform[16],
                             const char            *gltf_path)
{
    if (prim->type != cgltf_primitive_type_triangles) return NULL;

    const cgltf_accessor *pos_acc=NULL, *nrm_acc=NULL,
                         *uv_acc=NULL,  *tan_acc=NULL;

    for (cgltf_size a=0; a<prim->attributes_count; a++) {
        const cgltf_attribute *attr = &prim->attributes[a];
        switch (attr->type) {
        case cgltf_attribute_type_position: pos_acc = attr->data; break;
        case cgltf_attribute_type_normal:   nrm_acc = attr->data; break;
        case cgltf_attribute_type_texcoord: if (!attr->index) uv_acc = attr->data; break;
        case cgltf_attribute_type_tangent:  tan_acc = attr->data; break;
        default: break;
        }
    }
    if (!pos_acc) return NULL;

    if (!tan_acc)
        SDL_Log("  primitive: no TANGENT attribute – generating via UV-gradient");

    uint32_t vc    = (uint32_t)pos_acc->count;
    Vertex  *verts = (Vertex*)calloc(vc, sizeof(Vertex));
    if (!verts) return NULL;

    for (uint32_t i=0; i<vc; i++) {
        read_vec3(pos_acc, i, verts[i].position);
        verts[i].position[3] = 1.f;   /* w=1 for positions */
        if (nrm_acc) { read_vec3(nrm_acc, i, verts[i].normal); verts[i].normal[3] = 0.f; }
        else { verts[i].normal[1]=1.f; verts[i].normal[3]=0.f; }
        if (uv_acc) { read_vec2(uv_acc, i, verts[i].texcoord);
                      verts[i].texcoord[2]=0.f; verts[i].texcoord[3]=0.f; }
        if (tan_acc) {
            read_vec4(tan_acc, i, verts[i].tangent);
        } else {
            /* tangent.w = 0 signals "no tangents provided".
             * The fragment shader will derive TBN from screen-space
             * derivatives (dFdx/dFdy) which is the correct fallback
             * per glTF spec §3.7.2.1 and the Khronos sample viewer. */
            verts[i].tangent[0] = 0.f;
            verts[i].tangent[1] = 0.f;
            verts[i].tangent[2] = 0.f;
            verts[i].tangent[3] = 0.f;   /* sentinel: no tangents */
        }
    }

    uint32_t  ic=0; uint32_t *idxs=NULL;
    if (prim->indices) {
        ic   = (uint32_t)prim->indices->count;
        idxs = (uint32_t*)malloc(ic*sizeof(uint32_t));
        if (!idxs) { free(verts); return NULL; }
        for (uint32_t i=0; i<ic; i++) idxs[i] = read_index(prim->indices,i);
    }

    /* Generate tangents if the mesh didn't supply them.
     * Uses the UV-gradient / Lengyel method – gives correct results for
     * welded, indexed meshes like DamagedHelmet.                        */
    if (!tan_acc && ic > 0 && idxs) {
        tangent_gen(verts, vc, idxs, ic);
        SDL_Log("  generated %u tangents via UV-gradient method", vc);
    }

    Mesh *m = mesh_create(gpu, verts, vc, idxs, ic);
    free(verts); free(idxs);
    if (!m) return NULL;

    if (transform) memcpy(m->transform, transform, 64);
    load_material(gpu, tc, data, prim->material, &m->material, gltf_path);
    return m;
}

/* ─── gltf_load_into_scene ─────────────────────────────────────────────────── */

bool gltf_load_into_scene(SDL_GPUDevice *gpu, const char *path, Scene *scene)
{
    cgltf_options opts = {0};
    cgltf_data   *data = NULL;

    if (cgltf_parse_file(&opts, path, &data) != cgltf_result_success) {
        SDL_Log("gltf_loader: parse failed '%s'", path);
        return false;
    }
    if (cgltf_load_buffers(&opts, data, path) != cgltf_result_success) {
        SDL_Log("gltf_loader: buffer load failed '%s'", path);
        cgltf_free(data); return false;
    }
    cgltf_validate(data);

    SDL_Log("gltf_loader: '%s'  images=%zu materials=%zu meshes=%zu",
            path, data->images_count, data->materials_count, data->meshes_count);

    uint32_t loaded = 0;
    for (cgltf_size ni=0; ni<data->nodes_count; ni++) {
        const cgltf_node *node = &data->nodes[ni];
        if (!node->mesh) continue;
        float world[16];
        node_world_matrix(node, world);
        for (cgltf_size pi=0; pi<node->mesh->primitives_count; pi++) {
            Mesh *m = load_primitive(gpu, scene->tcache, data,
                                     &node->mesh->primitives[pi], world, path);
            if (m) { scene_add_mesh(scene, m); loaded++; }
        }
    }

    cgltf_free(data);
    SDL_Log("gltf_loader: loaded %u mesh(es)", loaded);
    return loaded > 0;
}
