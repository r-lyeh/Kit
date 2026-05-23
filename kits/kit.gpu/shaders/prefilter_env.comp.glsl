#version 450
/*
 * prefilter_env.comp  –  one dispatch per face per mip
 *
 * Uniform (set 2, binding 0): vec4 {roughness, env_resolution, face_index, _pad}
 * Output:  image2D u_face  (single face/mip, bound per-dispatch)
 */

layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout(set = 0, binding = 0)          uniform samplerCube u_env;
layout(set = 1, binding = 0, rgba16f) uniform writeonly image2D u_face;

layout(set = 2, binding = 0) uniform Params {
    float roughness;
    float env_resolution;
    float face_index_f;
    float sample_count_f;  /* cast of uint sample count: 64..1024 */
};

const float PI = 3.14159265359;

float radical_inverse_vdc(uint bits) {
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10;
}
vec2 hammersley(uint i, uint N) { return vec2(float(i)/float(N), radical_inverse_vdc(i)); }

vec3 importance_sample_ggx(vec2 xi, vec3 N, float rough) {
    float a = rough*rough, phi = 2.0*PI*xi.x;
    float ct = sqrt((1.0-xi.y)/max(1.0+(a*a-1.0)*xi.y, 1e-7));
    float st = sqrt(1.0-ct*ct);
    vec3  H  = vec3(cos(phi)*st, sin(phi)*st, ct);
    vec3  up = abs(N.y)<0.999 ? vec3(0,1,0) : vec3(1,0,0);
    vec3  tx = normalize(cross(up,N)), ty = cross(N,tx);
    return normalize(tx*H.x + ty*H.y + N*H.z);
}

vec3 cube_dir(uint face, vec2 uv) {
    switch (face) {
    case 0: return normalize(vec3( 1.0,  uv.y, -uv.x));
    case 1: return normalize(vec3(-1.0,  uv.y,  uv.x));
    case 2: return normalize(vec3( uv.x,  1.0, -uv.y));
    case 3: return normalize(vec3( uv.x, -1.0,  uv.y));
    case 4: return normalize(vec3( uv.x,  uv.y,  1.0));
    default:return normalize(vec3(-uv.x,  uv.y, -1.0));
    }
}

void main()
{
    ivec2 coord = ivec2(gl_GlobalInvocationID.xy);
    ivec2 size  = imageSize(u_face);
    if (coord.x >= size.x || coord.y >= size.y) return;

    uint face    = uint(face_index_f);
    vec2 uv   = (vec2(coord) + 0.5) / vec2(size) * 2.0 - 1.0;
    uv.y     = -uv.y;
    vec3 N    = cube_dir(face, uv);
    vec3 V    = N;

    const uint SAMPLES = uint(max(sample_count_f, 1.0));
    float total = 0.0;
    vec3  pf    = vec3(0.0);
    float alpha = roughness * roughness;

    for (uint i = 0u; i < SAMPLES; i++) {
        vec2 xi = hammersley(i, SAMPLES);
        vec3 H  = importance_sample_ggx(xi, N, roughness);
        vec3 L  = normalize(2.0*dot(V,H)*H - V);

        float NdotL = max(dot(N,L), 0.0);
        if (NdotL > 0.0) {
            float NdotH = max(dot(N,H), 0.0), VdotH = max(dot(V,H), 0.0);
            float D   = alpha*alpha / (PI*pow(NdotH*NdotH*(alpha*alpha-1.0)+1.0, 2.0));
            float pdf = D*NdotH / (4.0*VdotH + 1e-4);
            float sa_texel  = 4.0*PI / (6.0*env_resolution*env_resolution);
            float sa_sample = 1.0 / (float(SAMPLES)*pdf + 1e-4);
            float mip = roughness==0.0 ? 0.0 : 0.5*log2(sa_sample/sa_texel);
            pf    += textureLod(u_env, L, mip).rgb * NdotL;
            total += NdotL;
        }
    }
    pf /= max(total, 1e-4);

    imageStore(u_face, coord, vec4(pf, 1.0));
}
