#version 450

// ---------------------------------------------------------------------------

// shaders/postfx_vhs.glsl  (inspired by FMS_Cat, MIT licensed)
// u[0].x = time (seconds)
// u[0].y = noise strength (default 0.05)
// u[0].z = jitter strength (default 0.005)
// u[0].w = color bleed (default 0.003)

layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;

layout(set=2, binding=0) uniform sampler2D u_tex;

layout(set=3, binding=0) uniform Params {
    vec4 u[8];
};

float hash(float n) { return fract(sin(n) * 43758.5453123); }

void main() {
    float t       = u[0].x;
    float noise_s = u[0].y > 0.0 ? u[0].y : 0.05;
    float jitter  = u[0].z > 0.0 ? u[0].z : 0.005;
    float bleed   = u[0].w > 0.0 ? u[0].w : 0.003;

    vec2 uv = v_uv;

    // Horizontal jitter
    float line_hash = hash(floor(uv.y * 240.0) + t * 0.1);
    uv.x += (line_hash - 0.5) * jitter * hash(t * 13.7 + uv.y * 300.0);

    // Color bleed (chromatic aberration-ish)
    float r = texture(u_tex, uv + vec2(bleed, 0.0)).r;
    float g = texture(u_tex, uv).g;
    float b = texture(u_tex, uv - vec2(bleed, 0.0)).b;

    vec3 col = vec3(r, g, b);

    // Noise
    float n = hash(uv.x * 1234.5 + uv.y * 6789.0 + t * 0.3);
    col += (n - 0.5) * noise_s;

    // Slight luma compression (VHS blacks aren't deep)
    col = clamp(col * 0.9 + 0.03, 0.0, 1.0);

    out_color = vec4(col, 1.0);
}
