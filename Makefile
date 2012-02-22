CXX = g++
PREFIX=/tmp
VERSION=0.3.2
OUT = a.out
BIN = lev
DLIB = lev.so
SLIB = liblev.a
LUA_VERSION = 5.1

all: linux tags

none:
	@echo "Please do"
	@echo "  make PLATFORM"
	@echo "where PLATFORM is one of these:"
	@echo "  linux mac mingw"

linux: build-all bin lib

mac:
	make -f build/Makefile.macosx build-all \
		VERSION=$(VERSION)

mingw:
	make -f build/Makefile.mingw build-all \
		VERSION=$(VERSION)

build-all:
	cd src && make all

bin: src/$(OUT)
	install -d bin
	cp src/a.out bin/$(BIN)

lib: src/$(DLIB) src/$(SLIB)
	install -d lib
	cp src/{$(DLIB),$(SLIB)} lib/

tags:
	ctags -R src

install:
	install -d $(PREFIX)/bin
	install bin/$(BIN) $(PREFIX)/bin/$(BIN)
	install -d $(PREFIX)/lib
	install lib/$(SLIB) $(PREFIX)/lib/
	install -d $(PREFIX)/lib/lua/$(LUA_VERSION)
	install lib/$(DLIB) $(PREFIX)/lib/lua/$(LUA_VERSION)/

clean:
	cd src && make clean
	rm -rf tags

distclean:
	cd src && make distclean
	rm -rf bin lib
	rm -rf lev-win32-$(VERSION)

src/$(OUT): build
src/$(DLIB): build
src/$(SLIB): build

