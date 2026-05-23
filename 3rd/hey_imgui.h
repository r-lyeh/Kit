#pragma once

//#define CIMGUI_API extern "C"
#define GL3W_API IMGUI_IMPL_API
#define IMGUI_USER_CONFIG "cimgui/cimconfig.h"
//#define IMGUI_USE_WCHAR32

#ifdef __cplusplus
#	define IMGUI_IMPL_API extern "C"
#	define IMGUI_DEFINE_MATH_OPERATORS
#	include "cimgui/imgui/imgui.h"
//# include "cimgui/imgui/imgui_internal.h"

#	include "cimgui/imgui/imgui_impl_sdl3.h"
//#	include "cimgui/imgui/imgui_impl_glfw.h"
//#	include "cimgui/imgui/imgui_impl_wgpu.h"
//#	include "cimgui/imgui/imgui_impl_opengl3.h"

//# include "cimgui/cimgui.h"
#else
#	define IMGUI_IMPL_API
#	define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#	define CIMGUI_USE_SDL2
#	define CIMGUI_USE_SDL3
#	define CIMGUI_USE_GLFW
//#	define CIMGUI_USE_VULKAN
#	define CIMGUI_USE_OPENGL2
#	define CIMGUI_USE_OPENGL3
#	include "cimgui/cimgui.h"
#	include "cimgui/cimgui_impl.h"
//#	include "cimgui/imgui/imgui_impl_sdl3.h"
//#	include "cimgui/imgui/imgui_impl_glfw.h"
//#	include "cimgui/imgui/imgui_impl_wgpu.h"
//#	include "cimgui/imgui/imgui_impl_opengl3.h"
#	ifndef ImVec2
#	define ImVec2(x,y) ((ImVec2){(float)x, (float)y})
#	endif
#	ifndef ImVec4
#	define ImVec4(x,y,z,w) ((ImVec4){(float)x, (float)y, (float)z, (float)w})
#	endif
#	ifndef ImRect
#	define ImRect(x,y,z,w) ((ImRect){(float)x, (float)y, (float)z, (float)w})
#	endif
#endif
