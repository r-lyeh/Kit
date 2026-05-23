// fxFXAA.glsl — FXAA (Fast Approximate Anti-Aliasing) by Timothy Lottes
// Ported from r-lyeh/v2 demos/fx/fxFXAA.glsl
// Compile: glslc -fshader-stage=frag fxFXAA.glsl -o fxFXAA.spv
//
// u[0].xy = 1.0 / resolution  (update every frame)
// Tip: requires MSAA off for best results.

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

#define FXAA_REDUCE_MIN   (1.0/128.0)
#define FXAA_REDUCE_MUL   (1.0/8.0)
#define FXAA_SPAN_MAX     8.0
#define FXAA_SUBPIX_SHIFT (1.0/4.0)

float luma(vec3 c) { return dot(c, vec3(0.2126729, 0.7151522, 0.0721750)); }

vec4 fxaa(sampler2D tex, vec2 uv, vec2 rcpFrame) {
    vec4 posPos = vec4(uv, uv - rcpFrame * (0.5 + FXAA_SUBPIX_SHIFT));

    vec3 rgbNW = textureLod(tex, posPos.zw,                          0.0).rgb;
    vec3 rgbNE = textureLod(tex, posPos.zw + vec2(1.0, 0.0)*rcpFrame, 0.0).rgb;
    vec3 rgbSW = textureLod(tex, posPos.zw + vec2(0.0, 1.0)*rcpFrame, 0.0).rgb;
    vec3 rgbSE = textureLod(tex, posPos.zw + vec2(1.0, 1.0)*rcpFrame, 0.0).rgb;
    vec4 rgbM  = textureLod(tex, posPos.xy,                           0.0);

    float lumaNW = luma(rgbNW), lumaNE = luma(rgbNE);
    float lumaSW = luma(rgbSW), lumaSE = luma(rgbSE);
    float lumaM  = luma(rgbM.rgb);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max((lumaNW+lumaNE+lumaSW+lumaSE) * (0.25*FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
    float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
    dir = clamp(dir * rcpDirMin, -FXAA_SPAN_MAX, FXAA_SPAN_MAX) * rcpFrame;

    vec3 rgbA = 0.5 * (
        textureLod(tex, posPos.xy + dir*(1.0/3.0-0.5), 0.0).rgb +
        textureLod(tex, posPos.xy + dir*(2.0/3.0-0.5), 0.0).rgb);
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        textureLod(tex, posPos.xy + dir*(-0.5), 0.0).rgb +
        textureLod(tex, posPos.xy + dir*( 0.5), 0.0).rgb);

    float lumaB = luma(rgbB);
    return vec4((lumaB < lumaMin || lumaB > lumaMax) ? rgbA : rgbB, rgbM.a);
}

void main() {
    vec2 rcpFrame = u[0].xy; // must be set to 1/resolution each frame
    out_color = fxaa(u_tex, v_uv, rcpFrame);
}
