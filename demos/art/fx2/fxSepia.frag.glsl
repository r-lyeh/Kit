// fxSepia.glsl — Sepia tone
// Ported from r-lyeh/v2 demos/fx/fxSepia.glsl
// Compile: glslc -fshader-stage=frag fxSepia.glsl -o fxSepia.spv
//
// u[0].x = brightness  (default: 1.0)

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

void main() {
    float b   = u[0].x != 0.0 ? u[0].x : 1.0;
    vec4  src = texture(u_tex, v_uv);
    out_color = vec4(
        dot(src.rgb, vec3(0.393*b, 0.769*b, 0.189*b)),
        dot(src.rgb, vec3(0.349*b, 0.686*b, 0.168*b)),
        dot(src.rgb, vec3(0.272*b, 0.534*b, 0.131*b)),
        src.a
    );
}
