#version 450
/*
 * mesh.frag.glsl  –  glTF 2.0 PBR metallic-roughness (spec-clean v2)
 *
 * Spec reference:
 *   https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#appendix-b-brdf-implementation
 *
 * BRDF formulas are verbatim from the spec.
 * Non-spec additions are clearly labelled [NON-SPEC].
 *
 * Fixes vs v8:
 *   [FIX-A] Ambient specular was using hemi_col for both diffuse and specular,
 *           and spec_env=0.5 for smooth surfaces was washing dark albedos grey.
 *           Specular ambient now uses reflection direction for hemisphere tint
 *           and is scaled by (1-alpha) so smooth surfaces get crisp env colour
 *           while rough surfaces get diffuse-like ambient. Still approximate
 *           but no longer washes out dark materials.
 *   [FIX-B] AMBIENT_MIN floor removed from per-channel computation – it was
 *           lifting dark metallic surfaces incorrectly. Floor is now only a
 *           safety net against negative values (max with 0).
 *   [FIX-C] Key light intensity reduced from 2.8 to 2.0; the strong key was
 *           overexposing the dome and blowing out the specular highlight.
 */

layout(location = 0) in vec3  v_world_pos;
layout(location = 1) in vec2  v_texcoord;
layout(location = 2) in vec3  v_T;
layout(location = 3) in vec3  v_B;
layout(location = 4) in vec3  v_N;
layout(location = 5) in float v_tangent_w;

layout(set = 2, binding = 0) uniform sampler2D   u_albedo;
layout(set = 2, binding = 1) uniform sampler2D   u_metallic_roughness;
layout(set = 2, binding = 2) uniform sampler2D   u_normal_map;
layout(set = 2, binding = 3) uniform sampler2D   u_occlusion;
layout(set = 2, binding = 4) uniform sampler2D   u_emissive;
/* IBL – bound once per frame by app.c */
layout(set = 2, binding = 5) uniform samplerCube u_irradiance;   /* diffuse  */
layout(set = 2, binding = 6) uniform samplerCube u_prefilter;    /* specular */
layout(set = 2, binding = 7) uniform sampler2D   u_brdf_lut;    /* scale/bias */

/* std140 layout – offsets must match MaterialUBO in mesh.c exactly:
 *   0   base_color[4]      16 bytes
 *  16   emissive_factor    12 bytes  (vec3 → 16-byte slot, 4 bytes pad after)
 *  32   metallic_factor     4 bytes
 *  36   roughness_factor    4 bytes
 *  40   normal_scale        4 bytes
 *  44   occlusion_strength  4 bytes
 *  48   alpha_cutoff        4 bytes
 *  52   normal_y_sign       4 bytes
 *  56   _pad[2]             8 bytes
 *  Total: 64 bytes                                                          */
/* MaterialUBO – all vec3s promoted to vec4 to eliminate std140 padding ambiguity.
 * Exact C-side layout in mesh.c must match field-for-field.
 * Offsets:
 *   0  base_color         vec4  16
 *  16  emissive_factor    vec4  16  (w = normal_y_sign)
 *  32  metallic_factor    float  4
 *  36  roughness_factor   float  4
 *  40  normal_scale       float  4
 *  44  occlusion_strength float  4
 *  48  alpha_cutoff       float  4
 *  52  mr_swizzle         float  4
 *  56  _pad[2]            float  8
 *  Total: 64 bytes                  */
layout(set = 3, binding = 0) uniform MaterialUBO {
    vec4  base_color;
    vec4  emissive_normal_y;   /* rgb=emissive_factor, a=normal_y_sign */
    float metallic_factor;
    float roughness_factor;
    float normal_scale;
    float occlusion_strength;
    float alpha_cutoff;
    float mr_swizzle;
    float _pad[2];
};
/* Convenience accessors */
#define emissive_factor  emissive_normal_y.rgb
#define normal_y_sign    emissive_normal_y.a

layout(set = 3, binding = 1) uniform CameraUBO {
    vec4 cam_pos_w;
};

/* IBL enable flag – pushed per-frame from app.c via scalability system.
 * 1.0 = full IBL active, 0.0 = hemisphere fallback.                       */
/* ScalabilityUBO: vec4 (16 bytes, std140). x=sc_ibl_enabled, yzw unused. */
layout(set = 3, binding = 2) uniform ScalabilityUBO {
    vec4 sc_params;   /* x = ibl_enabled (0=hemisphere, 1=IBL) */
};
#define sc_ibl_enabled sc_params.x

layout(location = 0) out vec4 frag_color;

const float PI = 3.14159265359;
const float MAX_REFLECTION_LOD = 4.0; /* IBL_PREFILTER_MIPS - 1 */

/* ── [NON-SPEC] Studio light rig ─────────────────────────────────────────────*/
const vec3 KEY_DIR    = normalize(vec3( 1.0,  2.0,  1.5));
const vec3 KEY_COLOR  = vec3(2.0, 1.9, 1.7);   /* reduced – was blowing out dome */

const vec3 FILL_DIR   = normalize(vec3(-1.5,  0.8,  0.5));
const vec3 FILL_COLOR = vec3(0.5, 0.6, 0.9);

const vec3 RIM_DIR    = normalize(vec3( 0.2,  0.4, -2.0));
const vec3 RIM_COLOR  = vec3(0.9, 0.7, 0.4);

const vec3 BOT_DIR    = normalize(vec3( 0.0, -1.0,  0.3));
const vec3 BOT_COLOR  = vec3(0.25, 0.22, 0.30);


/* ── Colour space ────────────────────────────────────────────────────────────*/
vec3 srgb_to_linear(vec3 c) { return pow(max(c, vec3(0.0)), vec3(2.2)); }

/* ── [NON-SPEC] ACES filmic tonemap (Narkowicz 2015) ─────────────────────────*/
vec3 aces_filmic(vec3 x) {
    const float a=2.51, b=0.03, c=2.43, d=0.59, e=0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

/* ── glTF spec appendix B: BRDF ─────────────────────────────────────────────
 *
 * Notation: r = perceptual roughness (from texture × factor)
 *           α = r²  (linear roughness used in BRDF formulas)
 *
 * D_GGX receives alpha = r², computes a2 = α² = r⁴.
 * This is correct: spec says D = α² / (π·((NdotH²·(α²-1)+1)²))
 * where the numerator α² means r⁴.                                          */

float D_GGX(float NdotH, float alpha) {
    float a2    = alpha * alpha;
    float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;
    return a2 / max(PI * denom * denom, 1e-7);
}

/* G1 for direct lights: k = α/2  (Karis 2013, different from IBL k=(r+1)²/8) */
float G1_direct(float NdotX, float alpha) {
    float k = alpha * 0.5;
    return NdotX / max(NdotX * (1.0 - k) + k, 1e-7);
}

float G_Smith(float NdotV, float NdotL, float alpha) {
    return G1_direct(NdotV, alpha) * G1_direct(NdotL, alpha);
}

vec3 F_Schlick(float VdotH, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - VdotH, 0.0, 1.0), 5.0);
}

/* Schlick-roughness: ambient Fresnel with roughness dampening */
vec3 F_SchlickR(float NdotV, vec3 F0, float perceptual_rough) {
    return F0 + (max(vec3(1.0 - perceptual_rough), F0) - F0)
              * pow(clamp(1.0 - NdotV, 0.0, 1.0), 5.0);
}

/* One directional light – returns Lo contribution */
vec3 eval_direct(vec3 L, vec3 light_color,
                 vec3 N,  vec3 V,
                 vec3 albedo, float metallic, float alpha, vec3 F0)
{
    float NdotL = max(dot(N, L), 0.0);
    if (NdotL < 1e-4) return vec3(0.0);

    vec3  H     = normalize(V + L);
    float NdotV = max(dot(N, V), 1e-4);
    float NdotH = max(dot(N, H), 0.0);
    float VdotH = max(dot(V, H), 0.0);

    float D    = D_GGX(NdotH, alpha);
    float G    = G_Smith(NdotV, NdotL, alpha);
    vec3  F    = F_Schlick(VdotH, F0);

    vec3  spec = (D * G * F) / max(4.0 * NdotV * NdotL, 1e-4);
    vec3  kD   = (1.0 - F) * (1.0 - metallic);

    return (kD * albedo / PI + spec) * light_color * NdotL;
}

/* ── [NON-SPEC] Screen-space cotangent frame ─────────────────────────────────
 * Fallback for non-indexed geometry without TANGENT attribute.             */
mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv)
{
    vec3 dp1=dFdx(p), dp2=dFdy(p);
    vec2 duv1=dFdx(uv), duv2=dFdy(uv);

    float det     = duv1.x * duv2.y - duv1.y * duv2.x;
    float inv_det = (abs(det) > 1e-6) ? (1.0 / det) : 1.0;

    vec3 T = ( duv2.y * dp1 - duv1.y * dp2) * inv_det;
    vec3 B = (-duv2.x * dp1 + duv1.x * dp2) * inv_det;
    T = normalize(T - dot(T, N) * N);
    B = normalize(B - dot(B, N) * N - dot(B, T) * T);
    return mat3(T, B, N);
}

/* ── Main ────────────────────────────────────────────────────────────────────*/
void main()
{
    /* Albedo – texture is sRGB, factor is linear */
    vec4  albedo_samp = texture(u_albedo, v_texcoord);
    vec3  albedo      = srgb_to_linear(albedo_samp.rgb) * base_color.rgb;
    float alpha_val   = albedo_samp.a * base_color.a;
    if (alpha_cutoff > 0.0 && alpha_val < alpha_cutoff) discard;

    /* Metallic / roughness – linear texture.
     * glTF spec: G=roughness, B=metallic.
     * Legacy assets (mr_swizzle < 0): R=metallic, G=roughness, B=unused. */
    vec4  mr              = texture(u_metallic_roughness, v_texcoord);
    float perceptual_rough, metallic;
    if (mr_swizzle > 0.0) {
        /* Spec-correct: B=metallic, G=roughness */
        perceptual_rough = clamp(mr.g * roughness_factor, 0.045, 1.0);
        metallic         = clamp(mr.b * metallic_factor,  0.0,   1.0);
    } else {
        /* Legacy (e.g. DamagedHelmet original): R=metallic, G=roughness */
        perceptual_rough = clamp(mr.g * roughness_factor, 0.045, 1.0);
        metallic         = clamp(mr.r * metallic_factor,  0.0,   1.0);
    }
    float alpha = perceptual_rough * perceptual_rough;  /* α = r² */

    /* Occlusion – ambient-only per glTF spec §5.22.3 */
    float ao = texture(u_occlusion, v_texcoord).r;

    /* TBN */
    vec3 Nv = normalize(v_N);
    mat3 TBN = (abs(v_tangent_w) > 0.5)
        ? mat3(normalize(v_T), normalize(v_B), Nv)
        : cotangent_frame(Nv, v_world_pos, v_texcoord);

    /* Normal map – decode RG, apply scale, reconstruct Z */
    vec2  nm_rg = texture(u_normal_map, v_texcoord).rg * 2.0 - 1.0;
    vec2  nm_xy = vec2(nm_rg.x, nm_rg.y * normal_y_sign) * normal_scale;
    float nm_z  = sqrt(max(1.0 - dot(nm_xy, nm_xy), 0.0));
    vec3  N     = normalize(TBN * vec3(nm_xy, nm_z));

    /* Emissive – sRGB texture × linear factor (no extra boost) */
    vec3 emissive = srgb_to_linear(texture(u_emissive, v_texcoord).rgb)
                    * emissive_factor;

    /* View vector and F0 */
    vec3  V     = normalize(cam_pos_w.xyz - v_world_pos);
    float NdotV = max(dot(N, V), 1e-4);
    vec3  F0    = mix(vec3(0.04), albedo, metallic);

    /* Direct lighting (AO not applied – ambient occlusion only) */
    vec3 Lo = vec3(0.0);
    Lo += eval_direct(KEY_DIR,  KEY_COLOR,  N, V, albedo, metallic, alpha, F0);
    Lo += eval_direct(FILL_DIR, FILL_COLOR, N, V, albedo, metallic, alpha, F0);
    Lo += eval_direct(RIM_DIR,  RIM_COLOR,  N, V, albedo, metallic, alpha, F0);
    Lo += eval_direct(BOT_DIR,  BOT_COLOR,  N, V, albedo, metallic, alpha, F0);

    /* ── Indirect lighting – dual path ──────────────────────────────────────
     *
     * Path A (sc_ibl_enabled > 0.5): full IBL split-sum (Karis 2013)
     *   indirect_diffuse  = irradiance(N) × albedo × kD
     *   indirect_specular = prefilter(R, roughness) × (F0×brdf.r + brdf.g)
     *
     * Path B (sc_ibl_enabled < 0.5): hemisphere gradient fallback
     *   Cheap approximation: lerp sky/ground colours by N.y.
     *   No env map needed; always available.
     *
     * Both paths apply AO (glTF spec §5.22.3).                             */
    vec3  R      = reflect(-V, N);
    vec3  F_amb  = F_SchlickR(NdotV, F0, perceptual_rough);
    vec3  kD_amb = (1.0 - F_amb) * (1.0 - metallic);
    vec3  ambient;

    if (sc_ibl_enabled > 0.5) {
        /* ── Path A: IBL ─────────────────────────────────────────── */
        vec3 irradiance   = texture(u_irradiance, N).rgb;
        vec3 prefiltered  = textureLod(u_prefilter, R,
                                        perceptual_rough * MAX_REFLECTION_LOD).rgb;
        vec2 brdf         = texture(u_brdf_lut, vec2(NdotV, perceptual_rough)).rg;

        vec3 ind_diff     = kD_amb * albedo * irradiance;
        vec3 ind_spec     = prefiltered * (F_amb * brdf.r + brdf.g);

        ambient = (ind_diff + ind_spec) * mix(1.0, ao, occlusion_strength);

    } else {
        /* ── Path B: hemisphere gradient fallback ────────────────── */
        const vec3 SKY_COLOR    = vec3(0.20, 0.24, 0.36);
        const vec3 GROUND_COLOR = vec3(0.18, 0.16, 0.14);

        float hemi_diff   = clamp(N.y * 0.5 + 0.5, 0.05, 1.0);
        vec3  diff_env    = mix(GROUND_COLOR, SKY_COLOR, hemi_diff);

        float hemi_spec   = clamp(R.y * 0.5 + 0.5, 0.05, 1.0);
        vec3  spec_env    = mix(GROUND_COLOR, SKY_COLOR, hemi_spec);
        float spec_lobe   = (1.0 - perceptual_rough * perceptual_rough);
        spec_lobe        *= spec_lobe;

        vec3 ind_diff     = kD_amb * albedo * diff_env;
        vec3 ind_spec     = F_amb * spec_lobe * spec_env;

        ambient = (ind_diff + ind_spec) * mix(1.0, ao, occlusion_strength);
    }

    ambient = max(ambient, vec3(0.0));

    /* [NON-SPEC] Fresnel rim – silhouette edge brightening */
    float rim     = pow(1.0 - NdotV, 4.0) * (1.0 - perceptual_rough) * 0.25;
    vec3  rim_col = rim * mix(vec3(0.1, 0.12, 0.18), albedo, metallic);

    /* Combine */
    vec3 color = ambient + Lo + emissive + rim_col;

    /* [NON-SPEC] ACES + gamma */
    color = aces_filmic(color);
    color = pow(color, vec3(1.0 / 2.2));

    /* Debug visualisations (uncomment one):
     * color = N * 0.5 + 0.5;           // world-space normals
     * color = vec3(ao);                 // occlusion
     * color = vec3(perceptual_rough);   // roughness
     * color = vec3(metallic);           // metallic
     * color = albedo;                   // base color (linear) */

    frag_color = vec4(color, alpha_val);
}
