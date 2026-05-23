for /R %%i in (*.vert.glsl) do glslc -fshader-stage=vert %%i -o %%~di%%~pi%%~ni.spv
for /R %%i in (*.frag.glsl) do glslc -fshader-stage=frag %%i -o %%~di%%~pi%%~ni.spv
