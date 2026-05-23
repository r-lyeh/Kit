#version 450
/*
 * brdf_lut.comp
 *
 * Generates the 2D BRDF integration lookup table for the split-sum IBL.
 *
 * U axis = NdotV (0→1), V axis = roughness (0→1).
 * Output: RG16F – R = scale, G = bias for the Fresnel term.
 *
 * The result allows the specular IBL to be computed as:
 *   F0 * brdf.r + brdf.g
 *
 * Dispatched: (lut_size/8, lut_size/8, 1).
 */

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(set = 1, binding = 0, rg16f) uniform writeonly image2D u_lut;

const float PI = 3.14159265359;

float radical_inverse_vdc(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}
vec2 hammersley(uint i, uint N) { return vec2(float(i)/float(N), radical_inverse_vdc(i)); }

vec3 importance_sample_ggx(vec2 xi, vec3 N, float roughness)
{
    float a   = roughness * roughness;
    float phi = 2.0 * PI * xi.x;
    float cos_theta = sqrt((1.0 - xi.y) / (1.0 + (a*a - 1.0) * xi.y));
    float sin_theta = sqrt(1.0 - cos_theta * cos_theta);
    vec3  H   = vec3(cos(phi)*sin_theta, sin(phi)*sin_theta, cos_theta);
    vec3  up  = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3  tx  = normalize(cross(up, N));
    vec3  ty  = cross(N, tx);
    return normalize(tx * H.x + ty * H.y + N * H.z);
}

/* Schlick-GGX for IBL: k = α/2 = roughness²/2 */
float G_schlick_ibl(float NdotV, float roughness)
{
    float a = roughness * roughness;
    float k = a * 0.5;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float G_smith_ibl(float NdotV, float NdotL, float roughness)
{
    return G_schlick_ibl(NdotV, roughness) * G_schlick_ibl(NdotL, roughness);
}

void main()
{
    ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size  = imageSize(u_lut);
    if (coord.x >= size.x || coord.y >= size.y) return;

    /* NdotV on X, roughness on Y – both in (0,1], avoid 0 exactly */
    float NdotV   = (float(coord.x) + 0.5) / float(size.x);
    float roughness = (float(coord.y) + 0.5) / float(size.y);

    vec3 V = vec3(sqrt(1.0 - NdotV*NdotV), 0.0, NdotV);
    vec3 N = vec3(0.0, 0.0, 1.0);

    float scale = 0.0, bias = 0.0;
    const uint SAMPLES = 1024u;

    for (uint i = 0u; i < SAMPLES; i++) {
        vec2  xi = hammersley(i, SAMPLES);
        vec3  H  = importance_sample_ggx(xi, N, roughness);
        vec3  L  = normalize(2.0 * dot(V, H) * H - V);

        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if (NdotL > 0.0) {
            float G     = G_smith_ibl(NdotV, NdotL, roughness);
            float G_vis = (G * VdotH) / (NdotH * NdotV + 1e-7);
            float Fc    = pow(1.0 - VdotH, 5.0);
            scale      += (1.0 - Fc) * G_vis;
            bias       += Fc * G_vis;
        }
    }

    imageStore(u_lut, coord, vec4(scale / float(SAMPLES),
                                   bias  / float(SAMPLES),
                                   0.0, 1.0));
}
