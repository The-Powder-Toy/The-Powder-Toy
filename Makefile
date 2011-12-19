SOURCES := $(wildcard src/*.c src/elements/*.c)
HEADERS := $(wildcard includes/*.h)
OBJS := $(patsubst src/%.c,build/obj/%.o,$(SOURCES))

CFLAGS := -w -std=c99 -D_POSIX_C_SOURCE=200112L -DLUACONSOLE -DGRAVFFT -Iincludes/ -D_GNU_SOURCE
OFLAGS := -O3 -ffast-math -ftree-vectorize -funsafe-math-optimizations
LFLAGS := -lpthread -lSDL -lfftw3f -lm -lbz2 -lX11 -llua5.1 -lrt
LFLAGS_X := -lm -lbz2 -lSDLmain
LFLAGS_WIN := -lmingw32 -lgnurx -lws2_32 -lSDLmain -lpthread -lSDL -lfftw3f -lm -lbz2 -llua5.1
MFLAGS_SSE3 := -march=native -DX86 -DX86_SSE3 -msse3
MFLAGS_SSE2 := -march=native -DX86 -DX86_SSE2 -msse2
MFLAGS_SSE := -march=native -DX86 -DX86_SSE
FLAGS_DBUG := -Wall -pg -g
COMPILER := gcc
LINUX_TARG := powder-64-sse2 powder-sse powder-sse2
WIN32_TARG := powder-sse.exe powder-sse2.exe

CC := gcc
CC_WIN := i686-w64-mingw32-gcc
WIN_RES := i686-w64-mingw32-windres

powder: build/powder
powder-debug: build/powder-debug
powder-sse3: build/powder-sse3
powder-sse2: build/powder-sse2
powder-sse: build/powder-sse
powder-sse3-opengl: build/powder-sse3-opengl
powder-64-sse3: build/powder-64-sse3
powder-64-sse2: build/powder-64-sse2
powder-debug-64: build/powder-debug-64
powder-64-sse3-opengl: build/powder-64-sse3-opengl
powder-sse3.exe: build/powder-sse3.exe
powder-sse2.exe: build/powder-sse2.exe
powder-sse.exe: build/powder-sse.exe

# general compiler flags
build/powder: CFLAGS += -DINTERNAL -DLIN64 $(OFLAGS)
build/powder-debug: CFLAGS += -m32 -DLIN32 $(FLAGS_DBUG)
build/powder-sse3 build/powder-sse2 build/powder-sse build/powder-sse3-opengl: CFLAGS += -m32 -DLIN32 $(OFLAGS)
build/powder-64-sse3 build/powder-64-sse2 build/powder-64-sse3-opengl: CFLAGS += -m64 -DLIN64 $(OFLAGS)
build/powder-debug-64: CFLAGS += -m64 -DLIN64 $(FLAGS_DBUG)
build/powder-sse3.exe build/powder-sse2.exe build/powder-sse.exe: CFLAGS += -mwindows -DWIN32 $(OFLAGS)
build/powder-64-sse3-opengl build/powder-sse3-opengl: CFLAGS += -DOGLR -DPIX32OGL -DPIXALPHA

# SSE flags:
build/powder build/powder-sse3 build/powder-sse3-opengl build/powder-64-sse3 build/powder-64-sse3-opengl build/powder-debug build/powder-debug-64 build/powder-sse3.exe: CFLAGS += -march=native -DX86 -DX86_SSE3 -msse3
build/powder-sse2 build/powder-64-sse2 build/powder-sse2.exe: CFLAGS += -march=native -DX86 -DX86_SSE2 -msse2
build/powder-sse build/powder-sse.exe: CFLAGS += -march=native -DX86 -DX86_SSE

# libs:
build/powder build/powder-debug build/powder-sse3 build/powder-sse2 build/powder-sse build/powder-sse3-opengl build/powder-debug-64 build/powder-64-sse3 build/powder-64-sse2 build/powder-64-sse3-opengl: LIBS += $(LFLAGS)
build/powder-sse3.exe build/powder-sse2.exe build/powder-sse.exe: LIBS += $(LFLAGS_WIN)
build/powder-64-sse3-opengl build/powder-sse3-opengl: LIBS += -lGL

# extra windows stuff
build/powder-sse3.exe build/powder-sse2.exe build/powder-sse.exe: EXTRA_OBJS += build/obj/powder-res.o
build/powder-sse3.exe build/powder-sse2.exe build/powder-sse.exe: CC := $(CC_WIN)
build/powder-sse3.exe build/powder-sse2.exe build/powder-sse.exe: build/obj/powder-res.o

build/powder: $(patsubst build/obj/%.o,build/obj/%.powder.o,$(OBJS))
	$(CC) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder.o,$(OBJS)) $(LIBS) -o $@
build/obj/%.powder.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<

build/powder-debug: $(patsubst build/obj/%.o,build/obj/%.powder-debug.o,$(OBJS))
	$(CC) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-debug.o,$(OBJS)) $(LIBS) -o $@
build/obj/%.powder-debug.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<

build/powder-sse3: $(patsubst build/obj/%.o,build/obj/%.powder-sse3.o,$(OBJS))
	$(CC) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-sse3.o,$(OBJS)) $(LIBS) -o $@
	strip $@
build/obj/%.powder-sse3.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<

build/powder-sse2: $(patsubst build/obj/%.o,build/obj/%.powder-sse2.o,$(OBJS))
	$(CC) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-sse2.o,$(OBJS)) $(LIBS) -o $@
	strip $@
build/obj/%.powder-sse2.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<

build/powder-sse: $(patsubst build/obj/%.o,build/obj/%.powder-sse.o,$(OBJS))
	$(CC) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-sse.o,$(OBJS)) $(LIBS) -o $@
	strip $@
build/obj/%.powder-sse.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<

build/powder-sse3-opengl: $(patsubst build/obj/%.o,build/obj/%.powder-sse3-opengl.o,$(OBJS))
	$(CC) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-sse3-opengl.o,$(OBJS)) $(LIBS) -o $@
	strip $@
build/obj/%.powder-sse3-opengl.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<


build/powder-64-sse3: $(patsubst build/obj/%.o,build/obj/%.powder-64-sse3.o,$(OBJS))
	$(CC) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-64-sse3.o,$(OBJS)) $(LIBS) -o $@
	strip $@
build/obj/%.powder-64-sse3.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<

build/powder-64-sse2: $(patsubst build/obj/%.o,build/obj/%.powder-64-sse2.o,$(OBJS))
	$(CC) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-64-sse2.o,$(OBJS)) $(LIBS) -o $@
	strip $@
build/obj/%.powder-64-sse2.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<

build/powder-debug-64: $(patsubst build/obj/%.o,build/obj/%.powder-debug-64.o,$(OBJS))
	$(CC) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-debug-64.o,$(OBJS)) $(LIBS) -o $@
build/obj/%.powder-debug-64.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<

build/powder-64-sse3-opengl: $(patsubst build/obj/%.o,build/obj/%.powder-64-sse3-opengl.o,$(OBJS))
	$(CC) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-64-sse3-opengl.o,$(OBJS)) $(LIBS) -o $@
	strip $@
build/obj/%.powder-64-sse3-opengl.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<

build/powder-sse3.exe: $(patsubst build/obj/%.o,build/obj/%.powder-sse3.exe.o,$(OBJS))
	$(CC) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-sse3.exe.o,$(OBJS)) $(LIBS) -o $@
	strip $@
	chmod 0644 $@
build/obj/%.powder-sse3.exe.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<

build/powder-sse2.exe: $(patsubst build/obj/%.o,build/obj/%.powder-sse2.exe.o,$(OBJS))
	$(CC) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-sse2.exe.o,$(OBJS)) $(LIBS) -o $@
	strip $@
	chmod 0644 $@
build/obj/%.powder-sse2.exe.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<

build/powder-sse.exe: $(patsubst build/obj/%.o,build/obj/%.powder-sse.exe.o,$(OBJS))
	$(CC) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(patsubst build/obj/%.o,build/obj/%.powder-sse.exe.o,$(OBJS)) $(LIBS) -o $@
	strip $@
	chmod 0644 $@
build/obj/%.powder-sse.exe.o: src/%.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<



.PHONY: clean

clean:
	rm -f build/obj/*.o
	rm -f build/obj/elements/*.o



powder-icc: $(SOURCES)
	/opt/intel/Compiler/11.1/073/bin/intel64/icc -m64 -o$@ -Iincludes/ -O2 -march=core2 -msse3 -mfpmath=sse -lSDL -lbz2 -lm -xW $(SOURCES) -std=c99 -D_POSIX_C_SOURCE=200112L

build/obj/powder-res.o: src/Resources/powder-res.rc src/Resources/powder.ico src/Resources/document.ico
	cd src/Resources && $(WIN_RES) powder-res.rc powder-res.o
	mv src/Resources/powder-res.o build/obj/powder-res.o

powder-x: $(SOURCES)
	gcc -o $@ $(CFLAGS) $(OFLAGS) $(LFLAGS_X) $(MFLAGS) $(SOURCES) -DMACOSX -DPIX32BGRA -arch x86_64 -framework Cocoa -framework SDL
	strip $@ 
	mv $@ build
powder-x-ogl: $(SOURCES)
	gcc -o $@ $(CFLAGS) $(OFLAGS) $(LFLAGS_X) $(MFLAGS) $(SOURCES) -DOpenGL -DMACOSX -DPIX32BGRA -arch x86_64 -framework Cocoa -framework SDL -framework OpenGL
	strip $@ 
	mv $@ build
