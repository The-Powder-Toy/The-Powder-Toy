HEADERS := $(wildcard src/*.h) $(wildcard src/*/*.h)

SOURCES := $(wildcard src/*.cpp) $(wildcard src/*/*.cpp)
OBJS += $(patsubst src/%.cpp,build/obj/powder.exe/%.o,$(SOURCES))

FOLDERS := $(sort $(dir $(OBJS)))

CFLAGS := -w -Isrc/ -Idata/ -DWIN32 -DWINCONSOLE
OFLAGS := -fkeep-inline-functions #-O3 -ffast-math -ftree-vectorize -funsafe-math-optimizations -msse2
LFLAGS := -lmingw32 -lregex -lws2_32 -lSDLmain -lpthread -lSDL -lm -lbz2 # -mwindows

CFLAGS += $(OFLAGS)

CPPC := g++
CPPC_WIN := i686-w64-mingw32-gcc
WIN_RES := i686-w64-mingw32-windres

all: build/powder.exe
powder.exe: build/powder.exe


build/powder.exe: buildpaths $(OBJS)
	echo $(OBJS)
	$(CPPC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LFLAGS) -o $@ -ggdb
build/obj/powder.exe/%.o: src/%.cpp $(HEADERS)
	$(CPPC) -c $(CFLAGS) -o $@ $< -ggdb
buildpaths:
	$(shell mkdir build/obj/powder.exe/)
	$(shell mkdir $(FOLDERS))
	
clean:
	rm build/obj/core/*.o
	rm build/obj/ui/*.o
	rm build/obj/elements/*.o
	rm build/obj/*.o
	rm build/*.exe