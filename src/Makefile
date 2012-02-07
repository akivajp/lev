CC = gcc
CXX = g++
CXXFLAGS = -fPIC -O2 -I/usr/include/freetype2 `/usr/local/bin/sdl-config --cflags` \
	   -I./minizip
#CXXFLAGS = -fPIC -g -I/usr/include/freetype2 `/usr/local/bin/sdl-config --cflags` \
	   -I./minizip
SHARED= -shared
SLIB = liblev.a
OUT = a.out
PCH = prec.h.gch
AR = ar
RANLIB = ranlib
SRC  = archive.cpp draw.cpp entry.cpp font.cpp fs.cpp image.cpp map.cpp package.cpp prim.cpp \
       sound.cpp string.cpp system.cpp timer.cpp util.cpp window.cpp
OBJS = $(SRC:%.cpp=%.o)
DLIB = lev.so
#LIBS = -llua -lluabind -lSDL -lvorbisfile -lfreetype -lGL
LIBS = -llua -lluabind -lvorbisfile -lfreetype -lGL `/usr/local/bin/sdl-config --libs` \
       -L./minizip -lminizip -lboost_filesystem

MINIZIP_CFLAGS = -fPIC

all: pch tags bin lib

bin: $(OUT)

lib: $(SLIB) $(DLIB)

dlib: $(DLIB) minizip/libminizip.a
slib: $(SLIB) minizip/libminizip.a

minizip/libminizip.a: minizip
	cd minizip && make MINIZIP_CFLAGS="$(MINIZIP_CFLAGS)" RANLIB="$(RANLIB)" \
		CC="$(CC)" AR=$(AR)

win-bin: $(WIN_OUT)

$(WIN_OUT): $(ALL_OBJS) main.o resource.rc
	$(WINDRES) -o resource.o resource.rc
	$(CXX) -o $(WIN_OUT) main.o resource.o $(SLIB) $(LIBS)

# precompiled header
pch: $(PCH)

clean:
	rm -rf tags *.o
	cd minizip && make clean

distclean: clean
	rm -f *.out *.exe *.so *.dll *.a *.gch

tags: $(SRC) $(SRC:%.cpp=lev/%.hpp)
	ctags -R .

$(PCH): prec.h
	$(CXX) $(CXXFLAGS) $<

$(OUT): minizip/libminizip.a $(OBJS) main.o
	$(CXX) -o $(OUT) main.o $(OBJS) $(LIBS)

$(SLIB): $(OBJS)
	$(AR) r $(SLIB) $(OBJS)
	$(RANLIB) $(SLIB)

$(DLIB): $(OBJS)
	$(CXX) $(SHARED) -o $(DLIB) $(OBJS) $(LIBS)

archive.o: archive.cpp lev/archive.hpp
draw.o: draw.cpp lev/draw.hpp
entry.o: entry.cpp lev/entry.hpp
font.o: font.cpp lev/font.hpp
fs.o: fs.cpp lev/fs.hpp
image.o: image.cpp lev/image.hpp
main.o: main.cpp
map.o: map.cpp lev/map.hpp
package.o: package.cpp lev/package.hpp
prim.o: prim.cpp lev/prim.hpp
sound.o: sound.cpp lev/sound.hpp
string.o: string.cpp lev/string.hpp
system.o: system.cpp lev/system.hpp
timer.o: timer.cpp lev/timer.hpp
util.o: util.cpp lev/util.hpp
window.o: window.cpp lev/window.hpp
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

