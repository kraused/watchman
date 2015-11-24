
CXX      = g++
CPPFLAGS = -I.
CXXFLAGS = -fno-exceptions -fno-rtti -fPIC -Wall -O0 -ggdb
LD       = g++
LDFLAGS  = -fno-exceptions -fno-rtti -fPIC -Wl,-export-dynamic -O0 -ggdb
LIBS     = -ldl

TESTS = tests/test1.so tests/test2.so

default: all

all: watchman.exe $(TESTS)

watchman.exe: main.o watchman.o child.o program.o buffer.o file_pair.o initfini.o error.o
	$(LD) $(LDFLAGS) -o $@ $^ $(LIBS)

main.o: main.cxx
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

%.o: %.cxx
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

%.so: %.o
	$(CXX) $(LDFLAGS) -shared -o $@ $<

clean:
	-rm -f tests/*.o tests/*.so *.o watchman.exe

