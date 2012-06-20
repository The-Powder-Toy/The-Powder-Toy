HEADERS := $(wildcard src/*.h) $(wildcard src/*/*.h) $(wildcard generated/*.h)

SOURCES := $(wildcard src/*.cpp) $(wildcard src/*/*.cpp) $(wildcard src/*/*/*.cpp) $(wildcard generated/*.cpp)
GENERATEDSOURCES := $(wildcard src/*/*/*.h) $(wildcard src/*/*/*.cpp)
OBJS := $(patsubst src/%.cpp,build/obj/%.o,$(SOURCES))

CFLAGS := -w -Isrc/ -Idata/ -Igenerated/
OFLAGS := -fkeep-inline-functions 

CPPC := g++
CPPC_WIN := g++
WIN_RES := windres

all: build/powder

powder-release.exe: build/powder-release.exe
powder.exe: build/powder.exe
powder-release: build/powder-release
powder-opengl-release: build/powder-opengl-release
powder: build/powder
powder-x: build/powder-x
powder-x.jnilib: build/powder-x.jnilib

build/powder-release.exe: CFLAGS += -DWIN32 -O3 -ftree-vectorize -msse2 -funsafe-math-optimizations -ffast-math -fomit-frame-pointer -funsafe-loop-optimizations -Wunsafe-loop-optimizations
build/powder-release.exe: LFLAGS := -lmingw32 -lregex -lws2_32 -lSDLmain -lpthread -lSDL -lm -lbz2 -llua -lfftw3f-3 -mwindows
build/powder.exe: CFLAGS += -DWIN32
build/powder.exe: LFLAGS := -lmingw32 -lregex -lws2_32 -lSDLmain -lpthread -lSDL -lm -lbz2 -llua -lfftw3f-3 #-mwindows
build/powder-release: CFLAGS +=  -DLIN32 -O3 -ftree-vectorize -msse3 -funsafe-math-optimizations -ffast-math -fomit-frame-pointer -funsafe-loop-optimizations -Wunsafe-loop-optimizations
build/powder-release: LFLAGS := -lSDL -lm -lbz2 -llua -lfftw3f
build/powder-opengl-release: CFLAGS +=  -DLIN32 -O3 -ftree-vectorize -msse3 -funsafe-math-optimizations -ffast-math -fomit-frame-pointer -funsafe-loop-optimizations -Wunsafe-loop-optimizations -DOGLR -DPIX32OGL -DPIXALPHA
build/powder-opengl-release: LFLAGS := -lSDL -lm -lbz2 -llua -lfftw3f -lGL -lGLEW -DOGLR -DPIX32OGL -DPIXALPHA
build/powder: CFLAGS +=  -DLIN32
build/powder: LFLAGS := -lSDL -lm -lbz2 -llua -lfftw3f 
#build/powder-x: CFLAGS += -DMACOSX -I/Library/Frameworks/SDL.framework/Headers -I/Library/Frameworks/Lua.framework/Headers -I/Library/Frameworks/OpenGL.framework/Headers -DOGLR -DPIX32OGL -DPIXALPHA
#build/powder-x: LFLAGS := -lm -lbz2 -lfftw3f -framework SDL -framework Lua -framework Cocoa -framework OpenGL
build/powder-x: CFLAGS += -DMACOSX -I/Library/Frameworks/SDL.framework/Headers -I/Library/Frameworks/Lua.framework/Headers -DPIX32BGRA
build/powder-x: LFLAGS := -lm -lbz2 -lfftw3f -framework SDL -framework Lua -framework Cocoa
build/powder-x.jnilib: CFLAGS += -DMACOSX -DUSE_JNI -I/Library/Frameworks/Lua.framework/Headers -I/System/Library/Frameworks/JavaVM.framework/Headers -DOGLR -DPIX32OGL -DPIXALPHA
build/powder-x.jnilib: LFLAGS := -lm -lbz2 -lfftw3f -framework Lua -framework JavaVM -framework Cocoa -framework OpenGL

CFLAGS += -DGRAVFFT -DLUACONSOLE -DUSE_SDL

build/powder-release.exe: $(SOURCES) generate build/powder-res.o
	$(CPPC_WIN) $(CFLAGS) $(OFLAGS) $(LDFLAGS) $(SOURCES) $(LFLAGS) build/powder-res.o -o $@
	strip $@
build/powder-res.o: resources/powder-res.rc resources/powder.ico resources/document.ico 
	cd resources && $(WIN_RES) powder-res.rc powder-res.o
	mv resources/powder-res.o build/powder-res.o
build/powder-release: $(SOURCES)
	$(CPPC) $(CFLAGS) $(OFLAGS) $(LDFLAGS) $(SOURCES) $(LFLAGS) -o $@
	strip $@
build/powder-opengl-release: $(SOURCES)
	$(CPPC) $(CFLAGS) $(OFLAGS) $(LDFLAGS) $(SOURCES) $(LFLAGS) -o $@
	strip $@
build/powder.exe: buildpaths-powder.exe generate $(patsubst build/obj/%.o,build/obj/powder.exe/%.o,$(OBJS)) build/powder-res.o
	$(CPPC_WIN) $(CFLAGS) $(OFLAGS) $(LDFLAGS) $(patsubst build/obj/%.o,build/obj/powder.exe/%.o,$(OBJS)) build/powder-res.o $(LFLAGS) -o $@ -ggdb
build/obj/powder.exe/%.o: src/%.cpp $(HEADERS)
	$(CPPC_WIN) -c $(CFLAGS) $(OFLAGS) -o $@ $< -ggdb
buildpaths-powder.exe:
	$(shell mkdir -p build/obj/powder.exe/)
	$(shell mkdir -p $(sort $(dir $(patsubst build/obj/%.o,build/obj/powder.exe/%.o,$(OBJS)))))

build/powder: buildpaths-powder generate $(patsubst build/obj/%.o,build/obj/powder/%.o,$(OBJS))
	$(CPPC) $(CFLAGS) $(OFLAGS) $(LDFLAGS) $(patsubst build/obj/%.o,build/obj/powder/%.o,$(OBJS)) $(LFLAGS) -o $@ -ggdb
build/obj/powder/%.o: src/%.cpp $(HEADERS)
	$(CPPC) -c $(CFLAGS) $(OFLAGS) -o $@ $< -ggdb
buildpaths-powder:
	$(shell mkdir -p build/obj/powder/)
	$(shell mkdir -p $(sort $(dir $(patsubst build/obj/%.o,build/obj/powder/%.o,$(OBJS)))))

build/powder-x: buildpaths-powder-x generate $(patsubst build/obj/%.o,build/obj/powder-x/%.o,$(OBJS))
	$(CPPC) $(CFLAGS) $(OFLAGS) $(LDFLAGS) $(patsubst build/obj/%.o,build/obj/powder-x/%.o,$(OBJS)) SDLmain.m $(LFLAGS) -o $@ -ggdb
build/obj/powder-x/%.o: src/%.cpp $(HEADERS)
	$(CPPC) -c $(CFLAGS) $(OFLAGS) -o $@ $< -ggdb
buildpaths-powder-x:
	$(shell mkdir -p build/obj/powder-x/)
	$(shell mkdir -p $(sort $(dir $(patsubst build/obj/%.o,build/obj/powder-x/%.o,$(OBJS)))))

build/powder-x.jnilib: buildpaths-powder-x.jnilib generate $(patsubst build/obj/%.o,build/obj/powder-x.jnilib/%.o,$(OBJS))
	$(CPPC) -dynamiclib $(CFLAGS) $(OFLAGS) $(LDFLAGS) $(patsubst build/obj/%.o,build/obj/powder-x.jnilib/%.o,$(OBJS)) src/powdertoyjava/OpenGLCanvasMacOS.mm $(LFLAGS) -o $@ -ggdb
build/obj/powder-x.jnilib/%.o: src/%.cpp $(HEADERS)
	$(CPPC) -c $(CFLAGS) $(OFLAGS) -o $@ $< -ggdb
buildpaths-powder-x.jnilib:
	$(shell mkdir -p build/obj/powder-x.jnilib/)
	$(shell mkdir -p $(sort $(dir $(patsubst build/obj/%.o,build/obj/powder-x.jnilib/%.o,$(OBJS)))))
	
generate: $(GENERATEDSOURCES)
	touch generate
	python generator.py

clean:
	rm -r build/obj/*
	rm build/*.exe
