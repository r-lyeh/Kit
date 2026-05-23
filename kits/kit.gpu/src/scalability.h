#pragma once
/*
 * scalability.h  –  Multiplicative scalability settings tree
 *
 * Each node's *effective* value is the product of itself and all ancestors.
 * This means setting any ancestor to 0.0 disables the entire subtree.
 *
 * Node types:
 *   - Quality scalar  [0..1]  – continuous quality level (e.g. texture res)
 *   - Feature flag    [0|1]   – binary enable/disable; read via sc_flag()
 *   - Count           [0..N]  – integer capacity; read via sc_count()
 *
 * Effective value macros do the product chain for you:
 *   sc_eff_ibl(sc)        → float [0..1], 0 = IBL disabled
 *   sc_flag_ibl(sc)       → bool,  checks effective > 0.5
 *   sc_eff_mips(sc)       → float [0..1], 0 = no mipmaps, 1 = full chain
 *
 * Loading / saving:
 *   sc_defaults(&sc)      fill with all-1.0 defaults
 *   sc_load(&sc, path)    parse a simple "key = value" INI file
 *   sc_save(&sc, path)    write a commented INI file
 *   sc_apply_preset(&sc, preset)  LOW / MEDIUM / HIGH / ULTRA
 *
 * The struct is flat (plain floats) so it is trivially serialisable and
 * can be passed directly to shaders as a UBO if needed.
 */

#include <stdbool.h>

/* ── Preset names ─────────────────────────────────────────────────────────── */
typedef enum ScPreset {
    SC_PRESET_LOW    = 0,
    SC_PRESET_MEDIUM = 1,
    SC_PRESET_HIGH   = 2,
    SC_PRESET_ULTRA  = 3,
} ScPreset;

/* ── Settings tree ────────────────────────────────────────────────────────── */
typedef struct Scalability {

    /* ── Global ──────────────────────────────────────────────────────────── */
    float sc;                        /* [0..1] master scale */

    /* ── Render subtree ──────────────────────────────────────────────────── */
    float sc_render;                 /* [0..1] render quality */

    /* IBL (image-based lighting) */
    float sc_render_ibl;             /* [0|1] enable IBL */
    float sc_render_ibl_samples;     /* [0..1] prefilter sample quality
                                        0=64 samples, 1=1024 samples */

    /* Textures */
    float sc_render_texture;         /* [0..1] texture quality */
    float sc_render_texture_mipmaps; /* [0..1] 0=no mips, 1=full chain */
    float sc_render_texture_aniso;   /* [0..1] 0=1x, 1=16x anisotropy */

    /* Shadows (stub – not yet implemented) */
    float sc_render_shadows;         /* [0|1] enable shadows */
    float sc_render_shadows_res;     /* [0..1] shadow map resolution scale */

    /* Post-processing (stub) */
    float sc_render_post;            /* [0|1] enable post-processing */
    float sc_render_post_bloom;      /* [0|1] enable bloom */
    float sc_render_post_ao;         /* [0|1] enable screen-space AO */

    /* ── Audio subtree (stub) ────────────────────────────────────────────── */
    float sc_audio;                  /* [0..1] audio quality */
    float sc_audio_maxchannels;      /* [0..N] max simultaneous channels */

} Scalability;

/* ── Effective value helpers ─────────────────────────────────────────────── */

/* Continuous: product of node × all ancestors, clamped [0,1] */
#define sc_eff_render(s)       ((s)->sc_render               * (s)->sc)
#define sc_eff_ibl(s)          ((s)->sc_render_ibl            * sc_eff_render(s))
#define sc_eff_ibl_samples(s)  ((s)->sc_render_ibl_samples    * sc_eff_ibl(s))
#define sc_eff_texture(s)      ((s)->sc_render_texture        * sc_eff_render(s))
#define sc_eff_mipmaps(s)      ((s)->sc_render_texture_mipmaps * sc_eff_texture(s))
#define sc_eff_aniso(s)        ((s)->sc_render_texture_aniso  * sc_eff_texture(s))
#define sc_eff_shadows(s)      ((s)->sc_render_shadows        * sc_eff_render(s))
#define sc_eff_shadows_res(s)  ((s)->sc_render_shadows_res    * sc_eff_shadows(s))
#define sc_eff_post(s)         ((s)->sc_render_post           * sc_eff_render(s))
#define sc_eff_bloom(s)        ((s)->sc_render_post_bloom     * sc_eff_post(s))
#define sc_eff_ssao(s)         ((s)->sc_render_post_ao        * sc_eff_post(s))
#define sc_eff_audio(s)        ((s)->sc_audio                 * (s)->sc)

/* Boolean: effective > 0.5 */
#define sc_flag(eff)           ((eff) > 0.5f)
#define sc_flag_ibl(s)         sc_flag(sc_eff_ibl(s))
#define sc_flag_shadows(s)     sc_flag(sc_eff_shadows(s))
#define sc_flag_post(s)        sc_flag(sc_eff_post(s))
#define sc_flag_bloom(s)       sc_flag(sc_eff_bloom(s))
#define sc_flag_ssao(s)        sc_flag(sc_eff_ssao(s))

/* Integer count */
#define sc_count(eff, max_n)   ((int)((eff) * (max_n)))
#define sc_audio_channels(s)   sc_count(sc_eff_audio(s), (int)((s)->sc_audio_maxchannels))

/* IBL sample count: lerp 64 → 1024 */
static inline int sc_ibl_samples(const Scalability *s) {
    float e = sc_eff_ibl_samples(s);
    if (e < 0.001f) return 64;
    if (e > 0.999f) return 1024;
    return 64 + (int)(e * (1024 - 64));
}

/* ── Lifecycle ────────────────────────────────────────────────────────────── */

void sc_defaults(Scalability *s);
void sc_apply_preset(Scalability *s, ScPreset preset);
bool sc_load(Scalability *s, const char *path);   /* returns false if file missing */
bool sc_save(const Scalability *s, const char *path);
void sc_print(const Scalability *s);
