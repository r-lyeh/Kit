// fxScanlines.glsl — Animated scanlines + flicker
// Ported from r-lyeh/v2 demos/fx/fxScanlines.glsl
// Compile: glslc -fshader-stage=frag fxScanlines.glsl -o fxScanlines.spv
//
// u[0].x = hardness    (default: 0.1, range: 0..2)
// u[0].y = flickering  (default: 0.01)
// u[0].z = time        (seconds, update per frame)

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

void main() {
    float hardness   = u[0].x != 0.0 ? u[0].x : 0.1;
    float flickering = u[0].y != 0.0 ? u[0].y : 0.01;
    float t          = u[0].z;
    vec4  src        = texture(u_tex, v_uv);
    vec3  color      = src.rgb;
    color *= (1.0 - hardness) + hardness * sin(10.0*t + v_uv.y * 1000.0);
    color *= (1.0 - flickering) + flickering * sin(100.0 * t);
    out_color = vec4(color, src.a);
}
