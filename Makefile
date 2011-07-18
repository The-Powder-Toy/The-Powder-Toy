SOURCES := src/*.c src/elements/*.c

CFLAGS := -w -std=c99 -D_POSIX_C_SOURCE=200112L -Iincludes/ -DLUACONSOLE
OFLAGS := -O3 -ffast-math -ftree-vectorize -funsafe-math-optimizations
LFLAGS := -lpthread -lSDL -lm -lbz2 -lX11 -llua5.1
LFLAGS_X := -lm -lbz2 -lSDLmain -I/Library/Frameworks/Python.framework/Versions/$(PY_VERSION)/include/python$(PY_VERSION)
MFLAGS_SSE3 := -march=native -DX86 -DX86_SSE3 -msse3
MFLAGS_SSE2 := -march=native -DX86 -DX86_SSE2 -msse2
MFLAGS_SSE := -march=native -DX86 -DX86_SSE
FLAGS_DBUG := -Wall -std=c99 -D_POSIX_C_SOURCE=200112L -pg -DX86 -DX86_SSE3 -msse3 -g
COMPILER := gcc
WIN_COMPILER := i586-mingw32-gcc
WIN_RES := i586-mingw32-windres
LINUX_TARG := powder-64-sse2 powder-sse powder-sse2
WIN32_TARG := powder-sse.exe powder-sse2.exe

render: $(SOURCES)
	
	$(COMPILER) -m32 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE3) $(SOURCES) -DLIN32 -DRENDERER
	strip $@
	mv $@ powder-mod

render-x: $(SOURCES)
	
	$(COMPILER) -o $@ $(CFLAGS) $(OFLAGS) $(LFLAGS_X) -lSDL $(MFLAGS) $(SOURCES) -framework Cocoa -DMACOSX -DRENDERER -arch x86_64
	strip $@
	mv $@ powder-mod

powder: $(SOURCES)
	
	$(COMPILER) -DINTERNAL -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE3) $(SOURCES) $(LFLAGS) -DLIN64
	mv $@ mod-me4502
powder-debug-64: $(SOURCES)
	
	$(COMPILER) -m64 -o$@ $(FLAGS_DBUG) -DLIN64 $(SOURCES) -Iincludes/ $(LFLAGS)
	mv $@ powder-mod

powder-debug: $(SOURCES)
	
	$(COMPILER) -DINTERNAL -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE3) $(SOURCES) $(LFLAGS) -DLIN64 $(FLAGS_DBUG)
	mv $@ powder-mod

powder-sse3: $(SOURCES)
	
	$(COMPILER) -m32 -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE3) $(SOURCES) $(LFLAGS) -DLIN32
	strip $@
	mv $@ powder-mod
powder-sse2: $(SOURCES)
	
	$(COMPILER) -m32 -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE2) $(SOURCES) $(LFLAGS) -DLIN32
	strip $@
	mv $@ powder-mod
powder-sse: $(SOURCES)
	
	$(COMPILER) -m32 -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE) $(SOURCES) $(LFLAGS) -DLIN32
	strip $@
	mv $@ powder-mod

powder-64-sse3-opengl: $(SOURCES)
	
	$(COMPILER) -m64 -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE3) $(SOURCES) $(LFLAGS) -DLIN64 -lGL -lGLU -DOpenGL
	strip $@
	mv $@ powder-mod
powder-64-sse3: $(SOURCES)
	
	$(COMPILER) -m64 -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE3) $(SOURCES) $(LFLAGS) -DLIN64
	strip $@
	mv $@ powder-mod
powder-64-sse2: $(SOURCES)
	
	$(COMPILER) -m64 -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE2) $(SOURCES) $(LFLAGS) -DLIN64
	strip $@
	mv $@ powder-mod

powder-icc: $(SOURCES)
	/opt/intel/Compiler/11.1/073/bin/intel64/icc -m64 -o$@ -Iincludes/ -O2 -march=core2 -msse3 -mfpmath=sse -lSDL -lbz2 -lm -xW $(SOURCES) -std=c99 -D_POSIX_C_SOURCE=200112L

powder-res.o: src/Resources/powder-res.rc src/Resources/powder.ico src/Resources/document.ico
	$(WIN_RES) src/Resources/powder-res.rc powder-res.o

powder-sse3.exe: $(SOURCES) powder-res.o
	
	$(WIN_COMPILER) -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE3) $(SOURCES) powder-res.o -lmingw32 -lregex -lws2_32 -lSDLmain $(LFLAGS) -mwindows -DWIN32
	strip $@
	chmod 0644 $@
	mv $@ powder-mod

powder-sse2.exe: $(SOURCES) powder-res.o
	
	$(WIN_COMPILER) -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE2) $(SOURCES) powder-res.o -lmingw32 -lregex -lws2_32 -lSDLmain $(LFLAGS) -mwindows -DWIN32
	strip $@
	chmod 0644 $@
	mv $@ powder-mod

powder-sse.exe: $(SOURCES) powder-res.o
	
	$(WIN_COMPILER) -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE) $(SOURCES) powder-res.o -lmingw32 -lregex -lws2_32 -lSDLmain $(LFLAGS) -mwindows -DWIN32
	strip $@
	chmod 0644 $@
	mv $@ powder-mod

powder-x: $(SOURCES)
	 --64bit
	gcc -o $@ $(CFLAGS) $(OFLAGS) $(LFLAGS_X) $(MFLAGS) $(SOURCES) -DMACOSX -DPIX32BGRA -arch x86_64 -framework Cocoa -framework SDL -framework Python
	strip $@ 
	mv $@ powder-mod/Powder.app/Contents/MacOS/
	./build/Powder.app/Contents/MacOS/powder-x

powder-x-ogl: $(SOURCES)
	 --64bit
	gcc -o $@ $(CFLAGS) $(OFLAGS) $(LFLAGS_X) $(MFLAGS) $(SOURCES) -DOpenGL -DMACOSX -DPIX32BGRA -arch x86_64 -framework Cocoa -framework SDL -framework OpenGL -framework 

clean:
	rm -rf powder
	rm -rf build

all:
	make powder
	
