# CMakeLists.txt — kit
# Usage:
#   cmake -B build                        # uses vendored 3rd/SDL3 if present
#   cmake -B build -DKIT_FETCH_SDL3=ON    # force-download SDL3 via FetchContent
#   cmake -B build -DCMAKE_BUILD_TYPE=Debug
#   cmake --build build

cmake_minimum_required(VERSION 3.20)
project(kit C CXX)

set(CMAKE_C_STANDARD            11)
set(CMAKE_CXX_STANDARD          17)
set(CMAKE_C_STANDARD_REQUIRED   ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# options

option(KIT_FETCH_SDL3  "Force-download SDL3 via FetchContent (ignores vendored copy)" OFF)
option(KIT_BUILD_DEMOS "Build all demos"                                               ON)
option(KIT_RETAIL      "Retail/release build (disables debug/test code)"               OFF)

# ---------------------------------------------------------------------------
# SDL3 — three-tier resolution:
#   1. vendored  3rd/SDL3  (preferred on Windows, what cl.bat uses)
#   2. FetchContent        (when KIT_FETCH_SDL3=ON or no vendor + no system)
#   3. system find_package (macOS homebrew, Linux pkg)

set(KIT_SDL3_VENDORED OFF)
set(KIT_SDL3_FETCHED  OFF)

if(NOT KIT_FETCH_SDL3)
    # detect vendored layout: 3rd/SDL3/SDL.h  (included as <SDL3/SDL.h> via -I 3rd)
    if(EXISTS "${CMAKE_SOURCE_DIR}/3rd/SDL3/SDL.h")
        message(STATUS "kit: using vendored SDL3 from 3rd/SDL3/")
        set(KIT_SDL3_VENDORED ON)

        # build an INTERFACE target so the rest of the file can always use SDL3::SDL3
        add_library(SDL3_vendor INTERFACE)

        target_include_directories(SDL3_vendor INTERFACE
            "${CMAKE_SOURCE_DIR}/3rd")   # <SDL3/SDL.h> resolves from here

        if(WIN32)
            if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                set(_sdl3_libdir "${CMAKE_SOURCE_DIR}/3rd/SDL3/x64")
            else()
                set(_sdl3_libdir "${CMAKE_SOURCE_DIR}/3rd/SDL3/x86")
            endif()
            target_link_directories(SDL3_vendor INTERFACE "${_sdl3_libdir}")
            target_link_libraries(SDL3_vendor   INTERFACE SDL3)
            # copy SDL3.dll next to each executable at install/post-build
            file(GLOB _sdl3_dlls "${_sdl3_libdir}/*.dll")
        endif()

        add_library(SDL3::SDL3 ALIAS SDL3_vendor)
    endif()
endif()

# ---------------------------------------------------------------------------
# SDL3_ttf

set(KIT_SDL3_TTF_LIBS)
if(EXISTS "${CMAKE_SOURCE_DIR}/3rd/SDL3_ttf/SDL_ttf.h")
    message(STATUS "kit: using vendored SDL3_ttf from 3rd/SDL3_ttf/")
    add_library(SDL3_ttf_vendor INTERFACE)
    target_include_directories(SDL3_ttf_vendor INTERFACE "${CMAKE_SOURCE_DIR}/3rd")
    if(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(_sdl3_ttf_libdir "${CMAKE_SOURCE_DIR}/3rd/SDL3_ttf/x64")
        else()
            set(_sdl3_ttf_libdir "${CMAKE_SOURCE_DIR}/3rd/SDL3_ttf/x86")
        endif()
        target_link_directories(SDL3_ttf_vendor INTERFACE "${_sdl3_ttf_libdir}")
        target_link_libraries(SDL3_ttf_vendor   INTERFACE SDL3_ttf)
        file(GLOB _sdl3_ttf_dlls "${_sdl3_ttf_libdir}/*.dll")
        list(APPEND _sdl3_dlls ${_sdl3_ttf_dlls})
    endif()
    add_library(SDL3_ttf::SDL3_ttf ALIAS SDL3_ttf_vendor)
    set(KIT_SDL3_TTF_LIBS SDL3_ttf::SDL3_ttf)
else()
    find_package(SDL3_ttf QUIET)
    if(SDL3_ttf_FOUND)
        message(STATUS "kit: using system SDL3_ttf")
        set(KIT_SDL3_TTF_LIBS SDL3_ttf::SDL3_ttf)
    else()
        message(STATUS "kit: SDL3_ttf not found — TTF features disabled")
    endif()
endif()

# ---------------------------------------------------------------------------
# SDL3_net

set(KIT_SDL3_NET_LIBS)
if(EXISTS "${CMAKE_SOURCE_DIR}/3rd/SDL3_net/SDL_net.h")
    message(STATUS "kit: using vendored SDL3_net from 3rd/SDL3_net/")
    add_library(SDL3_net_vendor INTERFACE)
    target_include_directories(SDL3_net_vendor INTERFACE "${CMAKE_SOURCE_DIR}/3rd")
    if(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(_sdl3_net_libdir "${CMAKE_SOURCE_DIR}/3rd/SDL3_net/x64")
        else()
            set(_sdl3_net_libdir "${CMAKE_SOURCE_DIR}/3rd/SDL3_net/x86")
        endif()
        target_link_directories(SDL3_net_vendor INTERFACE "${_sdl3_net_libdir}")
        target_link_libraries(SDL3_net_vendor   INTERFACE SDL3_net)
        file(GLOB _sdl3_net_dlls "${_sdl3_net_libdir}/*.dll")
        list(APPEND _sdl3_dlls ${_sdl3_net_dlls})
    endif()
    add_library(SDL3_net::SDL3_net ALIAS SDL3_net_vendor)
    set(KIT_SDL3_NET_LIBS SDL3_net::SDL3_net)
else()
    find_package(SDL3_net QUIET)
    if(SDL3_net_FOUND)
        message(STATUS "kit: using system SDL3_net")
        set(KIT_SDL3_NET_LIBS SDL3_net::SDL3_net)
    else()
        message(STATUS "kit: SDL3_net not found — network features disabled")
    endif()
endif()

if(NOT KIT_SDL3_VENDORED)
    # try system install first (homebrew / pkg-config / vcpkg)
    find_package(SDL3 QUIET)

    if(SDL3_FOUND)
        message(STATUS "kit: using system SDL3")
    else()
        # last resort: download
        message(STATUS "kit: SDL3 not found locally — fetching via FetchContent")
        set(KIT_SDL3_FETCHED ON)
        include(FetchContent)
        FetchContent_Declare(SDL3
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG        release-3.4.0
            GIT_SHALLOW    TRUE)
        set(SDL_SHARED OFF CACHE BOOL "" FORCE)
        set(SDL_STATIC ON  CACHE BOOL "" FORCE)
        set(SDL_TEST   OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(SDL3)
    endif()
endif()

# ---------------------------------------------------------------------------
# platform link requirements

if(APPLE)
    set(KIT_PLATFORM_DEFINES   GL_SILENCE_DEPRECATION=1)
    set(KIT_PLATFORM_LIBS
        SDL3::SDL3
        ${KIT_SDL3_TTF_LIBS}
        ${KIT_SDL3_NET_LIBS}
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreAudio"
        "-framework AudioToolbox"
        "-framework CoreFoundation"
        "-framework Metal"
        "-framework QuartzCore")
elseif(UNIX)
    set(KIT_PLATFORM_DEFINES   _GNU_SOURCE)   # strsep on Linux
    set(KIT_PLATFORM_LIBS      SDL3::SDL3 ${KIT_SDL3_TTF_LIBS} ${KIT_SDL3_NET_LIBS} m dl pthread)
elseif(WIN32)
    set(KIT_PLATFORM_LIBS      SDL3::SDL3 ${KIT_SDL3_TTF_LIBS} ${KIT_SDL3_NET_LIBS})
endif()

# ---------------------------------------------------------------------------
# compiler flags

if(MSVC)
    # force dynamic CRT to match SDL3's precompiled libs
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

    add_compile_options(
        /MP /nologo
        /std:c11 /experimental:c11atomics   # for C files
    )
    if(NOT KIT_RETAIL)
        add_compile_options(/Zi)
        add_link_options(/DEBUG)
    else()
        add_compile_options(/Ox /Oy) # sadly, /MT would require to rebuild SDL3.dll
        add_compile_definitions(NDEBUG)
    endif()
else()
    # Clang / GCC / AppleClang
    add_compile_options(
        -Wall -Wextra
        -Wno-unused-parameter
        -Wno-missing-field-initializers
        -fno-strict-aliasing)
    if(NOT KIT_RETAIL)
        add_compile_options(-g)
    else()
        add_compile_options(-O2)
        add_compile_definitions(NDEBUG)
    endif()
endif()

# ---------------------------------------------------------------------------
# shared implementation library

add_library(kit_impl STATIC
    kits.c
    3rd/hey_sdl3.c
    3rd/hey_imgui.cc)

target_compile_definitions(kit_impl PUBLIC
    #KIT_IMPL=1
    ${KIT_PLATFORM_DEFINES})

target_include_directories(kit_impl PUBLIC
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/kit"
    "${CMAKE_SOURCE_DIR}/kits"
    "${CMAKE_SOURCE_DIR}/3rd")

# hey_imgui.cc needs C++ flags, not /std:c11
set_source_files_properties(3rd/hey_imgui.cc PROPERTIES LANGUAGE CXX)

target_link_libraries(kit_impl PUBLIC ${KIT_PLATFORM_LIBS})

# ---------------------------------------------------------------------------
# helper: single-file demo executable

function(kit_demo target source)
    add_executable(${target} "${source}" "kit.c")

    target_include_directories(${target} PRIVATE
        "${CMAKE_SOURCE_DIR}"
        "${CMAKE_SOURCE_DIR}/kit"
        "${CMAKE_SOURCE_DIR}/kits"
        "${CMAKE_SOURCE_DIR}/3rd")

    target_compile_definitions(${target} PRIVATE ${KIT_PLATFORM_DEFINES})
    target_link_libraries(${target} PRIVATE kit_impl)

    # run from repo root so relative asset paths work
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}")

    # copy vendored SDL3.dll(s) next to the executable on Windows
    if(WIN32 AND _sdl3_dlls)
        foreach(_dll ${_sdl3_dlls})
            add_custom_command(TARGET ${target} POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                    "${_dll}"
                    "$<TARGET_FILE_DIR:${target}>"
                VERBATIM)
        endforeach()
    endif()
endfunction()

# ---------------------------------------------------------------------------
# demos (ordered by tutorial complexity)

if(KIT_BUILD_DEMOS)
    kit_demo(hello hello.c)
#    kit_demo(01_loop          demos/00_loop.c)
#    kit_demo(02_entrypoints   demos/00_entrypoints.c)
#    kit_demo(03_list          demos/00_list.c)
#    kit_demo(04_audio         demos/00_audio.c)
#    kit_demo(05_webcam        demos/00_webcam.c)
#    kit_demo(06_triangle      demos/00_triangle.c)
#    kit_demo(07_lua           demos/00_lua.c)
#    kit_demo(08_ui            demos/00_ui.c)
#    kit_demo(09_imgui1        demos/00_imgui1.c)
#    kit_demo(10_vsync         demos/00_vsync.c)
#    kit_demo(11_tray          demos/00_tray.c)
#    kit_demo(12_imgui2        demos/00_imgui2.c)
#    kit_demo(13_postfx        demos/00_postfx.c)
#    kit_demo(14_font          demos/00_font.c)
#    kit_demo(15_sprite        demos/00_sprite.c)
#    kit_demo(16_input         demos/00_input.c)
#    kit_demo(17_bvh           demos/00_bvh.c)
#    kit_demo(18_dd            demos/00_dd.c)
endif()
