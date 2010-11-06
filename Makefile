UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
# Is Linux environment

SOURCES := src/*.c
CFLAGS := -Wall -std=c99 -D_POSIX_C_SOURCE=200112L -Iincludes/
OFLAGS := -O3 -ffast-math -ftree-vectorize -funsafe-math-optimizations
LFLAGS := -lSDL -lm -lbz2
MFLAGS_SSE3 := -march=native -DX86 -DX86_SSE3 -msse3
MFLAGS_SSE2 := -march=native -DX86 -DX86_SSE2 -msse2
MFLAGS_SSE := -march=native -DX86 -DX86_SSE
FLAGS_DBUG := -Wall -std=c99 -D_POSIX_C_SOURCE=200112L -pg -DX86 -DX86_SSE3 -msse3 -lSDL -lm -lbz2 -g
COMPILER := gcc
LINUX_TARG := powder-64-sse2 powder-sse powder-sse2
WIN32_TARG := powder-sse.exe powder-sse2.exe

powder: $(SOURCES)
	$(COMPILER) -DINTERNAL -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE3) $(SOURCES) -DLIN64
powder-debug-64: $(SOURCES)
	$(COMPILER) -m64 -o$@ $(FLAGS_DBUG) -DLIN64 $(SOURCES) -Iincludes/
powder-debug: $(SOURCES)
	$(COMPILER) -m32 -o$@ $(FLAGS_DBUG) -DLIN32 $(SOURCES) -Iincludes/
powder-sse3: $(SOURCES)
	$(COMPILER) -m32 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE3) $(SOURCES) -DLIN32
	strip $@
	mv $@ build
powder-sse2: $(SOURCES)
	$(COMPILER) -m32 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE2) $(SOURCES) -DLIN32
	strip $@
powder-sse: $(SOURCES)
	$(COMPILER) -m32 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE) $(SOURCES) -DLIN32
	strip $@
powder-64-sse3-opengl: $(SOURCES)
	$(COMPILER) -m64 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE3) $(SOURCES) -DLIN64 -lGL -lGLU -DOpenGL
	strip $@
powder-64-sse3: $(SOURCES)
	$(COMPILER) -m64 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE3) $(SOURCES) -DLIN64
	strip $@
	mv $@ build
powder-64-sse2: $(SOURCES)
	$(COMPILER) -m64 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE2) $(SOURCES) -DLIN64
	strip $@
powder-res.o: powder-res.rc powder.ico
	i586-mingw32msvc-windres powder-res.rc powder-res.o
powder-sse3.exe: $(SOURCES) powder-res.o
	i586-mingw32msvc-gcc -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE3) $(SOURCES) powder-res.o -lmingw32 -lws2_32 -lSDLmain $(LFLAGS) -mwindows -DWIN32
	strip $@
	chmod 0644 $@
powder-sse2.exe: $(SOURCES) powder-res.o
	i586-mingw32msvc-gcc -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE2) $(SOURCES) powder-res.o -lmingw32 -lws2_32 -lSDLmain $(LFLAGS) -mwindows -DWIN32
	strip $@
	chmod 0644 $@
powder-sse.exe: $(SOURCES) powder-res.o
	i586-mingw32msvc-gcc -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE) $(SOURCES) powder-res.o -lmingw32 -lws2_32 -lSDLmain $(LFLAGS) -mwindows -DWIN32
	strip $@
	chmod 0644 $@
powder-x: $(SOURCES)
	gcc -o $@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS) $(SOURCES) -lSDLmain -DMACOSX -DPIX32BGRA -arch x86_64 -framework Cocoa -ggdb
	strip $@
powder-x-ogl: $(SOURCES)
	gcc -o $@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS) $(SOURCES) -lSDLmain -DOpenGL -DMACOSX -DPIX32BGRA -arch x86_64 -framework Cocoa -framework OpenGL -ggdb
	strip $@
else
UNAME := $(shell uname | cut -c1-10)
ifeq ($(UNAME), MINGW32_NT)
# Is Windows (MinGW) environment
SOURCES := src/*.c
CFLAGS := -Wall -std=c99 -D_POSIX_C_SOURCE=200112L
OFLAGS := -O3 -ffast-math -ftree-vectorize -funsafe-math-optimizations
LFLAGS := -lmingw32 -lSDL.dll -lSDLmain -lSDL -lbz2 -lwsock32 -lws2_32
MFLAGS_SSE3 := -march=native -DX86 -DX86_SSE3 -msse3
MFLAGS_SSE2 := -march=native -DX86 -DX86_SSE2 -msse2
MFLAGS_SSE := -march=native -DX86 -DX86_SSE -msse

powder-sse3_mmx.exe: $(SOURCES) powder-res.o
	gcc -m32 $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE3) -mmmx -mwindows -DWIN32 -o$@ -Iincludes/ $(SOURCES) powder-res.o $(LFLAGS)
	
powder-sse2_mmx.exe: $(SOURCES) powder-res.o
	gcc -m32 $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE2) -mmmx -mwindows -DWIN32 -o$@ -Iincludes/ $(SOURCES) powder-res.o $(LFLAGS)

powder-sse_mmx.exe: $(SOURCES) powder-res.o
	gcc -m32 $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE) -mmmx -mwindows -DWIN32 -o$@ -Iincludes/ $(SOURCES) powder-res.o $(LFLAGS)

powder_mmx.exe: $(SOURCES) powder-res.o
	gcc -m32 $(CFLAGS) $(OFLAGS) -march=native -DX86 -mmmx -mwindows -DWIN32 -o$@ -Iincludes/ $(SOURCES) powder-res.o $(LFLAGS) 

powder-sse3.exe: $(SOURCES) powder-res.o
	gcc -m32 $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE3) -mwindows -DWIN32 -o$@ -Iincludes/ $(SOURCES) powder-res.o $(LFLAGS)
	
powder-sse2.exe: $(SOURCES) powder-res.o
	gcc -m32 $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE2) -mwindows -DWIN32 -o$@ -Iincludes/ $(SOURCES) powder-res.o $(LFLAGS)

powder-sse.exe: $(SOURCES) powder-res.o
	gcc -m32 $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE) -mwindows -DWIN32 -o$@ -Iincludes/ $(SOURCES) powder-res.o $(LFLAGS)

powder.exe: $(SOURCES) powder-res.o
	gcc -m32 $(CFLAGS) $(OFLAGS) -march=native -DX86 -mwindows -DWIN32 -o$@ -Iincludes/ $(SOURCES) powder-res.o $(LFLAGS) 	

powder-res.o: src/Resources/powder-res.rc src/Resources/powder.ico
	windres src/Resources/powder-res.rc powder-res.o
else
# Is other environment
endif
endif
