for %%i in (shaders\*.vert.glsl) do glslc -fshader-stage=vert %%i -o shaders\%%~ni.spv
for %%i in (shaders\*.frag.glsl) do glslc -fshader-stage=frag %%i -o shaders\%%~ni.spv

for %%i in (shaders2\*.vert.glsl) do glslc -fshader-stage=vert %%i -o shaders\%%~ni.spv
for %%i in (shaders2\*.frag.glsl) do glslc -fshader-stage=frag %%i -o shaders\%%~ni.spv
