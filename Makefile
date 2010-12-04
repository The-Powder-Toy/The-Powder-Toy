SOURCES := src/*.c

CFLAGS := -Wall -std=c99 -D_POSIX_C_SOURCE=200112L -Iincludes/
OFLAGS := -O3 -ffast-math -ftree-vectorize -funsafe-math-optimizations
LFLAGS := -lSDL -lm -lbz2 
MFLAGS_SSE3 := -march=native -DX86 -DX86_SSE3 -msse3
MFLAGS_SSE2 := -march=native-DX86 -DX86_SSE2 -msse2
MFLAGS_SSE := -march=native -DX86 -DX86_SSE
FLAGS_DBUG := -Wall -std=c99 -D_POSIX_C_SOURCE=200112L -pg -DX86 -DX86_SSE3 -msse3 -lSDL -lm -lbz2 -g
COMPILER := gcc
LINUX_TARG := powder-64-sse2 powder-sse powder-sse2
WIN32_TARG := powder-sse.exe powder-sse2.exe

powder: $(SOURCES) src/renderer/sw.c
	$(COMPILER) -DINTERNAL -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE3) $(SOURCES) src/renderer/sw.c -DLIN64
powder-debug-64: $(SOURCES) src/renderer/sw.c
	$(COMPILER) -m64 -o$@ $(FLAGS_DBUG) -DLIN64 $(SOURCES) src/renderer/sw.c -Iincludes/
powder-debug: $(SOURCES) src/renderer/sw.c
	$(COMPILER) -m32 -o$@ $(FLAGS_DBUG) -DLIN32 $(SOURCES) src/renderer/sw.c -Iincludes/ 
powder-sse3: $(SOURCES) src/renderer/sw.c
	$(COMPILER) -m32 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE3) $(SOURCES) src/renderer/sw.c -DLIN32
	strip $@
	mv $@ build
powder-sse2: $(SOURCES) src/renderer/sw.c
	$(COMPILER) -m32 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE2) $(SOURCES) src/renderer/sw.c -DLIN32
	strip $@
powder-sse: $(SOURCES) src/renderer/sw.c
	$(COMPILER) -m32 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE) $(SOURCES) src/renderer/sw.c -DLIN32
	strip $@
powder-64-sse3-opengl: $(SOURCES) src/renderer/ogl.c
	$(COMPILER) -m64 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE3) $(SOURCES) src/renderer/ogl.c -DLIN64 -lGL -lGLU -DOpenGL
	strip $@
powder-64-sse3: $(SOURCES) src/renderer/sw.c
	$(COMPILER) -m64 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE3) $(SOURCES) src/renderer/sw.c -DLIN64 
	strip $@
	mv $@ build
powder-64-sse2: $(SOURCES) src/renderer/sw.c
	$(COMPILER) -m64 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE2) $(SOURCES) src/renderer/sw.c -DLIN64
	strip $@

powder-res.o: powder-res.rc powder.ico
	i586-mingw32msvc-windres powder-res.rc powder-res.o

powder-sse3.exe: $(SOURCES) src/renderer/sw.c powder-res.o
	i586-mingw32msvc-gcc -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE3) $(SOURCES) src/renderer/sw.c powder-res.o -lmingw32 -lws2_32 -lSDLmain $(LFLAGS) -mwindows -DWIN32
	strip $@
	chmod 0644 $@
powder-sse2.exe: $(SOURCES) src/renderer/sw.c powder-res.o
	i586-mingw32msvc-gcc -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE2) $(SOURCES) src/renderer/sw.c powder-res.o -lmingw32 -lws2_32 -lSDLmain $(LFLAGS) -mwindows -DWIN32
	strip $@
	chmod 0644 $@
powder-sse.exe: $(SOURCES) src/renderer/sw.c powder-res.o
	i586-mingw32msvc-gcc -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE) $(SOURCES) src/renderer/sw.c powder-res.o -lmingw32 -lws2_32 -lSDLmain $(LFLAGS) -mwindows -DWIN32
	strip $@
	chmod 0644 $@

powder-x: $(SOURCES) src/renderer/sw.c
	gcc -o $@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS) $(SOURCES) src/renderer/sw.c -lSDLmain -DMACOSX -DPIX32BGRA -arch x86_64 -framework Cocoa -ggdb
	strip $@ 
powder-x-ogl: $(SOURCES) src/renderer/ogl.c
	gcc -o $@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS) $(SOURCES) src/renderer/ogl.c -lSDLmain -DOpenGL -DMACOSX -DPIX32BGRA -arch x86_64 -framework Cocoa -framework OpenGL -ggdb
	strip $@

win_powder-sse.exe: $(SOURCES) src/renderer/sw.c powder-res.o
	gcc -Wall -m32 -std=c99 -D_POSIX_C_SOURCE=200112L -O3 -ffast-math -ftree-vectorize -funsafe-math-optimizations -march=native -msse -mmmx -DX86 -mwindows -DWIN32 -o$@ -Iincludes src/*.c src/renderer/sw.c powder-res.o -lmingw32 -lSDL.dll -lSDLmain -lSDL -lbz2 -lwsock32 -lws2_32

win_powder-sse2.exe: $(SOURCES) src/renderer/sw.c powder-res.o
	gcc -Wall -m32 -std=c99 -D_POSIX_C_SOURCE=200112L -O3 -ffast-math -ftree-vectorize -funsafe-math-optimizations -march=native -msse2 -mmmx -DX86 -mwindows -DWIN32 -o$@ -Iincludes src/*.c src/renderer/sw.c powder-res.o -lmingw32 -lSDL.dll -lSDLmain -lSDL -lbz2 -lwsock32 -lws2_32

win_powder-sse3.exe: $(SOURCES) src/renderer/sw.c powder-res.o
	gcc -Wall -m32 -std=c99 -D_POSIX_C_SOURCE=200112L -O3 -ffast-math -ftree-vectorize -funsafe-math-optimizations -march=native -msse3 -mmmx -DX86 -mwindows -DWIN32 -o$@ -Iincludes src/*.c src/renderer/sw.c powder-res.o -lmingw32 -lSDL.dll -lSDLmain -lSDL -lbz2 -lwsock32 -lws2_32

win_powder-sse-ogl.exe: $(SOURCES) src/renderer/ogl.c powder-res.o
	gcc -Wall -m32 -std=c99 -D_POSIX_C_SOURCE=200112L -O3 -ffast-math -ftree-vectorize -funsafe-math-optimizations -march=native -msse -mmmx -DX86 -mwindows -DWIN32 -o$@ -Iincludes src/*.c src/renderer/ogl.c powder-res.o -lmingw32 -lSDL.dll -lSDLmain -lSDL -lbz2 -lwsock32 -lws2_32 -lopengl32

win_powder-sse2-ogl.exe: $(SOURCES) src/renderer/ogl.c powder-res.o
	gcc -Wall -m32 -std=c99 -D_POSIX_C_SOURCE=200112L -O3 -ffast-math -ftree-vectorize -funsafe-math-optimizations -march=native -msse2 -mmmx -DX86 -mwindows -DWIN32 -o$@ -Iincludes src/*.c src/renderer/ogl.c powder-res.o -lmingw32 -lSDL.dll -lSDLmain -lSDL -lbz2 -lwsock32 -lws2_32 -lopengl32

win_powder-sse3-ogl.exe: $(SOURCES) src/renderer/ogl.c powder-res.o
	gcc -Wall -m32 -std=c99 -D_POSIX_C_SOURCE=200112L -O3 -ffast-math -ftree-vectorize -funsafe-math-optimizations -march=native -msse3 -mmmx -DX86 -mwindows -DWIN32 -o$@ -Iincludes src/*.c src/renderer/ogl.c powder-res.o -lmingw32 -lSDL.dll -lSDLmain -lSDL -lbz2 -lwsock32 -lws2_32 -lopengl32
