#version 450

// Built-in fullscreen triangle vertex shader (SPIR-V, GLSL source below)
// Generates a clip-space triangle covering the screen with no vertex buffer.

layout(location=0) out vec2 v_uv;
void main() {
    vec2 pos[3] = vec2[](vec2(-1,-1), vec2(3,-1), vec2(-1,3));
    gl_Position = vec4(pos[gl_VertexIndex], 0, 1);
    v_uv = pos[gl_VertexIndex] * 0.5 + 0.5;
}
