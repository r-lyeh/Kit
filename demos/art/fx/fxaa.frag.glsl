#version 450

// ---------------------------------------------------------------------------

// shaders/postfx_fxaa.glsl  (Simplified FXAA 3.11 — good enough for 1080p)
// No uniforms needed.

layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;

layout(set=2, binding=0) uniform sampler2D u_tex;

layout(set=3, binding=0) uniform Params {
    vec4 u[8]; // u[0].xy = 1.0 / resolution
};

#define FXAA_SPAN_MAX   8.0
#define FXAA_REDUCE_MUL (1.0/8.0)
#define FXAA_REDUCE_MIN (1.0/128.0)

void main() {
    vec2 inv_res = u[0].xy; // set externally each frame

    vec3 rgb_nw = texture(u_tex, v_uv + vec2(-1,-1) * inv_res).rgb;
    vec3 rgb_ne = texture(u_tex, v_uv + vec2( 1,-1) * inv_res).rgb;
    vec3 rgb_sw = texture(u_tex, v_uv + vec2(-1, 1) * inv_res).rgb;
    vec3 rgb_se = texture(u_tex, v_uv + vec2( 1, 1) * inv_res).rgb;
    vec3 rgb_m  = texture(u_tex, v_uv).rgb;

    vec3 luma = vec3(0.299, 0.587, 0.114);
    float luma_nw = dot(rgb_nw, luma);
    float luma_ne = dot(rgb_ne, luma);
    float luma_sw = dot(rgb_sw, luma);
    float luma_se = dot(rgb_se, luma);
    float luma_m  = dot(rgb_m,  luma);

    float luma_min = min(luma_m, min(min(luma_nw, luma_ne), min(luma_sw, luma_se)));
    float luma_max = max(luma_m, max(max(luma_nw, luma_ne), max(luma_sw, luma_se)));

    vec2 dir = vec2(
        -((luma_nw + luma_ne) - (luma_sw + luma_se)),
         ((luma_nw + luma_sw) - (luma_ne + luma_se))
    );

    float dir_reduce = max(
        (luma_nw + luma_ne + luma_sw + luma_se) * (0.25 * FXAA_REDUCE_MUL),
        FXAA_REDUCE_MIN);
    float rcp = 1.0 / (min(abs(dir.x), abs(dir.y)) + dir_reduce);
    dir = clamp(dir * rcp, -FXAA_SPAN_MAX, FXAA_SPAN_MAX) * inv_res;

    vec3 a = 0.5 * (
        texture(u_tex, v_uv + dir * (1.0/3.0 - 0.5)).rgb +
        texture(u_tex, v_uv + dir * (2.0/3.0 - 0.5)).rgb);
    vec3 b = a * 0.5 + 0.25 * (
        texture(u_tex, v_uv + dir * -0.5).rgb +
        texture(u_tex, v_uv + dir *  0.5).rgb);

    float luma_b = dot(b, luma);
    out_color = vec4((luma_b < luma_min || luma_b > luma_max) ? a : b, 1.0);
}
