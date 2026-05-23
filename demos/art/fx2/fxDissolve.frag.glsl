// fxDissolve.glsl — Random pixel displacement / dissolve
// Ported from r-lyeh/v2 demos/fx/fxDissolve.glsl
// Compile: glslc -fshader-stage=frag fxDissolve.glsl -o fxDissolve.spv
//
// u[0].x = intensity  (default: 0.004, range: 0..0.03)

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main() {
    float intensity = u[0].x != 0.0 ? u[0].x : 0.004;
    out_color = texture(u_tex, v_uv + intensity * rand(v_uv));
}
