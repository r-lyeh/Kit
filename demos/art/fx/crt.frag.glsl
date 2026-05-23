#version 450

// ---------------------------------------------------------------------------

// shaders/postfx_crt.glsl  (classic scanline + barrel distortion)
// u[0].x = scanline intensity (0..1, default 0.5)
// u[0].y = barrel distortion  (default 0.05)
// u[0].z = time (seconds, for rolling scanline animation)
// u[0].w = resolution.y (pixel height)

layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;

layout(set=2, binding=0) uniform sampler2D u_tex;

layout(set=3, binding=0) uniform Params {
    vec4 u[8];
};

vec2 barrel(vec2 uv, float k) {
    vec2 p = uv - 0.5;
    float r2 = dot(p, p);
    return uv + p * r2 * k;
}

void main() {
    float scan_intensity = u[0].x > 0.0 ? u[0].x : 0.5;
    float barrel_k       = u[0].y > 0.0 ? u[0].y : 0.05;
    float time           = u[0].z;
    float res_y          = u[0].w > 0.0 ? u[0].w : 480.0;

    vec2 uv = barrel(v_uv, barrel_k);

    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        out_color = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    vec4 c = texture(u_tex, uv);

    // scanlines
    float scan = sin((uv.y * res_y + time * 60.0) * 3.14159265) * 0.5 + 0.5;
    scan = 1.0 - scan_intensity * (1.0 - scan * scan);
    c.rgb *= scan;

    out_color = c;
}
