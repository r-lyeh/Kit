#version 450

// shaders/postfx_passthrough.glsl
// Compile: glslangValidator -V postfx_passthrough.glsl -o postfx_passthrough.frag.spv

layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;

layout(set=2, binding=0) uniform sampler2D u_tex;

layout(set=3, binding=0) uniform Params {
    vec4 u[8];
};

void main() {
    out_color = texture(u_tex, v_uv);
}
