#
# Makefile for installing the Mingw32 version of the SDL library

CROSS_PATH := /usr/local/cross-tools/i686-w64-mingw32

all:
	@echo "Type \"make native\" to install to /usr"
	@echo "Type \"make cross\" to install to $(CROSS_PATH)"

native:
	make install-sdl prefix=/usr

cross:
	make install-sdl prefix=$(CROSS_PATH)

install-sdl:
	if test -d $(prefix); then \
	    cp -rv bin include lib share $(prefix)/; \
	    sed "s|^prefix=.*|prefix=$(prefix)|" <bin/sdl-config >$(prefix)/bin/sdl-config; \
	    chmod 755 $(prefix)/bin/sdl-config; \
	    sed "s|^libdir=.*|libdir=\'$(prefix)/lib\'|" <lib/libSDL.la >$(prefix)/lib/libSDL.la; \
	else \
	    echo "*** ERROR: $(prefix) does not exist!"; \
	    exit 1; \
	fi
