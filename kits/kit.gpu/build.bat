@echo off
REM build.bat: One-shot configure + build for Windows (MSVC or MinGW)
setlocal

set BUILD_DIR=%1
if "%BUILD_DIR%"=="" set BUILD_DIR=build

set BUILD_TYPE=%2
if "%BUILD_TYPE%"=="" set BUILD_TYPE=Release

echo =^> Configuring (build_type=%BUILD_TYPE%, dir=%BUILD_DIR%)
cmake -B %BUILD_DIR% -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DFETCH_SDL3=ON
if errorlevel 1 goto :err

echo =^> Building
cmake --build %BUILD_DIR% --config %BUILD_TYPE% --parallel
if errorlevel 1 goto :err

echo.
echo Done.  Run: %BUILD_DIR%\%BUILD_TYPE%\sdl3_gltf_starter.exe
goto :eof

:err
echo Build failed.
exit /b 1
