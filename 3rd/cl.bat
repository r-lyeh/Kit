@if "%1"=="tidy" del *.ilk *.pdb *.obj *.exe *.dll *.obj *.lib *.exp imgui.ini 2> nul & exit /b
@for /R %%i in (x64\*.dll) do copy /y %%i >nul 2>nul
cl.exe %* /Zi /MP -I . /std:c11 /experimental:c11atomics /nologo
