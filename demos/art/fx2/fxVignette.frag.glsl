// fxVignette.glsl — Vignette
// Ported from r-lyeh/v2 demos/fx/fxVignette.glsl
// Compile: glslc -fshader-stage=frag fxVignette.glsl -o fxVignette.spv
//
// u[0].x = radius  (default: 0.75; controls falloff)

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

void main() {
    float radius = u[0].x != 0.0 ? u[0].x : 0.75;
    vec4  src    = texture(u_tex, v_uv);
    float vig    = (1.0-radius) + radius * 16.0 * v_uv.x*v_uv.y*(1.0-v_uv.x)*(1.0-v_uv.y);
    out_color = vec4(src.rgb * vig, src.a);
}
