CXX = g++
CXXFLAGS = -fPIC -Os
SHARED= -shared
SLIB = liblev.a
OUT = a.out
PCH = prec.h.gch
RANLIB = ranlib
CORE_SRC  = entry.cpp image.cpp prim.cpp sound.cpp system.cpp util.cpp
OBJS = $(CORE_SRC:%.cpp=%.o)
DLIB = lev.so
LIBS = -llua -lluabind -lSDL -lvorbisfile

all: pch tags bin lib

bin: $(OUT)

lib: $(SLIB) $(DLIB)

dlib: $(DLIB)
slib: $(SLIB)

win-bin: $(WIN_OUT)

$(WIN_OUT): $(ALL_OBJS) main.o resource.rc
	$(WINDRES) -o resource.o resource.rc
	$(CXX) -o $(WIN_OUT) main.o resource.o $(SLIB) $(LIBS)

# precompiled header
pch: $(PCH)

clean:
	rm -rf tags *.o

distclean: clean
	rm -f *.out *.exe *.so *.dll *.a *.gch

tags: $(SRC) $(SRC:%.cpp=lev/%.hpp)
	ctags -R .

$(PCH): prec.h
	$(CXX) $(CXXFLAGS) $<

$(OUT): $(OBJS) main.o
	$(CXX) -o $(OUT) main.o $(OBJS) $(LIBS)

$(SLIB): $(OBJS)
	ar r $(SLIB) $(ALL_OBJS)
	$(RANLIB) $(SLIB)

$(DLIB): $(OBJS)
	$(CXX) $(SHARED) -o $(DLIB) $(OBJS) $(LIBS)

entry.o: entry.cpp lev/entry.hpp
image.o: image.cpp lev/image.hpp
prim.o: prim.cpp lev/prim.hpp
sound.o: sound.cpp lev/sound.hpp
system.o: system.cpp lev/system.hpp
util.o: util.cpp lev/util.hpp
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

