@echo off

if "%1"==""      echo %0 [source.c][...] && echo %0 [ext] && echo %0 [tidy] && exit /b
if "%1"=="env"   set "path=\prj\tools;%path%" && exit /b
if "%1"=="tidy"  del *.exe *.obj *.ilk *.pdb *.dll *.ini *.exp *.lib *.dll ext*.c .*.png 2> nul & exit /b
if "%1"=="push"  call %0 tidy & (git add -Af . & git commit --amend && git push -f) & exit /b
if "%1"=="ext"   (call kit\EXT.bat %2 %3) & exit /b

if "%cc%"=="" (
(where cl.exe >nul 2>nul && set cc=cl) || (echo ERROR: cl.exe not found in PATH && exit /b)
)

pushd .
cd /D "%~dp0"

setlocal EnableDelayedExpansion

if not exist "3rd" (echo 3rd/ folder not found, cloning... && git clone https://github.com/r-lyeh/hey 3rd --depth=1 && rd /q /s 3rd\glfw 3rd\sdl2 ) || exit /b
for /R %%i in (x64\*.dll) do copy /y %%i >nul 2>nul

rem echo #define KIT_IMPL 1 > kits.c
rem echo #include "kit.h" >> kits.c

rem echo #define LUA_IMPL > kits.c
rem echo #include "3rd/3rd_minilua.h" >> kits.c

echo // Auto-generated file: DO NOT EDIT! > kits.h
for /D %%i in (kit*) do echo #if __has_include^("%%i/kit.h"^) >> kits.h && echo #         include "%%i/kit.h" >> kits.h && echo #endif >> kits.h
echo #ifdef KIT_IMPL >> kits.h
echo #ifndef CODE >> kits.h
echo #define CODE 1 >> kits.h
echo #include __FILE__ >> kits.h
echo #endif >> kits.h
echo #endif >> kits.h

rem kits++.cc 
set _3rd=3rd\hey_imgui.cc 3rd\hey_sdl3.c kits.c /EHsc && rem 3rd\hey_wgpu.c kits.cc 3rd\hey_opengl.c /DIMGUI_IMPL_WEBGPU_BACKEND_WGVK 

set old=%time%

    rem following 2 lines are a small build optimization that saves some linking time, and they can be removed at any time
    set "cold=" && if not exist "3rd.lib" set "cold=(cold build)" && del *.obj 2> nul & (cl.exe %_3rd% /c /Zi /MP -I 3rd/3rd -I 3rd /std:c11 /experimental:c11atomics /nologo && lib /nologo *.obj /out:3rd.lib)
    set _3rd=3rd.lib 3rd\hey_sdl3.c && rem 3rd\hey_opengl.c /DIMGUI_IMPL_WEBGPU_BACKEND_WGVK 

    (where /q cl.exe       && cl.exe            %* kit.c %_3rd% /Zi /MP -I. -I 3rd /std:c11 /experimental:c11atomics /nologo /link /LIBPATH:3rd && goto ok) || goto error
    (where /q clang-cl.exe && clang-cl.exe      %* kit.c %_3rd% /Zi /MP -I. -I 3rd /std:c11 /experimental:c11atomics /nologo /link /LIBPATH:3rd && goto ok) || goto error
     where /q clang.exe    && clang -o %~n1.exe %* kit.c -I 3rd 3rd/SDL3/x64/sdl3.lib && goto ok
     where /q gcc.exe      && gcc   -o %~n1.exe %* kit.c -I 3rd 3rd/SDL3/x64/sdl3.lib && goto ok

    echo No compiler found.
    goto error

:ok
set now=%time%
set "old=!old::0=: !"
set "old=!old:,0=: !"
set "old=!old:.0=: !"
set "now=!now::0=: !"
set "now=!now:,0=: !"
set "now=!now:.0=: !"
set cs0=!old:~0,2!*3600*100+!old:~3,2!*60*100+!old:~6,2!*100+!old:~9,2!
set cs1=!now:~0,2!*3600*100+!now:~3,2!*60*100+!now:~6,2!*100+!now:~9,2!
set /A diff=!cs1!-^(!cs0!^)
set diff=!diff:~0,-2!.!diff:~-2!
echo !diff!s !cold!

:quit
endlocal
popd
exit /b 0

:error
endlocal
popd
@copy , ,, >nul
