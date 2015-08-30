
CXX      = g++
CPPFLAGS = -I.
CXXFLAGS = -fno-exceptions -fno-rtti -fPIC -Wall
LD       = g++
LDFLAGS  = -fno-exceptions -fno-rtti -fPIC
LIBS     = -ldl

TESTS = tests/test1.so

default: all

all: watchman.exe $(TESTS)

watchman.exe: main.o watchman.o child.o initfini.o error.o
	$(LD) $(LFLAGS) -o $@ $^ $(LIBS)

main.o: main.cxx
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

%.o: %.cxx
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $<

%.so: %.o
	$(CXX) $(LDFLAGS) -shared -o $@ $<

tests/test1.so: 

clean:
	-rm -f tests/*.o tests/*.so *.o watchman.exe

