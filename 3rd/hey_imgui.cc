#include "hey_imgui.h"

#include "cimgui/imgui/imgui.cpp"
#include "cimgui/imgui/imgui_demo.cpp"
#include "cimgui/imgui/imgui_draw.cpp"
#include "cimgui/imgui/imgui_tables.cpp"
#include "cimgui/imgui/imgui_widgets.cpp"

#include "cimgui/cimgui.cpp"

//#include "cimgui/imgui/imgui_impl_opengl3.cpp" // gl3: highest include priority
//#include "cimgui/imgui/imgui_impl_dx10.cpp"
//#include "cimgui/imgui/imgui_impl_dx11.cpp"
//#include "cimgui/imgui/imgui_impl_dx12.cpp"
//#include "cimgui/imgui/imgui_impl_dx9.cpp"
//#include "cimgui/imgui/imgui_impl_opengl2.cpp"
//#include "cimgui/imgui/imgui_impl_vulkan.cpp"
//#include "cimgui/imgui/imgui_impl_wgpu.cpp"

#if __has_include("SDL3/SDL.h")
#include "cimgui/imgui/imgui_impl_sdl3.cpp"
#include "cimgui/imgui/imgui_impl_sdlgpu3.cpp"
#include "cimgui/imgui/imgui_impl_sdlrenderer3.cpp"
#elif __has_include("SDL2/SDL.h")
#include "cimgui/imgui/imgui_impl_sdl2.cpp"
#include "cimgui/imgui/imgui_impl_sdlrenderer2.cpp"
#endif
#if __has_include("GLFW/glfw3.h")
#define Saturate Saturate2
#include "cimgui/imgui/imgui_impl_glfw.cpp"
#endif
//#include "cimgui/imgui/imgui_impl_allegro5.cpp"
//#include "cimgui/imgui/imgui_impl_glut.cpp"

#ifdef _WIN32
#include "cimgui/imgui/imgui_impl_win32.cpp"
#endif
//#include "cimgui/imgui/imgui_impl_android.cpp"
//#include "cimgui/imgui/imgui_impl_metal.mm"
//#include "cimgui/imgui/imgui_impl_osx.mm"
//#include "cimgui/imgui/imgui_impl_null.cpp"
