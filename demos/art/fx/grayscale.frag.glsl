#version 450

// shaders/postfx_grayscale.glsl
// u[0].x = mix factor (0=color, 1=gray)

layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;

layout(set=2, binding=0) uniform sampler2D u_tex;

layout(set=3, binding=0) uniform Params {
    vec4 u[8];
};

void main() {
    vec4 c = texture(u_tex, v_uv);
    float luma = dot(c.rgb, vec3(0.2126, 0.7152, 0.0722));
    out_color = vec4(mix(c.rgb, vec3(luma), u[0].x), c.a);
}
