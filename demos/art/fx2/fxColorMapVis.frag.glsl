// fxColorMapVis.glsl — Scientific colormap visualization (matplotlib colormaps)
// License: CC0 (public domain) — polynomial fits by mattz
// Ported from r-lyeh/v2 demos/fx/fxColorMapVis.glsl
// Compile: glslc -fshader-stage=frag fxColorMapVis.glsl -o fxColorMapVis.spv
//
// u[0].x = colormap int (0=off, 1=inferno, 2=viridis, 3=plasma, 4=magma)

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

vec3 inferno(float t) {
    const vec3 c0=vec3(0.00021894,0.00165100,-0.01948090);
    const vec3 c1=vec3(0.10651342,0.56395644,3.93271239);
    const vec3 c2=vec3(11.6024931,-3.97285397,-15.9423941);
    const vec3 c3=vec3(-41.7039961,17.4363989,44.3541452);
    const vec3 c4=vec3(77.1629357,-33.4023589,-81.8073093);
    const vec3 c5=vec3(-71.3194282,32.6260643,73.2095199);
    const vec3 c6=vec3(25.1311262,-12.2426690,-23.0703250);
    return c0+t*(c1+t*(c2+t*(c3+t*(c4+t*(c5+t*c6)))));
}
vec3 viridis(float t) {
    const vec3 c0=vec3(0.27772733,0.00540734,0.33409981);
    const vec3 c1=vec3(0.10509304,1.40461353,1.38459016);
    const vec3 c2=vec3(-0.33086183,0.21484756,0.09509516);
    const vec3 c3=vec3(-4.63423050,-5.79910097,-19.3324410);
    const vec3 c4=vec3(6.22826994,14.1799334,56.6905526);
    const vec3 c5=vec3(4.77638500,-13.7451454,-65.3530326);
    const vec3 c6=vec3(-5.43545586,4.64585261,26.3124352);
    return c0+t*(c1+t*(c2+t*(c3+t*(c4+t*(c5+t*c6)))));
}
vec3 plasma(float t) {
    const vec3 c0=vec3(0.05873234,0.02333670,0.54334018);
    const vec3 c1=vec3(2.17651463,0.23838342,0.75396046);
    const vec3 c2=vec3(-2.68946048,-7.45585114,3.11079994);
    const vec3 c3=vec3(6.13034835,42.3461881,-28.5188547);
    const vec3 c4=vec3(-11.1074362,-82.6663111,60.1398477);
    const vec3 c5=vec3(10.0230656,71.4136177,-54.0721866);
    const vec3 c6=vec3(-3.65871384,-22.9315347,18.1919078);
    return c0+t*(c1+t*(c2+t*(c3+t*(c4+t*(c5+t*c6)))));
}
vec3 magma(float t) {
    const vec3 c0=vec3(-0.00213649,-0.00074966,-0.00538613);
    const vec3 c1=vec3(0.25166054,0.67752324,2.49402660);
    const vec3 c2=vec3(8.35371728,-3.57771951,0.31446790);
    const vec3 c3=vec3(-27.6687331,14.2647308,-13.6492132);
    const vec3 c4=vec3(52.1761398,-27.9436061,12.9441694);
    const vec3 c5=vec3(-50.7685254,29.0465828,4.23415300);
    const vec3 c6=vec3(18.6557051,-11.4897735,-5.60196151);
    return c0+t*(c1+t*(c2+t*(c3+t*(c4+t*(c5+t*c6)))));
}

void main() {
    int  mode = int(u[0].x);
    vec4 src  = texture(u_tex, v_uv);
    if (mode == 0) { out_color = src; return; }
    float luma = dot(src.rgb, vec3(0.299, 0.587, 0.114));
    luma = clamp(luma, 0.0, 1.0);
    if (mode == 1) { out_color = vec4(inferno(luma), src.a); return; }
    if (mode == 2) { out_color = vec4(viridis(luma), src.a); return; }
    if (mode == 3) { out_color = vec4(plasma(luma),  src.a); return; }
                    out_color = vec4(magma(luma),   src.a);
}
