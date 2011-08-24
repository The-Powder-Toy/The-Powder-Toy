SOURCES := src/*.c src/elements/*.c

PY_BIN := echo python
PY_VERSION := $(shell $(PY_BIN) -c "import sys;print sys.version[:3]")
PY_LIBPATH := $(shell $(PY_BIN) -c "import os.path,sys;print os.path.join(sys.exec_prefix,\"lib\",\"python%s\"%sys.version[:3],\"config\")")
PY_INCPATH := $(shell $(PY_BIN) -c "import os.path,sys;print os.path.join(sys.exec_prefix,\"include\",\"python%s\"%sys.version[:3])")
PY_LDFLAGS := $(shell $(PY_BIN) -c "import distutils.sysconfig;print distutils.sysconfig.get_config_var('LINKFORSHARED')")
PYCOMMAND := $(PY_BIN) getheader.py

CFLAGS := -w -std=c99 -D_POSIX_C_SOURCE=200112L -DLUACONSOLE -DGRAVFFT -Iincludes/ -D_GNU_SOURCE
OFLAGS := -O3 -ffast-math -ftree-vectorize -funsafe-math-optimizations
LFLAGS := -lpthread -lSDL -lfftw3f -lm -lbz2 -lX11 -llua5.1 #-lpython$(PY_VERSION) -L$(PY_LIBPATH) -I$(PY_INCPATH) $(PY_LDFLAGS)
LFLAGS_X := -lm -lbz2 -lSDLmain -I/Library/Frameworks/Python.framework/Versions/$(PY_VERSION)/include/python$(PY_VERSION)
MFLAGS_SSE3 := -march=native -DX86 -DX86_SSE3 -msse3
MFLAGS_SSE2 := -march=native -DX86 -DX86_SSE2 -msse2
MFLAGS_SSE := -march=native -DX86 -DX86_SSE
FLAGS_DBUG := -Wall -std=c99 -D_POSIX_C_SOURCE=200112L -pg -DX86 -DX86_SSE3 -msse3 -g
COMPILER := gcc
WIN_COMPILER := gcc
WIN_RES := windres
LINUX_TARG := powder-64-sse2 powder-sse powder-sse2
WIN32_TARG := powder-sse.exe powder-sse2.exe

powder: $(SOURCES)
	$(PYCOMMAND)
	$(COMPILER) -DINTERNAL -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE3) $(SOURCES) $(LFLAGS) -DLIN64
	mv $@ build

powder-debug-64: $(SOURCES)
	$(PYCOMMAND)
	$(COMPILER) -m64 -o$@ $(FLAGS_DBUG) -DLIN64 $(SOURCES) -Iincludes/ $(LFLAGS)
	mv $@ build

powder-debug: $(SOURCES)
	$(PYCOMMAND)
	$(COMPILER) -DINTERNAL -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE3) $(SOURCES) $(LFLAGS) -DLIN64 $(FLAGS_DBUG)
	mv $@ build

powder-sse3: $(SOURCES)
	$(PYCOMMAND)
	$(COMPILER) -m32 -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE3) $(SOURCES) $(LFLAGS) -DLIN32
	strip $@
	mv $@ build
powder-sse2: $(SOURCES)
	$(PYCOMMAND)
	$(COMPILER) -m32 -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE2) $(SOURCES) $(LFLAGS) -DLIN32
	strip $@
	mv $@ build
powder-sse: $(SOURCES)
	$(PYCOMMAND)
	$(COMPILER) -m32 -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE) $(SOURCES) $(LFLAGS) -DLIN32
	strip $@
	mv $@ build

powder-64-sse3-opengl: $(SOURCES)
	$(PYCOMMAND)
	$(COMPILER) -m64 -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE3) $(SOURCES) $(LFLAGS) -DLIN64 -lGL -lGLU -DOpenGL
	strip $@
	mv $@ build
powder-sse3-opengl: $(SOURCES)
	$(PYCOMMAND)
	$(COMPILER) -m32 -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE3) $(SOURCES) $(LFLAGS) -DLIN32 -lGL -lGLU -DOpenGL
	strip $@
	mv $@ build
powder-64-sse3: $(SOURCES)
	$(PYCOMMAND)
	$(COMPILER) -m64 -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE3) $(SOURCES) $(LFLAGS) -DLIN64
	strip $@
	mv $@ build
powder-64-sse2: $(SOURCES)
	$(PYCOMMAND)
	$(COMPILER) -m64 -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE2) $(SOURCES) $(LFLAGS) -DLIN64
	strip $@
	mv $@ build

powder-icc: $(SOURCES)
	/opt/intel/Compiler/11.1/073/bin/intel64/icc -m64 -o$@ -Iincludes/ -O2 -march=core2 -msse3 -mfpmath=sse -lSDL -lbz2 -lm -xW $(SOURCES) -std=c99 -D_POSIX_C_SOURCE=200112L

powder-res.o: src/Resources/powder-res.rc src/Resources/powder.ico src/Resources/document.ico
	$(WIN_RES) src/Resources/powder-res.rc powder-res.o

powder-sse3.exe: $(SOURCES) powder-res.o
	$(PYCOMMAND)
	$(WIN_COMPILER) -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE3) $(SOURCES) powder-res.o -lm -lmingw32 -lregex -lws2_32 -lSDLmain $(LFLAGS) -mwindows -DWIN32
	strip $@
	chmod 0644 $@
	mv $@ build

powder-sse2.exe: $(SOURCES) powder-res.o
	$(PYCOMMAND)
	$(WIN_COMPILER) -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE2) $(SOURCES) powder-res.o -lm -lmingw32 -lregex -lws2_32 -lSDLmain $(LFLAGS) -mwindows -DWIN32 -ggdb
	strip $@
	chmod 0644 $@
	mv $@ build

powder-sse.exe: $(SOURCES) powder-res.o
	$(PYCOMMAND)
	$(WIN_COMPILER) -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE) $(SOURCES) powder-res.o -lm -lmingw32 -lregex -lws2_32 -lSDLmain $(LFLAGS) -mwindows -DWIN32
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

render: $(SOURCES)
	$(PYCOMMAND)
	$(COMPILER) -m32 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE3) $(SOURCES) -DLIN32 -DRENDERER
	strip $@
	mv $@ build

render-x: $(SOURCES)
	$(PYCOMMAND)
	$(COMPILER) -o $@ $(CFLAGS) $(OFLAGS) $(LFLAGS_X) -lSDL $(MFLAGS) $(SOURCES) -framework Cocoa -DMACOSX -DRENDERER -arch x86_64
	strip $@
	mv $@ build
