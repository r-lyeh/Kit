// fxDithering.glsl — Ordered (Bayer) dithering, 4x4 matrix
// Ported from r-lyeh/v2 demos/fx/fxDithering.glsl
// Compile: glslc -fshader-stage=frag fxDithering.glsl -o fxDithering.spv
//
// No uniforms — uses 4x4 Bayer matrix.

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

const float threshold[16] = float[16](
     1.0/16.0,  9.0/16.0,  3.0/16.0, 11.0/16.0,
    13.0/16.0,  5.0/16.0, 15.0/16.0,  7.0/16.0,
     4.0/16.0, 12.0/16.0,  2.0/16.0, 10.0/16.0,
    16.0/16.0,  8.0/16.0, 14.0/16.0,  6.0/16.0
);

void main() {
    vec4  src  = texture(u_tex, v_uv);
    ivec2 fc   = ivec2(gl_FragCoord.xy) % 4;
    float luma = dot(vec3(0.2126, 0.7152, 0.0722), src.rgb);
    luma       = step(threshold[fc.y * 4 + fc.x], luma);
    out_color  = vec4(vec3(luma), src.a);
}
