@if "%1"=="env"  set "path=\prj\tools;%path%" && exit /b
@if "%1"=="tidy" del *.exe *.obj *.ilk *.pdb *.dll *.ini 2> nul & exit /b
@if "%1"=="push" call %0 tidy & (git add -Af . & git commit --amend && git push -f) & exit /b

set wgvk=backends\3rd\wgvk\src\wgvk.c -I backends\3rd\wgvk\include
set glfw=-D_GLFW_WIN32 backends\3rd\glfw3\glfw.c -I backends\3rd\glfw3\include
set imgui=-DIMGUI_IMPL_WEBGPU_BACKEND_WGVK backends\3rd\imgui\backends\imgui_impl_glfw.cpp -I backends\3rd\imgui -I backends\3rd\imgui\backends
set common=-I. -I backends -I backends\3rd /experimental:c11atomics /std:c11 /Zi /nologo /MP
set cc=cl

rem %cc% test.imgui.glfw.wgpu.cc backends\kit*.c* %imgui% %wgvk% %glfw% %common% %* || exit /b

cl test.ui.c backends\kit_tigr.c %common%
