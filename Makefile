HEADERS := $(wildcard src/*.h) $(wildcard src/*/*.h)

SOURCES := $(wildcard src/*.cpp) $(wildcard src/*/*.cpp)
OBJS := $(patsubst src/%.cpp,build/obj/%.o,$(SOURCES))

FOLDERS := 

CFLAGS := -w -Isrc/ -Idata/
OFLAGS := -fkeep-inline-functions -O3 -ffast-math -ftree-vectorize -funsafe-math-optimizations -msse2

CPPC := g++
CPPC_WIN := g++
WIN_RES := windres

all: build/powder

powder-release.exe: build/powder-release.exe
powder.exe: build/powder.exe
powder-release: build/powder-release
powder: build/powder

build/powder-release.exe: CFLAGS +=  -DWIN32
build/powder-release.exe: LFLAGS := -lmingw32 -lregex -lws2_32 -lSDLmain -lpthread -lSDL -lm -lbz2 -mwindows
build/powder.exe: CFLAGS +=  -DWIN32 -DWINCONSOLE
build/powder.exe: LFLAGS := -lmingw32 -lregex -lws2_32 -lSDLmain -lpthread -lSDL -lm -lbz2 #-mwindows
build/powder-release: CFLAGS +=  -DLIN32
build/powder-release: LFLAGS := -lSDL -lm -lbz2
build/powder: CFLAGS +=  -DLIN32
build/powder: LFLAGS := -lSDL -lm -lbz2

build/powder-release.exe: $(SOURCES) build/powder-res.o
	$(CPPC_WIN) $(CFLAGS) $(OFLAGS) $(LDFLAGS) $(SOURCES) $(LFLAGS) build/powder-res.o -o $@
	strip $@
build/powder-res.o: resources/powder-res.rc resources/powder.ico resources/document.ico 
	cd resources && $(WIN_RES) powder-res.rc powder-res.o
	mv resources/powder-res.o build/powder-res.o
build/powder-release: $(SOURCES)
	$(CPPC) $(CFLAGS) $(OFLAGS) $(LDFLAGS) $(SOURCES) $(LFLAGS) -o $@
	strip $@

build/powder.exe: buildpaths-powder.exe $(patsubst build/obj/%.o,build/obj/powder.exe/%.o,$(OBJS))
	$(CPPC_WIN) $(CFLAGS) $(OFLAGS) $(LDFLAGS) $(patsubst build/obj/%.o,build/obj/powder.exe/%.o,$(OBJS)) $(LFLAGS) -o $@ -ggdb
build/obj/powder.exe/%.o: src/%.cpp $(HEADERS)
	$(CPPC_WIN) -c $(CFLAGS) $(OFLAGS) -o $@ $< -ggdb
buildpaths-powder.exe:
	$(shell mkdir build/obj/powder.exe/)
	$(shell mkdir $(sort $(dir $(patsubst build/obj/%.o,build/obj/powder.exe/%.o,$(OBJS)))))

build/powder: buildpaths-powder $(patsubst build/obj/%.o,build/obj/powder/%.o,$(OBJS))
	$(CPPC) $(CFLAGS) $(OFLAGS) $(LDFLAGS) $(patsubst build/obj/%.o,build/obj/powder/%.o,$(OBJS)) $(LFLAGS) -o $@ -ggdb
build/obj/powder/%.o: src/%.cpp $(HEADERS)
	$(CPPC) -c $(CFLAGS) $(OFLAGS) -o $@ $< -ggdb
buildpaths-powder:
	$(shell mkdir build/obj/powder/)
	$(shell mkdir $(sort $(dir $(patsubst build/obj/%.o,build/obj/powder/%.o,$(OBJS)))))
	
clean:
	rm -r build/obj/*
	rm build/*.exe
