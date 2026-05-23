// fxBloom.glsl — Simple brightness boost / bloom pass
// Ported from r-lyeh/v2 demos/fx/fxBloom.glsl
// Compile: glslc -fshader-stage=frag fxBloom.glsl -o fxBloom.spv
//
// u[0].x = intensity  (default: 2.0)

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

void main() {
    float intensity = u[0].x != 0.0 ? u[0].x : 2.0;
    vec4 src  = texture(u_tex, v_uv);
    out_color = vec4(clamp(src.rgb * intensity, 0.0, 1.0), src.a);
}
