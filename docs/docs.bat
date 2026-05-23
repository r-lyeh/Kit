type docs.css > docs.html &&^
type docs.md >> docs.html &&^
cl docs.c && (docs.exe < ..\kit.h >> docs.html) &&^
start "" docs.html & del *.exe *.obj
