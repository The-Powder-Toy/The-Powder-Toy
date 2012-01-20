HEADERS := $(wildcard src/*.h) $(wildcard src/*/*.h)

SOURCES := $(wildcard src/*.cpp) $(wildcard src/*/*.cpp)
OBJS := $(patsubst src/%.cpp,build/obj/%.o,$(SOURCES))

FOLDERS := 

CFLAGS := -w -Isrc/ -Idata/
OFLAGS := -fkeep-inline-functions #-O3 -ffast-math -ftree-vectorize -funsafe-math-optimizations -msse2

CPPC := g++
CPPC_WIN := i686-w64-mingw32-gcc
WIN_RES := i686-w64-mingw32-windres

all: build/powder

powder.exe: build/powder.exe
powder: build/powder

build/powder.exe: CFLAGS +=  -DWIN32 -DWINCONSOLE
build/powder.exe: LFLAGS := -lmingw32 -lregex -lws2_32 -lSDLmain -lpthread -lSDL -lm -lbz2 #-mwindows
build/powder: CFLAGS +=  -DLIN32
build/powder: LFLAGS := -lSDL -lm -lbz2



build/powder.exe: buildpaths-powder.exe $(patsubst build/obj/%.o,build/obj/powder.exe/%.o,$(OBJS))
	$(CPPC) $(CFLAGS) $(OFLAGS) $(LDFLAGS) $(patsubst build/obj/%.o,build/obj/powder.exe/%.o,$(OBJS)) $(LFLAGS) -o $@ -ggdb
build/obj/powder.exe/%.o: src/%.cpp $(HEADERS)
	$(CPPC) -c $(CFLAGS) $(OFLAGS) -o $@ $< -ggdb
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
