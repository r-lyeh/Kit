// vert.glsl — shared fullscreen triangle vertex shader for all kit.fx passes
// Compile: glslc -fshader-stage=vert vert.glsl -o vert.spv

#version 450

layout(location = 0) out vec2 v_uv;

void main() {
    // Fullscreen triangle: no vertex buffer needed.
    // Vertex 0: (-1,-1)  Vertex 1: (3,-1)  Vertex 2: (-1, 3)
    vec2 pos = vec2(
        float((gl_VertexIndex & 1) << 2) - 1.0,
        float((gl_VertexIndex & 2) << 1) - 1.0
    );
    gl_Position = vec4(pos, 0.0, 1.0);
    v_uv = pos * 0.5 + 0.5;
}
