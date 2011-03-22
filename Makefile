SOURCES := src/*.c src/elements/*.c

CFLAGS := -w -std=c99 -D_POSIX_C_SOURCE=200112L -Iincludes/ 
OFLAGS := -O3 -ffast-math -ftree-vectorize -funsafe-math-optimizations
LFLAGS := -lSDL -lm -lbz2 -lpython2.7 -L/usr/lib/python2.7/config -I/usr/include/python2.7
LFLAGS_X := -lm -lbz2 -lSDLmain -I/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7
MFLAGS_SSE3 := -march=native -DX86 -DX86_SSE3 -msse3
MFLAGS_SSE2 := -march=native -DX86 -DX86_SSE2 -msse2
MFLAGS_SSE := -march=native -DX86 -DX86_SSE
FLAGS_DBUG := -Wall -std=c99 -D_POSIX_C_SOURCE=200112L -pg -DX86 -DX86_SSE3 -msse3 -lSDL -lm -lbz2 -g
COMPILER := gcc
LINUX_TARG := powder-64-sse2 powder-sse powder-sse2
WIN32_TARG := powder-sse.exe powder-sse2.exe
PYCOMMAND := python getheader.py

powder: $(SOURCES)
	$(PYCOMMAND)
	$(COMPILER) -DINTERNAL -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE3) $(SOURCES) -DLIN64
	mv $@ build
powder-debug-64: $(SOURCES)
	$(PYCOMMAND) --64bit
	$(COMPILER) -m64 -o$@ $(FLAGS_DBUG) -DLIN64 $(SOURCES) -Iincludes/
	mv $@ build
powder-debug: $(SOURCES)
	$(PYCOMMAND)
	$(COMPILER) -DINTERNAL -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE3) $(SOURCES) -DLIN64 $(FLAGS_DBUG)
	mv $@ build
powder-sse3: $(SOURCES)
	$(PYCOMMAND)
	$(COMPILER) -m32 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE3) $(SOURCES) -DLIN32
	strip $@
	mv $@ build
powder-sse2: $(SOURCES)
	$(PYCOMMAND)
	$(COMPILER) -m32 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE2) $(SOURCES) -DLIN32
	strip $@
	mv $@ build
powder-sse: $(SOURCES)
	$(PYCOMMAND)
	$(COMPILER) -m32 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE) $(SOURCES) -DLIN32
	strip $@
	mv $@ build
powder-64-sse3-opengl: $(SOURCES)
	$(PYCOMMAND) --64bit
	$(COMPILER) -m64 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE3) $(SOURCES) -DLIN64 -lGL -lGLU -DOpenGL
	strip $@
	mv $@ build
powder-64-sse3: $(SOURCES)
	$(PYCOMMAND) --64bit
	$(COMPILER) -m64 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE3) $(SOURCES) -DLIN64 
	strip $@
	mv $@ build
powder-64-sse2: $(SOURCES)
	$(PYCOMMAND) --64bit
	$(COMPILER) -m64 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE2) $(SOURCES) -DLIN64
	strip $@
	mv $@ build

powder-icc: $(SOURCES)
	/opt/intel/Compiler/11.1/073/bin/intel64/icc -m64 -o$@ -Iincludes/ -O2 -march=core2 -msse3 -mfpmath=sse -lSDL -lbz2 -lm -xW $(SOURCES) -std=c99 -D_POSIX_C_SOURCE=200112L

powder-res.o: powder-res.rc powder.ico
	i586-mingw32msvc-windres powder-res.rc powder-res.o

powder-sse3.exe: $(SOURCES) powder-res.o
	$(PYCOMMAND)
	i586-mingw32msvc-gcc -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE3) $(SOURCES) powder-res.o -lmingw32 -llibregex -lws2_32 -lSDLmain $(LFLAGS) -mwindows -DWIN32
	strip $@
	chmod 0644 $@
	mv $@ build
powder-sse2.exe: $(SOURCES) powder-res.o
	$(PYCOMMAND)
	i586-mingw32msvc-gcc -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE2) $(SOURCES) powder-res.o -lmingw32 -llibregex -lws2_32 -lSDLmain $(LFLAGS) -mwindows -DWIN32
	strip $@
	chmod 0644 $@
	mv $@ build
powder-sse.exe: $(SOURCES) powder-res.o
	$(PYCOMMAND)
	i586-mingw32msvc-gcc -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE) $(SOURCES) powder-res.o -lmingw32 -llibregex -lws2_32 -lSDLmain $(LFLAGS) -mwindows -DWIN32
	strip $@
	chmod 0644 $@
	mv $@ build
powder-x: $(SOURCES)
	$(PYCOMMAND) --64bit
	gcc -o $@ $(CFLAGS) $(OFLAGS) $(LFLAGS_X) $(MFLAGS) $(SOURCES) -DMACOSX -DPIX32BGRA -arch x86_64 -framework Cocoa -framework SDL -framework Python
	strip $@ 
	mv $@ build/Powder.app/Contents/MacOS/
powder-x-ogl: $(SOURCES)
	$(PYCOMMAND) --64bit
	gcc -o $@ $(CFLAGS) $(OFLAGS) $(LFLAGS_X) $(MFLAGS) $(SOURCES) -DOpenGL -DMACOSX -DPIX32BGRA -arch x86_64 -framework Cocoa -framework SDL -framework OpenGL -framework Python
	strip $@ 
	mv $@ build
