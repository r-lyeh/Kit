@echo off
REM compile_shaders.bat:  Compile all GLSL shaders to SPIR-V
REM Requires glslc from the Vulkan SDK: https://vulkan.lunarg.com/
REM Run this once before building, or whenever you edit a shader.

setlocal

set OUT=%~dp0shaders

where glslc >nul 2>&1
if errorlevel 1 (
    echo ERROR: glslc not found. Install the Vulkan SDK and add it to your PATH.
    exit /b 1
)

echo Compiling graphics shaders...
glslc -fshader-stage=vert  shaders\mesh.vert.glsl        -o %OUT%\mesh.vert.spv         || goto :err
glslc -fshader-stage=frag  shaders\mesh.frag.glsl        -o %OUT%\mesh.frag.spv         || goto :err

echo Compiling IBL compute shaders...
glslc -fshader-stage=comp  shaders\equirect_to_cube.comp.glsl  -o %OUT%\equirect_to_cube.comp.spv  || goto :err
glslc -fshader-stage=comp  shaders\irradiance_conv.comp.glsl   -o %OUT%\irradiance_conv.comp.spv   || goto :err
glslc -fshader-stage=comp  shaders\prefilter_env.comp.glsl     -o %OUT%\prefilter_env.comp.spv     || goto :err
glslc -fshader-stage=comp  shaders\brdf_lut.comp.glsl          -o %OUT%\brdf_lut.comp.spv          || goto :err
glslc -fshader-stage=comp  shaders\mip_gen.comp.glsl           -o %OUT%\mip_gen.comp.spv           || goto :err

echo.
echo All shaders compiled to %OUT%\
echo Note: CMake copies these to the build output directory automatically.
goto :eof

:err
echo Shader compilation failed.
exit /b 1
