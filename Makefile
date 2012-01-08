HEADERS := $(wildcard includes/*.h)

OLD_SOURCES := $(wildcard src/*.c)
OLD_OBJS := $(patsubst src/%.c,build/obj/%.o,$(OLD_SOURCES))

EL_SOURCES := $(wildcard elements/*.cpp)
EL_OBJS := $(patsubst elements/%.cpp,build/obj/elements/%.o,$(EL_SOURCES))
EL_PREREQ := $(patsubst build/obj/elements/%.o,build/obj/elements/%.powder.exe.o,$(EL_OBJS))

CORE_SOURCES := $(wildcard src/*.cpp)
CORE_OBJS := $(patsubst src/%.cpp,build/obj/core/%.o,$(CORE_SOURCES))
CORE_PREREQ := $(patsubst build/obj/core/%.o,build/obj/core/%.powder.exe.o,$(CORE_OBJS))

UI_SOURCES := $(wildcard src/interface/*.cpp)
UI_OBJS := $(patsubst src/interface/%.cpp,build/obj/ui/%.o,$(UI_SOURCES))
UI_PREREQ := $(patsubst build/obj/ui/%.o,build/obj/ui/%.powder.exe.o,$(UI_OBJS))

CFLAGS := -Iincludes/ -Idata/ -DWIN32
OFLAGS := -O3 -ffast-math -ftree-vectorize -funsafe-math-optimizations -fkeep-inline-functions
LFLAGS := -lmingw32 -lregex -lws2_32 -lSDLmain -lpthread -lSDL -lm -lbz2 # -mwindows

CFLAGS += $(OFLAGS)

CPPC := g++
CPPC_WIN := i686-w64-mingw32-gcc
WIN_RES := i686-w64-mingw32-windres

build/powder.exe: $(EL_PREREQ) $(CORE_PREREQ) $(UI_PREREQ)
	$(CPPC) $(CFLAGS) $(LDFLAGS) $(EXTRA_OBJS) $(EL_PREREQ) $(CORE_PREREQ) $(UI_PREREQ) $(LFLAGS) -o $@ -ggdb
build/obj/ui/%.powder.exe.o: src/interface/%.cpp $(HEADERS)
	$(CPPC) -c $(CFLAGS) -o $@ $< -ggdb
build/obj/elements/%.powder.exe.o: elements/%.cpp $(HEADERS)
	$(CPPC) -c $(CFLAGS) -o $@ $< -ggdb
build/obj/core/%.powder.exe.o: src/%.cpp $(HEADERS)
	$(CPPC) -c $(CFLAGS) -o $@ $< -ggdb
	
clean:
	rm build/obj/core/*.o
	rm build/obj/ui/*.o
	rm build/obj/elements/*.o
	rm build/obj/*.o
	rm build/*.exe