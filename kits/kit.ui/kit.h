#include "hey_imgui.h"

#include "kit.fonts.h"
#include "kit.theme.h"


#if KIT_CODE
#pragma once

#ifndef UI2_GLUE
#define UI2_GLUE

#ifndef API
#define API
#endif

#define vec2 float2
#define vec3 float3
#define vec4 float4

#define REALLOC realloc
#define FREE(p) REALLOC(p,0)

#define countof COUNTOF

#define key_down(x) 0
#define key_held(x) 0
#define mouse(b) 0

#ifndef IVEC_H
#define IVEC_H
typedef struct ivec2 { int x,y; } ivec2;
typedef struct ivec3 { int x,y,z; } ivec3;
#endif

#endif

#include "kit.ui/3rd_imgui_notify.h"
#include "kit.ui/kit.ui.igextra.h"
#include "kit.ui2.h"
#include "kit.ui3.h"

#endif
