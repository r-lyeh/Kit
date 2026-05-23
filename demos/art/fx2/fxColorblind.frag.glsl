// fxColorblind.glsl — Colorblind simulation
// Ref: https://www.inf.ufrgs.br/~oliveira/pubs_files/CVD_Simulation/CVD_Simulation.html
// Ported from r-lyeh/v2 demos/fx/fxColorblind.glsl
// Compile: glslc -fshader-stage=frag fxColorblind.glsl -o fxColorblind.spv
//
// u[0].x = mode (int 0..4)
//   0 = normal, 1 = achromatopsia, 2 = protanomaly, 3 = deuteranomaly, 4 = tritanomaly

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

const mat3 MATRICES[5] = mat3[5](
    mat3(1.000, 0.000, 0.000,  0.000, 1.000, 0.000,  0.000, 0.000, 1.000), // 0 normal
    mat3(0.299, 0.587, 0.114,  0.299, 0.587, 0.114,  0.299, 0.587, 0.114), // 1 achromatopsia
    mat3( 0.152286, 1.052583,-0.204868,  0.114503, 0.786281, 0.099216, -0.003882,-0.048116, 1.051998), // 2 protanomaly
    mat3( 0.367322, 0.860646,-0.227968,  0.280085, 0.672501, 0.047413, -0.011820, 0.042940, 0.968881), // 3 deuteranomaly
    mat3( 1.255528,-0.076749,-0.178779, -0.078411, 0.930809, 0.147602,  0.004733, 0.691367, 0.303900)  // 4 tritanomaly
);

void main() {
    int  mode = int(u[0].x);
    vec4 src  = texture(u_tex, v_uv);
    out_color = vec4(MATRICES[clamp(mode, 0, 4)] * src.rgb, src.a);
}
