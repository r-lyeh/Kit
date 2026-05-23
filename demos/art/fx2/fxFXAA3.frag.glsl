// fxFXAA3.glsl — NVIDIA FXAA 3.11 (Quality preset 15) by Timothy Lottes
// Copyright (C) 2010, 2011 NVIDIA Corporation. All Rights Reserved.
// Ported from r-lyeh/v2 demos/fx/fxFXAA3.glsl
// Compile: glslc -fshader-stage=frag fxFXAA3.glsl -o fxFXAA3.spv
//
// u[0].xy = 1.0 / resolution  (update every frame)
// Tip: requires MSAA off for best results.

#version 450
layout(location=0) in  vec2 v_uv;
layout(location=0) out vec4 out_color;
layout(set=2, binding=0) uniform sampler2D u_tex;
layout(set=3, binding=0) uniform Params { vec4 u[8]; };

// FXAA Quality preset 15 step sizes
#define PS 8
#define P0 1.0
#define P1 1.5
#define P2 2.0
#define P3 2.0
#define P4 2.0
#define P5 2.0
#define P6 4.0
#define P7 12.0

#define FxaaTex  sampler2D
#define FxaaTop(t,p)    textureLod(t,p,0.0)
#define FxaaOff(t,p,o,r) textureLod(t,p+(vec2(o)*r),0.0)
#define Luma(rgba) rgba.y  // GREEN_AS_LUMA

vec4 fxaa3(FxaaTex tex, vec2 pos, vec2 rcpFrame,
           float qualSubpix, float qualEdgeThresh, float qualEdgeThreshMin) {
    vec2 posM = pos;
    vec4 rgbyM = FxaaTop(tex, posM);
    float lumaM = Luma(rgbyM);

    float lumaS = Luma(FxaaOff(tex, posM, ivec2( 0, 1), rcpFrame));
    float lumaE = Luma(FxaaOff(tex, posM, ivec2( 1, 0), rcpFrame));
    float lumaN = Luma(FxaaOff(tex, posM, ivec2( 0,-1), rcpFrame));
    float lumaW = Luma(FxaaOff(tex, posM, ivec2(-1, 0), rcpFrame));

    float rangeMax = max(lumaM, max(max(lumaN,lumaS),max(lumaE,lumaW)));
    float rangeMin = min(lumaM, min(min(lumaN,lumaS),min(lumaE,lumaW)));
    float range    = rangeMax - rangeMin;
    if (range < max(qualEdgeThreshMin, rangeMax * qualEdgeThresh)) return rgbyM;

    float lumaNW = Luma(FxaaOff(tex,posM,ivec2(-1,-1),rcpFrame));
    float lumaSE = Luma(FxaaOff(tex,posM,ivec2( 1, 1),rcpFrame));
    float lumaNE = Luma(FxaaOff(tex,posM,ivec2( 1,-1),rcpFrame));
    float lumaSW = Luma(FxaaOff(tex,posM,ivec2(-1, 1),rcpFrame));

    float edgeHorz = abs(-2.0*lumaW + lumaNW+lumaSW) + abs(-2.0*lumaM + lumaN+lumaS)*2.0 + abs(-2.0*lumaE + lumaNE+lumaSE);
    float edgeVert = abs(-2.0*lumaN + lumaNW+lumaNE) + abs(-2.0*lumaM + lumaW+lumaE)*2.0 + abs(-2.0*lumaS + lumaSW+lumaSE);
    bool horzSpan  = edgeHorz >= edgeVert;

    float lengthSign = horzSpan ? rcpFrame.y : rcpFrame.x;
    float lumaN2 = horzSpan ? lumaN : lumaW;
    float lumaS2 = horzSpan ? lumaS : lumaE;
    float gradientN = abs(lumaN2 - lumaM);
    float gradientS = abs(lumaS2 - lumaM);
    if (gradientN < gradientS) { lumaN2 = lumaS2; lengthSign = -lengthSign; }

    vec2 posB = posM;
    if (horzSpan) posB.y += lengthSign * 0.5;
    else          posB.x += lengthSign * 0.5;

    vec2 offNP = horzSpan ? vec2(rcpFrame.x, 0.0) : vec2(0.0, rcpFrame.y);
    vec2 posN  = posB - offNP * P0;
    vec2 posP  = posB + offNP * P0;

    float lumaNN = lumaN2 + lumaM;
    float gradient4 = (gradientN + gradientS) * 0.25;

    float lumaEndN = Luma(FxaaTop(tex, posN)) - lumaNN * 0.5;
    float lumaEndP = Luma(FxaaTop(tex, posP)) - lumaNN * 0.5;
    bool doneN = abs(lumaEndN) >= gradient4;
    bool doneP = abs(lumaEndP) >= gradient4;

    #define STEP(Pn) \
        if (!doneN) posN -= offNP * Pn; \
        if (!doneP) posP += offNP * Pn; \
        if (!doneN || !doneP) { \
            if (!doneN) lumaEndN = Luma(FxaaTop(tex, posN)) - lumaNN*0.5; \
            if (!doneP) lumaEndP = Luma(FxaaTop(tex, posP)) - lumaNN*0.5; \
            doneN = abs(lumaEndN) >= gradient4; \
            doneP = abs(lumaEndP) >= gradient4; \
        }
    STEP(P1) STEP(P2) STEP(P3) STEP(P4) STEP(P5) STEP(P6) STEP(P7)
    #undef STEP

    float dstN = horzSpan ? posM.x - posN.x : posM.y - posN.y;
    float dstP = horzSpan ? posP.x - posM.x : posP.y - posM.y;
    bool  goodN = (lumaEndN < 0.0) != (lumaM < lumaNN * 0.5);
    bool  goodP = (lumaEndP < 0.0) != (lumaM < lumaNN * 0.5);
    float spanLen = dstN + dstP;
    float pixOffset = (min(dstN,dstP) * (-1.0/spanLen)) + 0.5;
    float subpixG   = clamp(abs((lumaNW+lumaNE+lumaSW+lumaSE)*0.25 + (lumaN+lumaS+lumaE+lumaW)*0.5 - lumaM * 2.0) / range, 0.0, 1.0);
    float subpixH   = subpixG * subpixG * qualSubpix;
    float pixFinal  = max((goodN || goodP) ? pixOffset : 0.0, subpixH);

    vec2 posF = posM;
    if (horzSpan) posF.y += pixFinal * lengthSign;
    else          posF.x += pixFinal * lengthSign;
    return vec4(FxaaTop(tex, posF).rgb, rgbyM.a);
}

void main() {
    vec2 rcpFrame = u[0].xy;
    out_color = fxaa3(u_tex, v_uv, rcpFrame, 0.75, 0.166, 0.0625);
}
