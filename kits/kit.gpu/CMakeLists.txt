cmake_minimum_required(VERSION 3.21)
project(sdl3_gltf_starter C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ─── Options ────────────────────────────────────────────────────────────────
option(FETCH_SDL3 "Download SDL3 via FetchContent if not found locally" ON)

# ─── SDL3 ────────────────────────────────────────────────────────────────────
include(FetchContent)   # always needed (SDL3 and/or cgltf)
if(FETCH_SDL3)
    FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        release-3.4.8        # pin to a known-good tag
        GIT_SHALLOW    TRUE
    )
    set(SDL_SHARED  OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC  ON  CACHE BOOL "" FORCE)
    set(SDL_TEST    OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(SDL3)
else()
    find_package(SDL3 REQUIRED)
endif()

# ─── cgltf (single-header) ───────────────────────────────────────────────────
# If the real header already exists locally (vendored), use it.
# Otherwise, fetch it from GitHub with FetchContent.
if(EXISTS "${CMAKE_SOURCE_DIR}/external/cgltf/cgltf.h" AND
   NOT "${CMAKE_SOURCE_DIR}/external/cgltf/cgltf.h" MATCHES "PLACEHOLDER")
    message(STATUS "cgltf: using vendored copy in external/cgltf/")
    add_library(cgltf INTERFACE)
    target_include_directories(cgltf INTERFACE external/cgltf)
else()
    message(STATUS "cgltf: fetching from GitHub...")
    FetchContent_Declare(
        cgltf
        GIT_REPOSITORY https://github.com/jkuhlmann/cgltf.git
        GIT_TAG        v1.14
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(cgltf)
    add_library(cgltf INTERFACE)
    target_include_directories(cgltf INTERFACE ${cgltf_SOURCE_DIR})
endif()


# ─── stb_image (single-header image loader) ─────────────────────────────────
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(stb)
add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${stb_SOURCE_DIR})

# ─── Sources ─────────────────────────────────────────────────────────────────
set(SOURCES
    src/main.c
    src/app.c
    src/gltf_loader.c
    src/gpu_pipeline.c
    src/mesh.c
    src/camera.c
    src/texture_cache.c
    src/ibl.c
    src/scalability.c
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        SDL3::SDL3-static
        stb_image
        cgltf
)

# ─── Shader compilation (optional, needs SDL_shadercross or glslc) ────────────
# Shaders are compiled at build time when glslc is found.
find_program(GLSLC glslc)
if(GLSLC)
    set(SHADER_DIR "${CMAKE_SOURCE_DIR}/shaders")
    set(SHADER_OUT "${CMAKE_BINARY_DIR}/shaders")
    file(MAKE_DIRECTORY ${SHADER_OUT})

    foreach(STAGE vert frag)
        add_custom_command(
            OUTPUT  "${SHADER_OUT}/mesh.${STAGE}.spv"
            COMMAND ${GLSLC} -fshader-stage=${STAGE}
                    "${SHADER_DIR}/mesh.${STAGE}.glsl"
                    -o "${SHADER_OUT}/mesh.${STAGE}.spv"
            DEPENDS "${SHADER_DIR}/mesh.${STAGE}.glsl"
            COMMENT "Compiling mesh.${STAGE}.glsl → SPIR-V"
        )
        list(APPEND SPIRV_OUTPUTS "${SHADER_OUT}/mesh.${STAGE}.spv")
    endforeach()

    # Compute shaders
    foreach(CS equirect_to_cube irradiance_conv prefilter_env brdf_lut mip_gen)
        add_custom_command(
            OUTPUT  "${SHADER_OUT}/${CS}.comp.spv"
            COMMAND ${GLSLC} -fshader-stage=compute
                    "${SHADER_DIR}/${CS}.comp.glsl"
                    -o "${SHADER_OUT}/${CS}.comp.spv"
            DEPENDS "${SHADER_DIR}/${CS}.comp.glsl"
            COMMENT "Compiling ${CS}.comp.glsl → SPIR-V"
        )
        list(APPEND SPIRV_OUTPUTS "${SHADER_OUT}/${CS}.comp.spv")
    endforeach()

    add_custom_target(shaders ALL DEPENDS ${SPIRV_OUTPUTS})
    add_dependencies(${PROJECT_NAME} shaders)

    target_compile_definitions(${PROJECT_NAME} PRIVATE
        SHADER_PATH="shaders/"
        USE_SPIRV=1
    )
else()
    message(FATAL_ERROR
        "glslc not found. glslc is required to compile GLSL shaders to SPIR-V.\n"
        "Install the Vulkan SDK from https://vulkan.lunarg.com/ which includes glslc,\n"
        "then re-run cmake. Alternatively, run compile_shaders.bat (Windows) or\n"
        "compile_shaders.sh (Linux/macOS) manually after adding glslc to your PATH."
    )
endif()

# Copy assets next to the binary
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMENT "Copying assets"
)

# Copy scalability.ini next to the binary (app reads from CWD)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/assets/scalability.ini"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/scalability.ini"
    COMMENT "Copying scalability.ini"
)

# Copy compiled shaders next to the binary so SHADER_PATH="shaders/" always works
if(GLSLC)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${SHADER_OUT}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders"
        COMMENT "Copying compiled shaders to output"
    )
endif()

# ─── Platform tweaks ─────────────────────────────────────────────────────────
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wno-unused-parameter)
endif()
