-- premake5.lua — kit
-- Windows:  premake5 vs2022 && msbuild build\kit.sln /p:Configuration=Debug /p:Platform=x64 /m /nologo
-- Windows:  premake5 vs2019 && msbuild build\kit.sln /p:Configuration=Debug /p:Platform=x64 /m /nologo
-- Linux:    premake5 gmake2 && make -C build config=debug_x64
-- macOS:    premake5 gmake2 && make -C build config=debug_x64
-- macOS:    premake5 xcode4

workspace "kit"
    configurations  { "Debug", "Release" }
    platforms       { "x64", "x86" }
    location        "build"
    cdialect        "C11"
    cppdialect      "C++17"
    startproject    "hello"

    -- architecture mapping
    filter "platforms:x64"
        architecture "x86_64"
    filter "platforms:x86"
        architecture "x86"
    filter {}

    -- shared include paths
    includedirs { ".", "kit", "kits", "3rd" }

    -- platform defines
    filter "system:windows"
        defines      { "KIT_WINDOWS=1" }
        characterset "Unicode"

    filter "system:macosx"
        defines      { "KIT_MACOS=1", "GL_SILENCE_DEPRECATION=1" }

    filter "system:linux"
        defines      { "KIT_LINUX=1", "_GNU_SOURCE" }

    filter {}

-- ---------------------------------------------------------------------------
-- SDL3 linkage — vendored on Windows, pkg-config on macOS/Linux

local function sdl3_link()
    filter "system:windows"
        filter { "system:windows", "platforms:x64" }
            if os.isfile("3rd/SDL3/x64/SDL3.lib") then
                libdirs { "3rd/SDL3/x64" }
            end
        filter { "system:windows", "platforms:x86" }
            if os.isfile("3rd/SDL3/x86/SDL3.lib") then
                libdirs { "3rd/SDL3/x86" }
            end
        filter "system:windows"
            links { "SDL3" }
            if os.isfile("3rd/SDL3_ttf/x64/SDL3_ttf.lib") then
                links { "SDL3_ttf" }
            end
            if os.isfile("3rd/SDL3_net/x64/SDL3_net.lib") then
                links { "SDL3_net" }
            end

    filter "system:macosx"
        buildoptions { "`pkg-config --cflags sdl3 2>/dev/null || echo -I/usr/local/include`" }
        linkoptions  {
            "`pkg-config --libs sdl3 2>/dev/null || echo -L/usr/local/lib -lSDL3`",
            "`pkg-config --libs sdl3-ttf 2>/dev/null`",
            "`pkg-config --libs sdl3-net 2>/dev/null`",
        }
        links {
            "Cocoa.framework",
            "IOKit.framework",
            "CoreAudio.framework",
            "AudioToolbox.framework",
            "CoreFoundation.framework",
            "Metal.framework",
            "QuartzCore.framework",
        }

    filter "system:linux"
        buildoptions { "`pkg-config --cflags sdl3`" }
        linkoptions  {
            "`pkg-config --libs sdl3`",
            "`pkg-config --libs sdl3-ttf 2>/dev/null`",
            "`pkg-config --libs sdl3-net 2>/dev/null`",
        }
        links { "m", "dl", "pthread" }

    filter {}
end

-- ---------------------------------------------------------------------------
-- shared implementation library

project "kit_impl"
    kind       "StaticLib"
    targetdir  "build/%{cfg.buildcfg}_%{cfg.platform}"
    objdir     "build/obj/%{prj.name}/%{cfg.buildcfg}_%{cfg.platform}"

    -- C sources
    files { "kits.c", "3rd/hey_sdl3.c" }

    -- C++ source (imgui glue) — override language per file
    files { "3rd/hey_imgui.cc" }
    filter "files:3rd/hey_imgui.cc"
        language "C++"
    filter {}

    -- SDL3 vendored include path (headers live under 3rd/)
    filter { "system:windows", "platforms:x64" }
        libdirs { "3rd/SDL3/x64", "3rd/SDL3_ttf/x64", "3rd/SDL3_net/x64" }
    filter { "system:windows", "platforms:x86" }
        libdirs { "3rd/SDL3/x86", "3rd/SDL3_ttf/x86", "3rd/SDL3_net/x86" }
    filter {}

    -- MSVC flags — c11 only for C files, not for hey_imgui.cc
    filter { "toolset:msc*", "files:**.c" }
        buildoptions {
            "/MP",
            "/std:c11",
            "/experimental:c11atomics",
            "/wd4996",
        }
    filter { "toolset:msc*", "files:**.cc" }
        buildoptions {
            "/MP",
            "/wd4996",
        }
    -- Clang/GCC flags
    filter "toolset:clang* or toolset:gcc*"
        buildoptions {
            "-Wall", "-Wextra",
            "-Wno-unused-parameter",
            "-Wno-missing-field-initializers",
            "-fno-strict-aliasing",
        }
    filter {}

    sdl3_link()

    filter "configurations:Debug"
        symbols   "On"
        optimize  "Off"
        runtime   "Debug"
    filter "configurations:Release"
        symbols   "Off"
        optimize  "Speed"
        runtime   "Release"
        defines   { "NDEBUG", "KIT_RETAIL=1" }
    filter {}

-- ---------------------------------------------------------------------------
-- helper: single-file kit demo/executable

local function kit_exe(name, src)
    project(name)
        kind       "ConsoleApp"
        language   "C"
        -- output at repo root so relative asset paths (demos/art/) work
        targetdir  "."
        objdir     "build/obj/%{prj.name}/%{cfg.buildcfg}_%{cfg.platform}"

        files      { src, "kit.c" }
        links      { "kit_impl" }

        -- SDL3 vendored lib dirs for the executable linker pass too
        filter { "system:windows", "platforms:x64" }
            libdirs { "3rd/SDL3/x64", "3rd/SDL3_ttf/x64", "3rd/SDL3_net/x64" }
        filter { "system:windows", "platforms:x86" }
            libdirs { "3rd/SDL3/x86", "3rd/SDL3_ttf/x86", "3rd/SDL3_net/x86" }
        filter {}

        -- copy vendored DLLs next to the exe on Windows
        filter { "system:windows", "platforms:x64" }
            postbuildcommands {
                '{COPYFILE} "%{wks.location}/../3rd/SDL3/x64/SDL3.dll"     "%{cfg.targetdir}"',
                '{COPYFILE} "%{wks.location}/../3rd/SDL3_ttf/x64/SDL3_ttf.dll" "%{cfg.targetdir}" 2>nul || ver>nul',
                '{COPYFILE} "%{wks.location}/../3rd/SDL3_net/x64/SDL3_net.dll" "%{cfg.targetdir}" 2>nul || ver>nul',
            }
        filter { "system:windows", "platforms:x86" }
            postbuildcommands {
                '{COPYFILE} "%{wks.location}/../3rd/SDL3/x86/SDL3.dll"     "%{cfg.targetdir}"',
                '{COPYFILE} "%{wks.location}/../3rd/SDL3_ttf/x86/SDL3_ttf.dll" "%{cfg.targetdir}" 2>nul || ver>nul',
                '{COPYFILE} "%{wks.location}/../3rd/SDL3_net/x86/SDL3_net.dll" "%{cfg.targetdir}" 2>nul || ver>nul',
            }
        filter {}
        filter { "toolset:msc*", "files:**.c" }
            buildoptions { "/std:c11", "/experimental:c11atomics", "/wd4996" }
        filter { "toolset:msc*", "files:**.cc" }
            buildoptions { "/wd4996" }
        filter {}

        sdl3_link()

        filter "configurations:Debug"
            symbols  "On"
            optimize "Off"
            runtime  "Debug"
        filter "configurations:Release"
            symbols  "Off"
            optimize "Speed"
            runtime  "Release"
            defines  { "NDEBUG", "KIT_RETAIL=1" }
        filter {}
end

-- ---------------------------------------------------------------------------
-- executables

kit_exe("hello",          "hello.c")

-- demos (tutorial order — uncomment as they become ready)
--kit_exe("01_loop",        "demos/00_loop.c")
--kit_exe("02_entrypoints", "demos/00_entrypoints.c")
--kit_exe("03_list",        "demos/00_list.c")
--kit_exe("04_audio",       "demos/00_audio.c")
--kit_exe("05_webcam",      "demos/00_webcam.c")
--kit_exe("06_triangle",    "demos/00_triangle.c")
--kit_exe("07_lua",         "demos/00_lua.c")
--kit_exe("08_ui",          "demos/00_ui.c")
--kit_exe("09_imgui1",      "demos/00_imgui1.c")
--kit_exe("10_vsync",       "demos/00_vsync.c")
--kit_exe("11_tray",        "demos/00_tray.c")
--kit_exe("12_imgui2",      "demos/00_imgui2.c")
--kit_exe("13_postfx",      "demos/00_postfx.c")
--kit_exe("14_font",        "demos/00_font.c")
--kit_exe("15_sprite",      "demos/00_sprite.c")
--kit_exe("16_input",       "demos/00_input.c")
--kit_exe("17_bvh",         "demos/00_bvh.c")
--kit_exe("18_dd",          "demos/00_dd.c")
