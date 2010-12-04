:: The Powder Toy + MinGW
:: by JoJoBond
@echo off
cls
echo Please enter your target architecture:
echo [1] Nativ (local machine)
echo [2] Generic (universal machine)
echo [Others] Cancel
set /p ARCH=Selection:
IF %ARCH%==1 (
set ARCH=-march=native
) ELSE (
IF %ARCH%==2 (
set ARCH=-march=generic
) ELSE (
GOTO QUIT
)
)
echo Please enter your SSE instruction set:
echo [1] No SSE
echo [2] SSE1
echo [3] SSE2
echo [4] SSE3
echo [5] SSE4
echo [6] SSE4.1
echo [7] SSE4.2
echo [Others] Cancel
set /p SSE=Selection:
IF %SSE%==1 (
set SSE=-mno-sse
) ELSE (
IF %SSE%==2 (
set SSE=-msse
) ELSE (
IF %SSE%==3 (
set SSE=-msse2
) ELSE (
IF %SSE%==4 (
set SSE=-msse3
) ELSE (
IF %SSE%==5 (
set SSE=-msse4
) ELSE (
IF %SSE%==6 (
set SSE=-msse4.1
) ELSE (
IF %SSE%==7 (
	set SSE=-msse4.2
) ELSE (
	GOTO QUIT
)
)
)
)
)
)
)
echo Please enter your MMX compatibility:
echo [1] No MMX
echo [2] MMX
echo [Others] Cancel
set /p MMX=Selection:
IF %MMX%==1 (
set MMX=-mno-mmx
) ELSE (
IF %MMX%==2 (
set MMX=-mmmx
) ELSE (
GOTO QUIT
)
)
echo Deleting old files
del Powder.exe
del powder-res.o
echo Packing resources
windres src/Resources/powder-res.rc powder-res.o
echo Compiling
gcc -Wall -m32 -std=c99 -D_POSIX_C_SOURCE=200112L -O3 -ffast-math -ftree-vectorize -funsafe-math-optimizations %ARCH% %SSE% %MMX% -DX86 -mwindows -DWIN32 -oPowder -Iincludes src/*.c src/renderer/sw.c powder-res.o -lmingw32 -lSDL.dll -lSDLmain -lSDL -lbz2 -lwsock32 -lws2_32
IF NOT ERRORLEVEL 1 GOTO QUIT
pause
:QUIT
:QUIT