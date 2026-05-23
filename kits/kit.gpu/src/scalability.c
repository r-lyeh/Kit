/*
 * scalability.c  –  Scalability settings implementation
 */

#include "scalability.h"
#include <SDL3/SDL.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ── Defaults ─────────────────────────────────────────────────────────────── */

void sc_defaults(Scalability *s)
{
    s->sc                           = 1.0f;

    s->sc_render                    = 1.0f;

    s->sc_render_ibl                = 1.0f;
    s->sc_render_ibl_samples        = 1.0f;   /* 1024 samples */

    s->sc_render_texture            = 1.0f;
    s->sc_render_texture_mipmaps    = 1.0f;
    s->sc_render_texture_aniso      = 1.0f;

    s->sc_render_shadows            = 0.0f;   /* not yet implemented */
    s->sc_render_shadows_res        = 1.0f;

    s->sc_render_post               = 0.0f;   /* not yet implemented */
    s->sc_render_post_bloom         = 1.0f;
    s->sc_render_post_ao            = 1.0f;

    s->sc_audio                     = 1.0f;
    s->sc_audio_maxchannels         = 32.0f;
}

/* ── Presets ──────────────────────────────────────────────────────────────── */

void sc_apply_preset(Scalability *s, ScPreset preset)
{
    sc_defaults(s);   /* start from defaults, then override */

    switch (preset) {
    case SC_PRESET_LOW:
        s->sc_render                 = 0.5f;
        s->sc_render_ibl             = 0.0f;   /* hemisphere fallback */
        s->sc_render_texture_mipmaps = 0.5f;
        s->sc_render_texture_aniso   = 0.0f;   /* 1× aniso */
        s->sc_audio_maxchannels      = 8.0f;
        break;

    case SC_PRESET_MEDIUM:
        s->sc_render_ibl_samples     = 0.25f;  /* ~256 samples */
        s->sc_render_texture_aniso   = 0.25f;  /* ~4× aniso */
        s->sc_audio_maxchannels      = 16.0f;
        break;

    case SC_PRESET_HIGH:
        s->sc_render_ibl_samples     = 0.5f;   /* ~512 samples */
        s->sc_render_texture_aniso   = 0.5f;   /* ~8× aniso */
        s->sc_audio_maxchannels      = 24.0f;
        break;

    case SC_PRESET_ULTRA:
        /* all defaults = max quality */
        break;
    }
}

/* ── INI parser ───────────────────────────────────────────────────────────── */

/* Map key string to float* member in the struct */
typedef struct { const char *key; float *value; } KVEntry;

static void build_kv_table(Scalability *s, KVEntry *table, int *count)
{
    int n = 0;
#define ENTRY(field) table[n++] = (KVEntry){ #field, &s->field }
    ENTRY(sc);
    ENTRY(sc_render);
    ENTRY(sc_render_ibl);
    ENTRY(sc_render_ibl_samples);
    ENTRY(sc_render_texture);
    ENTRY(sc_render_texture_mipmaps);
    ENTRY(sc_render_texture_aniso);
    ENTRY(sc_render_shadows);
    ENTRY(sc_render_shadows_res);
    ENTRY(sc_render_post);
    ENTRY(sc_render_post_bloom);
    ENTRY(sc_render_post_ao);
    ENTRY(sc_audio);
    ENTRY(sc_audio_maxchannels);
#undef ENTRY
    *count = n;
}

bool sc_load(Scalability *s, const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f) return false;

    KVEntry table[32];
    int     table_size = 0;
    build_kv_table(s, table, &table_size);

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        /* Skip comments and blank lines */
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (*p == '#' || *p == ';' || *p == '\n' || *p == '\0') continue;

        /* Parse "key = value" */
        char key[128]; float val;
        if (sscanf(p, "%127s = %f", key, &val) != 2) continue;

        for (int i = 0; i < table_size; i++) {
            if (SDL_strcmp(table[i].key, key) == 0) {
                *table[i].value = val;
                break;
            }
        }
    }
    fclose(f);
    SDL_Log("scalability: loaded '%s'", path);
    return true;
}

/* ── INI writer ───────────────────────────────────────────────────────────── */

bool sc_save(const Scalability *s, const char *path)
{
    FILE *f = fopen(path, "w");
    if (!f) return false;

    fprintf(f,
        "# Scalability settings\n"
        "# Each value in [0..1] unless noted otherwise.\n"
        "# Effective value = node * all ancestors.\n"
        "# Feature flags [0|1]: 0 disables the feature and its subtree.\n"
        "\n"
        "# ── Global ──────────────────────────────────────────────────\n"
        "sc = %.3f\n\n"

        "# ── Render ──────────────────────────────────────────────────\n"
        "sc_render = %.3f\n\n"

        "# IBL – Image-Based Lighting\n"
        "# sc_render_ibl [0|1]: 0 = hemisphere fallback, 1 = full IBL\n"
        "sc_render_ibl = %.3f\n"
        "# sc_render_ibl_samples [0..1]: 0=64 samples, 1=1024 samples\n"
        "sc_render_ibl_samples = %.3f\n\n"

        "# Textures\n"
        "sc_render_texture = %.3f\n"
        "sc_render_texture_mipmaps = %.3f\n"
        "sc_render_texture_aniso = %.3f\n\n"

        "# Shadows (not yet implemented)\n"
        "sc_render_shadows = %.3f\n"
        "sc_render_shadows_res = %.3f\n\n"

        "# Post-processing (not yet implemented)\n"
        "sc_render_post = %.3f\n"
        "sc_render_post_bloom = %.3f\n"
        "sc_render_post_ao = %.3f\n\n"

        "# ── Audio ───────────────────────────────────────────────────\n"
        "sc_audio = %.3f\n"
        "# sc_audio_maxchannels [0..N]: maximum simultaneous channels\n"
        "sc_audio_maxchannels = %.0f\n",

        s->sc,
        s->sc_render,
        s->sc_render_ibl,
        s->sc_render_ibl_samples,
        s->sc_render_texture,
        s->sc_render_texture_mipmaps,
        s->sc_render_texture_aniso,
        s->sc_render_shadows,
        s->sc_render_shadows_res,
        s->sc_render_post,
        s->sc_render_post_bloom,
        s->sc_render_post_ao,
        s->sc_audio,
        s->sc_audio_maxchannels
    );

    fclose(f);
    SDL_Log("scalability: saved '%s'", path);
    return true;
}

/* ── Debug print ──────────────────────────────────────────────────────────── */

void sc_print(const Scalability *s)
{
    SDL_Log("=== Scalability ===");
    SDL_Log("  sc                         = %.3f", s->sc);
    SDL_Log("  sc_render           (eff=%.3f)", sc_eff_render(s));
    SDL_Log("    sc_render_ibl     (eff=%.3f)  flag=%d", sc_eff_ibl(s), sc_flag_ibl(s));
    SDL_Log("    sc_render_ibl_samples      ibl_samples=%d", sc_ibl_samples(s));
    SDL_Log("    sc_render_texture (eff=%.3f)", sc_eff_texture(s));
    SDL_Log("      sc_render_texture_mipmaps (eff=%.3f)", sc_eff_mipmaps(s));
    SDL_Log("      sc_render_texture_aniso   (eff=%.3f)", sc_eff_aniso(s));
    SDL_Log("    sc_render_shadows (eff=%.3f)  flag=%d", sc_eff_shadows(s), sc_flag_shadows(s));
    SDL_Log("    sc_render_post    (eff=%.3f)  flag=%d", sc_eff_post(s), sc_flag_post(s));
    SDL_Log("  sc_audio            (eff=%.3f)  channels=%d",
            sc_eff_audio(s), sc_audio_channels(s));
}
