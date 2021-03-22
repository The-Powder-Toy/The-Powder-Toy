SOURCES := powder.c http.c md5.c update.c
HEADERS := font.h http.h md5.h icon.h update.h fbi.h

CFLAGS := -Wall -std=c99 -D_POSIX_C_SOURCE=200112L -fgnu89-inline
OFLAGS := -O3 -ffast-math -ftree-vectorize -funsafe-math-optimizations
LFLAGS := -lSDL -lm -lbz2 
MFLAGS_SSE3 := -march=k8 -DX86 -DX86_SSE3 -msse3
MFLAGS_SSE2 := -march=k8 -DX86 -DX86_SSE2
MFLAGS_SSE := -march=pentium3 -DX86 -DX86_SSE

LINUX_TARG := powder-64-sse2 powder-sse powder-sse2
WIN32_TARG := powder-sse.exe powder-sse2.exe

powder: $(SOURCES) $(HEADERS)
	gcc -m64 -DINTERNAL -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE3) $(SOURCES) -DLIN64

powder-sse3: $(SOURCES) $(HEADERS)
	gcc -m32 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE3) $(SOURCES) -DLIN32
	strip $@
powder-sse2: $(SOURCES) $(HEADERS)
	gcc -m32 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE2) $(SOURCES) -DLIN32
	strip $@
powder-sse: $(SOURCES) $(HEADERS)
	gcc -m32 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE) $(SOURCES) -DLIN32
	strip $@

powder-64-sse3: $(SOURCES) $(HEADERS)
	gcc -m64 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE3) $(SOURCES) -DLIN64
	strip $@
powder-64-sse2: $(SOURCES) $(HEADERS)
	gcc -m64 -o$@ $(CFLAGS) $(OFLAGS) $(LFLAGS) $(MFLAGS_SSE2) $(SOURCES) -DLIN64
	strip $@

powder-res.o: powder-res.rc powder.ico
	i586-mingw32msvc-windres powder-res.rc powder-res.o

powder-sse3.exe: $(SOURCES) $(HEADERS) powder-res.o
	i586-mingw32msvc-gcc -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE3) $(SOURCES) powder-res.o -lmingw32 -lws2_32 -lSDLmain $(LFLAGS) -mwindows -DWIN32
	strip $@
	chmod 0644 $@
powder-sse2.exe: $(SOURCES) $(HEADERS) powder-res.o
	i586-mingw32msvc-gcc -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE2) $(SOURCES) powder-res.o -lmingw32 -lws2_32 -lSDLmain $(LFLAGS) -mwindows -DWIN32
	strip $@
	chmod 0644 $@
powder-sse.exe: $(SOURCES) $(HEADERS) powder-res.o
	i586-mingw32msvc-gcc -o$@ $(CFLAGS) $(OFLAGS) $(MFLAGS_SSE) $(SOURCES) powder-res.o -lmingw32 -lws2_32 -lSDLmain $(LFLAGS) -mwindows -DWIN32
	strip $@
	chmod 0644 $@

powder-src.tar.bz2: *.c *.h *.rc *.ico Makefile
	mkdir powder-src
	cp *.c *.h *.rc *.ico Makefile powder-src/
	tar cfj powder-src.tar.bz2 powder-src
	rm -rf powder-src

release: $(LINUX_TARG) $(WIN32_TARG) powder-src.tar.bz2
	tar cfz powder-linux.tar.gz $(LINUX_TARG)
	cp /usr/i586-mingw32msvc/bin/SDL.dll .
	zip powder-win32.zip $(WIN32_TARG) SDL.dll
	mkdir -p release
	mv powder-linux.tar.gz release/
	mv powder-win32.zip release/
	mv powder-src.tar.bz2 release/
	cp powder-sse.exe powder.exe
	rm -f release/powder.zip
	zip release/powder.zip powder.exe SDL.dll
	cp powder-64-sse2 release/powder64
	rm -f release/powder64.gz
	gzip release/powder64
	rm -f release/powder.gz
	cp powder-sse release/powder
	gzip release/powder
	cd release; tar czf powder-bin.tar.gz powder.gz powder64.gz powder-linux.tar.gz powder-win32.zip powder.zip; cd ..
	rm -f $(LINUX_TARG) $(WIN32_TARG) SDL.dll powder.exe
